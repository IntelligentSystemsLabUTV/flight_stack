/**
 * Flight Control timer callbacks.
 *
 * Roberto Masocco <robmasocco@gmail.com>
 * Intelligent Systems Lab <isl.torvergata@gmail.com>
 *
 * April 26, 2022
 */

#define NOOP ((void)0)

#include <flight_control/flight_control.hpp>

namespace FlightControl
{

/**
 * @brief Setpoints publishing timer callback.
 *
 * @throws RuntimeError if an impossible thing happens internally, i.e. if there's a bug.
 */
void FlightControlNode::setpoints_timer_callback()
{
  // Check if a setpoint stream is in progress
  if (last_stream_ts_.load(std::memory_order_acquire) != 0ULL) {
    // Check if the stream has timed out
    uint64_t now = get_time_us();
    if ((now - last_stream_ts_.load(std::memory_order_acquire) > setpoint_stream_timeout_us_) &&
      stream_reset_lock_.try_lock())
    {
      last_stream_ts_.store(0ULL, std::memory_order_release);
      stop_drone();
      stream_reset_lock_.unlock();
      RCLCPP_WARN(this->get_logger(), "Setpoint stream timed out, holding position");
    } else {
      return;
    }
  }

  OffboardControlMode control_mode_msg{};
  TrajectorySetpoint setpoint_msg{};
  std::array<float, 3> nans{NAN, NAN, NAN};
  uint64_t timestamp = get_time_us();

  // Get current setpoint
  setpoint_lock_.lock();
  Setpoint current_setpoint = fmu_setpoint_;
  setpoint_lock_.unlock();

  // Fill offboard_control_mode message
  control_mode_msg.set__timestamp(timestamp);
  control_mode_msg.set__acceleration(false);
  control_mode_msg.set__attitude(false);
  control_mode_msg.set__body_rate(false);
  if (current_setpoint.control_mode == ControlModes::POSITION) {
    control_mode_msg.set__position(true);
    control_mode_msg.set__velocity(false);
  } else if (current_setpoint.control_mode == ControlModes::VELOCITY) {
    control_mode_msg.set__position(false);
    control_mode_msg.set__velocity(true);
  } else {
    // Should never happen, if it does it's a bug
    RCLCPP_FATAL(this->get_logger(), "Invalid OFFBOARD control mode stored");
    throw std::runtime_error("Invalid OFFBOARD control mode stored");
  }

  // Get the latest odom -> map transform
  TransformStamped map_to_odom{};
  rclcpp::Time tf_time = this->get_clock()->now();
  while (true) {
    try {
      map_to_odom = tf_buffer_->lookupTransform(
        map_frame_,
        odom_frame_,
        tf_time,
        tf2::durationFromSec(tf2_timeout_));
      break;
    } catch (const tf2::ExtrapolationException & e) {
      // Just get the latest
      tf_time = rclcpp::Time{};
    } catch (const tf2::TransformException & e) {
      RCLCPP_ERROR(
        this->get_logger(),
        "FlightControlNode::setpoints_timer_callback: TF exception: %s",
        e.what());
      return;
    }
  }
  Eigen::Isometry3d odom_map_iso = tf2::transformToEigen(map_to_odom).inverse();

  // Fill trajectory_setpoint message (from map to odom) (from NWU to NED) (vyaw does not change)
  setpoint_msg.set__timestamp(timestamp);
  setpoint_msg.set__acceleration(nans);
  setpoint_msg.set__jerk(nans);
  setpoint_msg.set__thrust(nans);
  if (current_setpoint.control_mode == ControlModes::POSITION) {
    Eigen::Isometry3d setpoint_map_iso = Eigen::Isometry3d::Identity();
    setpoint_map_iso.rotate(Eigen::AngleAxisd(current_setpoint.yaw, Eigen::Vector3d::UnitZ()));
    setpoint_map_iso.pretranslate(
      Eigen::Vector3d(
        current_setpoint.x,
        current_setpoint.y,
        current_setpoint.z));
    Eigen::Isometry3d setpoint_odom_iso = odom_map_iso * setpoint_map_iso;
    Eigen::Quaterniond setpoint_odom_q(setpoint_odom_iso.rotation());

    setpoint_msg.set__x(setpoint_odom_iso.translation().x());
    setpoint_msg.set__y(-setpoint_odom_iso.translation().y());
    setpoint_msg.set__z(-setpoint_odom_iso.translation().z());
    setpoint_msg.set__yaw(-Eigen::EulerAnglesXYZd(setpoint_odom_iso.rotation()).gamma());
    setpoint_msg.set__vx(NAN);
    setpoint_msg.set__vy(NAN);
    setpoint_msg.set__vz(NAN);
    setpoint_msg.set__yawspeed(NAN);

    // Publish the setpoint to RViz for visualization
    PoseStamped rviz_setpoint_msg{};
    rviz_setpoint_msg.header.set__frame_id(odom_frame_);
    rviz_setpoint_msg.header.set__stamp(rclcpp::Time(timestamp * 1000UL));
    rviz_setpoint_msg.pose.position.set__x(setpoint_odom_iso.translation().x());
    rviz_setpoint_msg.pose.position.set__y(setpoint_odom_iso.translation().y());
    rviz_setpoint_msg.pose.position.set__z(setpoint_odom_iso.translation().z());
    rviz_setpoint_msg.pose.orientation.set__w(setpoint_odom_q.w());
    rviz_setpoint_msg.pose.orientation.set__x(setpoint_odom_q.x());
    rviz_setpoint_msg.pose.orientation.set__y(setpoint_odom_q.y());
    rviz_setpoint_msg.pose.orientation.set__z(setpoint_odom_q.z());
    rviz_position_setpoint_pub_->publish(rviz_setpoint_msg);
  }
  if (current_setpoint.control_mode == ControlModes::VELOCITY) {
    Eigen::Vector3d v_setpoint_map(
      current_setpoint.vx,
      current_setpoint.vy,
      current_setpoint.vz);
    Eigen::Matrix3d R_odom_map = odom_map_iso.rotation();
    Eigen::Vector3d v_setpoint_odom = R_odom_map * v_setpoint_map;
    double yaw_setpoint_odom = 0.0f;
    if (!std::isnan(current_setpoint.yaw)) {
      Eigen::AngleAxisd yaw_setpoint_map(current_setpoint.yaw, Eigen::Vector3d::UnitZ());
      yaw_setpoint_odom = Eigen::EulerAnglesXYZd(R_odom_map * yaw_setpoint_map).gamma();
    } else {
      yaw_setpoint_odom = NAN;
    }

    setpoint_msg.set__x(NAN);
    setpoint_msg.set__y(NAN);
    setpoint_msg.set__z(NAN);
    setpoint_msg.set__yaw(-yaw_setpoint_odom);
    setpoint_msg.set__vx(v_setpoint_odom.x());
    setpoint_msg.set__vy(-v_setpoint_odom.y());
    setpoint_msg.set__vz(-v_setpoint_odom.z());
    setpoint_msg.set__yawspeed(-current_setpoint.vyaw);
  }

  // Publish messages
  offboard_control_mode_pub_->publish(control_mode_msg);
  trajectory_setpoint_pub_->publish(setpoint_msg);
}

} // namespace FlightControl

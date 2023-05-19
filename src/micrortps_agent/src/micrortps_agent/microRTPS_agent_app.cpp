/**
 * MicroRTPS Agent standalone application.
 *
 * Roberto Masocco <robmasocco@gmail.com>
 * Intelligent Systems Lab <isl.torvergata@gmail.com>
 *
 * May 9, 2023
 */

#include <cstdlib>
#include <csignal>

#include <rclcpp/rclcpp.hpp>

#include <ros2_app_manager/ros2_app_manager.hpp>
#include <ros2_signal_handler/ros2_signal_handler.hpp>

#include <micrortps_agent/microRTPS_agent.hpp>

using namespace DUAAppManagement;

int main(int argc, char ** argv)
{
  ROS2AppManager<rclcpp::executors::MultiThreadedExecutor, MicroRTPSAgent::AgentNode> app_manager(
    argc,
    argv,
    "microRTPS_agent_app");

  SignalHandler & sig_handler = SignalHandler::get_global_signal_handler();
  sig_handler.init(
    app_manager.get_context(),
    "microRTPS_agent_app_signal_handler");
  sig_handler.install(SIGINT);
  sig_handler.install(SIGTERM);
  sig_handler.install(SIGQUIT);
  sig_handler.install(SIGABRT);
  sig_handler.ignore(SIGPIPE);
  sig_handler.ignore(SIGBUS);
  sig_handler.ignore(SIGHUP);
  sig_handler.ignore(SIGUSR1);
  sig_handler.ignore(SIGUSR2);

  app_manager.run();

  sig_handler.fini();
  app_manager.shutdown();

  exit(EXIT_SUCCESS);
}
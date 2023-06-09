/**
 * Definition of data types for the MicroRTPS ROS 2 node implementation.
 *
 * Roberto Masocco <robmasocco@gmail.com>
 * Intelligent Systems Lab <isl.torvergata@gmail.com>
 *
 * May 12, 2023
 */

#ifndef MICRORTPS_AGENT__TYPES_HPP_
#define MICRORTPS_AGENT__TYPES_HPP_

namespace MicroRTPSAgent
{

/**
 * Holds an outbound message in the queue.
 */
struct OutboundMsg
{
  uint8_t topic_id;
  void * msg;
};

} // namespace MicroRTPSAgent

#endif // MICRORTPS_AGENT__TYPES_HPP_

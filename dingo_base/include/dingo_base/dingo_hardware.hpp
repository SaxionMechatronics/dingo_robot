/**
 *  \file       dingo_hardware.h
 *  \brief      Class representing Dingo hardware
 *  \copyright  Copyright (c) 2020, Clearpath Robotics, Inc.
 *
 * Software License Agreement (BSD)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Clearpath Robotics, Inc. nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL CLEARPATH ROBOTICS, INC. BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Please send comments, questions, or patches to code@clearpathrobotics.com
 */

#ifndef DINGO_BASE_DINGO_HARDWARE_HPP_
#define DINGO_BASE_DINGO_HARDWARE_HPP_

#include <vector>

#include "boost/thread.hpp"
#include "boost/shared_ptr.hpp"
#include "hardware_interface/base_interface.hpp"
#include "hardware_interface/handle.hpp"
#include "hardware_interface/hardware_info.hpp"
#include "hardware_interface/system_interface.hpp"
#include "hardware_interface/types/hardware_interface_return_values.hpp"
#include "hardware_interface/types/hardware_interface_status_values.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "puma_motor_driver/socketcan_gateway.hpp"
#include "puma_motor_driver/driver.hpp"
#include "puma_motor_driver/multi_driver_node.hpp"
#include "puma_motor_msgs/msg/multi_feedback.hpp"

namespace dingo_base
{

/** This class encapsulates the Dingo hardware */
class DingoHardware 
: public hardware_interface::BaseInterface<hardware_interface::SystemInterface>
{
public:
  hardware_interface::return_type configure(const hardware_interface::HardwareInfo & info) override;

  std::vector<hardware_interface::StateInterface> export_state_interfaces() override;

  std::vector<hardware_interface::CommandInterface> export_command_interfaces() override;

  hardware_interface::return_type start() override;

  hardware_interface::return_type stop() override;

  hardware_interface::return_type read() override;

  hardware_interface::return_type write() override;

  /** Makes a single attempt to connect to the CAN bus if not connected.
   *  @return true if connected; false if not connected
   */
  bool connectIfNotConnected();





  /** Validates the parameters for the motor drivers */
  void verify();

  /** Determines if the robot is active and updates the state.
   *  @return true if the robot is active (drivers configured); else false
   */
  bool isActive();

  /** Determines if ALL drivers are active.
   *  @return true all are active; else false
   */
  bool areAllDriversActive();

  /** Checks each driver to see if power has been reset and resets the
   *  driver if needed.
   */
  void powerHasNotReset();

  /** Checks to see if the motor drivers are configured.
   *  @return true if the motor drivers are not configured; false if configured
   */
  bool inReset();

  /** Requests the power state of each motor driver */
  void requestData();

  /** Populates the internal joint state struct from the most recent CAN data
   *  received from the motor controller. Called from the controller thread.
   */
  void updateJointsFromHardware();

  /** Populates and publishes Drive message based on the controller outputs.
   *  Called from the controller thread.
   */
  void command();

  /** Processes all received messages through the connected driver instances. */
  void canRead();

	void canReadThread();

  /** Sends all queued data to Puma motor driver the gateway. */
  void canSend();

private:
  /** Gateway for Puma motor driver */
  std::shared_ptr<puma_motor_driver::Gateway> gateway_;

  /** Indicates if the drivers are configured */
  bool active_;

  /** The gear ratio for the motors */
  double gear_ratio_;

  /** The number of encoding steps per revolution */
  int encoder_cpr_;

  /** The PID gains for the motor controller */
  double gain_p_, gain_i_, gain_d_;

  /** Indicates if the motor direction should be flipped */
  bool flip_motor_direction_;

  /** Puma motor drivers (2 or 4) */
  std::vector<puma_motor_driver::Driver> drivers_;

  /** Puma multi-node driver */
  std::shared_ptr<puma_motor_driver::MultiDriverNode> multi_driver_node_;

	std::thread can_read_thread_;

  /** Latest information for each joint.
   *  These are mutated on the controls thread only.
   *  2 joints for Dingo-D, 4 joints for Dingo-O.
   */
  struct Joint
  {
    double position;
    double velocity;
    double effort;
    double velocity_command;

    Joint() : position(0), velocity(0), effort(0), velocity_command(0)
    {
    }
  }
  joints_[4];
};

}  // namespace dingo_base

#endif  // DINGO_BASE_DINGO_HARDWARE_HPP_

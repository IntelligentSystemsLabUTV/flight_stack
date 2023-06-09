# flight_stack

PX4-based software stack for autonomous flight.

## Abstract

This project is a collection of software modules that constitute a basic software stack for autonomous flight, relying on the [PX4](px4.io) firmware and the [ROS 2](https://index.ros.org/doc/ros2/) middleware with the [FastDDS](https://www.eprosima.com/index.php/products-all/eprosima-fast-dds) implementation.

The stack supposes that a PX4-based flight controller is available, and that it is connected to a companion computer running ROS 2. The purpose of the stack is to provide a set of functionalities to communicate, and exchange data, with the firmware, and to implement basic autonomous flight capabilities.

This stack offers only a software abstraction towards the low-level, flight control firmware. It does not offer localization or mapping capabilities, but it can be used in conjunction with other software stacks that provide such functionalities and to forward such information to the flight controller. It can also provide real-time data such as global and local position information from the flight controller to the rest of a software architecture.

## Contents

The Flight Stack is made of three main components:

- [x] [`px4_msgs`](src/px4_msgs/README.md): A set of ROS 2 messages that are used to communicate with the PX4 firmware.
- [x] [`micrortps_agent`](src/micrortps_agent/README.md): A software module that handles communication with the PX4 firmware over UART or UDP, reading and writing data on a FastDDS network.
- [x] [`flight_control`](src/flight_control/README.md): A ROS 2 node that abstracts the drone, and implements basic autonomous flight capabilities.
- [x] [`flight_stack_bringup`](src/flight_stack_bringup/README.md): A ROS 2 package that offers launch files to start the Flight Stack nodes in multiple configurations.

There is also a `bringup` package, with launch configurations for the `micrortps_agent` and `flight_control` nodes.

## Usage

See the individual package READMEs for specific usage instructions.

### DUA integration

This project is based on the [Distributed Unified Architecture](dua-template.md).

It is an independent unit that can also be used as a module in a larger project.

#### Supported targets

The following targets are supported:

- [x] `x86-base`
- [x] `x86-dev`
- [x] `x86-cudev`

`armv8` and `jetson` targets should be supported as well, but they have not been tested yet.

---

## License

This work is licensed under the GNU General Public License v3.0. See the [`LICENSE`](LICENSE) file for details.

## Copyright

Copyright (c) 2023, Intelligent Systems Lab, University of Rome Tor Vergata

Copyright (c) 2017, Proyectos y Sistemas de Mantenimiento SL (eProsima)

Copyright (c) 2018-2021, PX4 Development Team

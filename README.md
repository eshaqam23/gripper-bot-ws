# Gripper Bot Workspace

ROS 2 workspace for a small differential-drive gripper car. The robot is controlled from ROS topics and bridged over serial to an ESP-based motor and servo controller.

## Packages

- `esp_bridge`: ROS 2 Python node that converts `cmd_vel` and gripper commands into serial messages for the ESP.
- `gripper_controller`: keyboard gripper command publisher.
- `robot_bringup`: launch/config package for starting the robot control stack.
- `motor_logic`: Arduino/ESP firmware for motor and gripper servo control.

## ROS Topics

- `cmd_vel` (`geometry_msgs/Twist`): differential drive velocity command.
- `gripper_open` (`std_msgs/Bool`): `true` opens the gripper, `false` closes it.
- `ros_data` (`std_msgs/String`): serial message sent from ROS to the ESP, useful for testing.
- `esp_data` (`std_msgs/String`): serial response received from the ESP.

## Serial Protocol

- Drive command: `D,<left_pwm>,<left_dir>,<right_pwm>,<right_dir>`
- Gripper command: `G,<servo_angle>`

Example messages:

```text
D,255,1,255,1
G,120
G,30
```

## Build

```bash
colcon build --symlink-install
source install/setup.bash
```

On Windows PowerShell with a sourced ROS 2 environment, use the equivalent setup script:

```powershell
colcon build --symlink-install
.\install\setup.ps1
```

## Run

Start the bringup launch file:

```bash
ros2 launch robot_bringup start_rc.launch.py
```

Pass a custom microcontroller serial port if needed:

```bash
ros2 launch robot_bringup start_rc.launch.py port:=/dev/ttyUSB0
```

## Quick Topic Tests

Open the gripper:

```bash
ros2 topic pub --once /gripper_open std_msgs/msg/Bool "{data: true}"
```

Close the gripper:

```bash
ros2 topic pub --once /gripper_open std_msgs/msg/Bool "{data: false}"
```

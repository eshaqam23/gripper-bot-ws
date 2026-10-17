# gripper-bot-ws

Minimal ROS 2 workspace skeleton for the gripper bot.

## Layout

- `src/gripper_bot_skeleton` — hello-world node, launch file and params
- `src/gripper_controller` — gripper/drive command subscriber (logs only for now)
- `src/usb_bridge` — `motor_commands` node: converts `/cmd_vel` into
  `leftPwm,leftDir,rightPwm,rightDir\n` and sends it to the microcontroller over serial
- `src/robot_bringup` — launches keyboard teleop + `motor_commands`; hardware
  settings (serial port, baud, wheel base, max speed) live in `config/params.yaml`
- `firmware/motor_logic` — Arduino sketch that reads those serial commands and drives
  the motors through an L298N-style driver (not built by colcon; flash it with the Arduino IDE)

## Build

```bash
colcon build --symlink-install
source install/setup.bash
```

## Run

```bash
ros2 launch gripper_bot_skeleton skeleton.launch.py
ros2 launch robot_bringup start_rc.launch.py   # needs the microcontroller connected
```

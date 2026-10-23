# gripper-bot-ws

ROS 2 workspace for the gripper bot: an RC car with a two-servo gripper (claw + joint)
that can drive around and grab/release objects.

```
teleop_twist_keyboard ──/cmd_vel──┐
                                  ├─► usb_bridge/motor_commands ──serial──► firmware/motor_logic ──► drive motors + gripper servos
gripper_keys ──/gripper/grab, ────┘
               /gripper/release
```

## Layout

- `src/usb_bridge` — `motor_commands` node, the only node that talks to the microcontroller:
  - `/cmd_vel` (`geometry_msgs/Twist`) → drive command
  - `/gripper/position` (`std_msgs/Float32`, 0.0 = open, 1.0 = closed) → claw position,
    mapped to servo angles by `claw_open_angle` / `claw_closed_angle` in params
  - `/gripper/grab`, `/gripper/release` (`std_srvs/Trigger`) → claw position 1.0 / 0.0
  - `/gripper_cmd` (`std_msgs/Float32MultiArray`, `[claw, joint]` degrees) → exact servo
    angles, for tuning the open/closed angles
  - resends the drive command at 10 Hz as a heartbeat; publishes sent lines on `/ros_data`
    and microcontroller replies on `/usb_data`
- `src/gripper_controller` — `gripper_keys` keyboard node (g = grab, r = release, q = quit)
- `src/robot_bringup` — `start_rc.launch.py` starts teleop, gripper keys and the bridge;
  all hardware settings live in `config/params.yaml`
- `firmware/motor_logic` — Arduino sketch: L298N drive motors + gripper servos
  (not built by colcon; flash it with the Arduino IDE or `arduino-cli`)
- `tools/fake_arduino.py` — stands in for the microcontroller so everything can be tested
  without hardware

## Serial protocol

One command per line at 9600 baud; the microcontroller replies `OK` or `ERR` to each.

| Command | Meaning |
|---|---|
| `D,leftPwm,leftDir,rightPwm,rightDir` | drive: pwm 0–255, dir 1 = forward, 0 = backward |
| `G,clawAngle,jointAngle` | gripper servo angles in degrees |

Safety: the firmware stops the drive motors if no `D` command arrives for 500 ms
(for example if the bridge crashes or the link drops).

## Build

```bash
colcon build --symlink-install
source install/setup.bash
```

## Run

```bash
ros2 launch robot_bringup start_rc.launch.py   # microcontroller connected
```

Two xterm windows open: drive with the teleop window, grab/release with the gripper window.

### Without hardware

```bash
python3 tools/fake_arduino.py                                  # terminal 1
ros2 launch robot_bringup start_rc.launch.py port:=/tmp/fake_arduino   # terminal 2
```

The fake Arduino prints what the motors and gripper would do.

## Tests

```bash
colcon test --packages-select usb_bridge robot_bringup && colcon test-result --verbose
```

## Still placeholders (fill in once the hardware is known)

- `src/robot_bringup/config/params.yaml` — serial port, baud, wheel base, max speed,
  claw open/closed angles
- `firmware/motor_logic/helpers.h` — motor and servo pins, servo angle limits,
  servo start angles, drive timeout

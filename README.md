# gripper-bot-ws

ROS 2 workspace for the gripper bot: an RC car with a servo gripper (claw, plus an
optional joint servo that is still TBD) that can drive around and grab/release objects.

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
- `firmware/motor_logic` — Arduino sketch: L298N drive motors + gripper servos.
  Builds for the ESP32-C6 (ESP32 Arduino core 3.x) or an Arduino Uno for bench tests;
  all pins are in `helpers.h`, one block per board, picked by the board selected in the IDE.
  Talks over USB serial (the C6 has no classic Bluetooth).
  Not built by colcon; flash it with the Arduino IDE or `arduino-cli`.
- `tools/fake_arduino.py` — stands in for the microcontroller so everything can be tested
  without hardware
- `src/gripper_bot_skeleton` — original template package, not used by the launch file

## Serial protocol

One command per line at 9600 baud; the microcontroller replies `OK` or `ERR` to each.

| Command | Meaning |
|---|---|
| `D,leftPwm,leftDir,rightPwm,rightDir` | drive: pwm 0–255, dir 1 = forward, 0 = backward |
| `G,clawAngle,jointAngle` | gripper servo angles in degrees |

Safety: the firmware stops the drive motors if no `D` command arrives for 500 ms
(for example if the bridge crashes or the link drops).

## Setup

On the ROS machine (Ubuntu with ROS 2 installed), from the workspace root:

```bash
sudo apt install xterm python3-serial ros-$ROS_DISTRO-teleop-twist-keyboard
rosdep install --from-paths src --ignore-src -y   # or let rosdep find the above
sudo usermod -aG dialout $USER                    # serial port access; log out and back in
```

## Flash the firmware

Open `firmware/motor_logic/motor_logic.ino` in the Arduino IDE, pick the board under
Tools > Board, then upload:

- **ESP32-C6:** install "esp32" by Espressif **3.x** in the Boards Manager (older versions
  don't support the C6), then select the ESP32-C6 board. If you plug into the board's
  native USB port (not the UART one), set Tools > USB CDC On Boot > Enabled, or nothing
  shows up in Serial Monitor.
- **Arduino Uno (bench tests):** works as is; needs the Servo library.

Pins are set in `firmware/motor_logic/helpers.h`, one block per board. To check the
firmware on its own, open Serial Monitor (9600 baud, Newline) and type a command such as
`G,-20,90`; it replies `OK`. Close Serial Monitor before starting ROS, only one program can
use the port.

## Build

```bash
colcon build --symlink-install
source install/setup.bash   # in every new terminal
```

## Run

Find the microcontroller's port (usually `/dev/ttyACM0`):

```bash
ls /dev/ttyACM* /dev/ttyUSB*
```

```bash
ros2 launch robot_bringup start_rc.launch.py port:=/dev/ttyACM0
```

Two xterm windows open: drive with the teleop window, grab/release with the gripper window.
Wait ~2 s after launching; opening the port can reset the microcontroller (the Uno does).

## Gripper commands

The claw is commanded as a position: 0.0 = open, 1.0 = closed. `claw_open_angle` and
`claw_closed_angle` in `src/robot_bringup/config/params.yaml` set the servo angle at each end
(relaunch after changing them, no reflash needed).

| How | Command | Effect |
|---|---|---|
| Keyboard | `g` / `r` in the gripper window (`q` quits) | grab (1.0) / release (0.0) |
| Services | `ros2 service call /gripper/grab std_srvs/srv/Trigger` (or `/gripper/release`) | same as `g` / `r` |
| Position | `ros2 topic pub --once /gripper/position std_msgs/msg/Float32 "{data: 0.5}"` | any position 0.0-1.0 |
| Raw angles | `ros2 topic pub --once /gripper_cmd std_msgs/msg/Float32MultiArray "{data: [120, 90]}"` | exact `[claw, joint]` servo angles, for tuning |

To see what is sent and what the microcontroller replies:

```bash
ros2 topic echo /ros_data   # lines sent, e.g. G,180,90 (plus the D heartbeat at 10 Hz)
ros2 topic echo /usb_data   # replies, should be OK
```

### Without hardware

```bash
python3 tools/fake_arduino.py                                  # terminal 1
ros2 launch robot_bringup start_rc.launch.py port:=/tmp/fake_arduino   # terminal 2
```

The fake Arduino prints what the motors and gripper would do.

## Tests

```bash
colcon test --packages-select usb_bridge robot_bringup gripper_controller && colcon test-result --verbose
```

## Still placeholders (fill in once the hardware is known)

- `src/robot_bringup/config/params.yaml` — serial port, baud, wheel base, max speed,
  claw open/closed angles
- `firmware/motor_logic/helpers.h` — ESP32-C6 ENA/ENB and claw pins, `JOINT_PIN`
  (-1 until we know if there is a joint servo), servo angle limits, servo start angles,
  drive timeout

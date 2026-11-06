# gripper-bot-ws

ROS 2 workspace for the VT CRO gripper bot: keyboard teleop, an ESP motor bridge, and a gripper servo.

The robot computer is expected to be Linux. Motor and gripper commands go out over USB serial to the ESP firmware in `src/motor_logic`.

## Packages

| Path | What it does |
|---|---|
| `src/robot_bringup` | Launch file that starts teleop and the motor bridge. |
| `src/esp_bridge` | `motor_commands` node. Subscribes to `cmd_vel` and `gripper_open`, converts them to serial lines, and writes them to the ESP. |
| `src/motor_logic` | Arduino sketch for the ESP. Drives a differential pair and a gripper servo from serial commands. |

## Dependencies

- ROS 2 (`rclpy`, `geometry_msgs`, `std_msgs`, `teleop_twist_keyboard`)
- `xterm` (the teleop node is launched inside one)
- `python3-serial` (`pyserial`), used by `esp_bridge`
- Arduino IDE (or equivalent) to flash `src/motor_logic` onto the ESP
- `ESP32Servo` library for the gripper servo

```bash
sudo apt update
sudo apt install xterm python3-serial ros-${ROS_DISTRO}-teleop-twist-keyboard
```

## Build

From the workspace root:

```bash
colcon build --symlink-install
source install/setup.bash
```

## Run

Flash `src/motor_logic/motor_logic.ino` to the ESP first. The sketch talks at 9600 baud and expects one command per line:

```text
D,left_pwm,left_dir,right_pwm,right_dir
G,servo_angle
```

PWM is 0–255. Direction is `1` for forward and `0` for backward. `G` sets the gripper servo angle in degrees. The ESP replies `OK DRIVE`, `OK GRIPPER`, or `ERR`.

`esp_bridge` opens `/dev/ttyUSB0` at 9600 baud. The wheelbase used for the differential-drive split is 8 inches (0.2032 m), and each wheel speed is scaled so 0.5 m/s maps to PWM 255. `gripper_open` `true` sends angle 120; `false` sends angle 30.

Bring up teleop and the motor bridge:

```bash
ros2 launch robot_bringup start_rc.launch.py
```

Teleop publishes `geometry_msgs/Twist` on `cmd_vel`. Use the keys printed in the xterm window.

Open or close the gripper from another terminal:

```bash
ros2 topic pub --once /gripper_open std_msgs/msg/Bool "{data: true}"
ros2 topic pub --once /gripper_open std_msgs/msg/Bool "{data: false}"
```

Published debug topics:

- `ros_data` — the serial line sent to the ESP
- `esp_data` — the line read back from the ESP

## Motor pins

Defined in `src/motor_logic/helpers.h`:

| Side | Direction pins | PWM pin |
|---|---|---|
| Left | IN3 = 18, IN4 = 9 | ENB = 25 |
| Right | IN1 = 20, IN2 = 19 | ENA = 26 |

Gripper servo: pin 15. Open angle 120, closed angle 30.

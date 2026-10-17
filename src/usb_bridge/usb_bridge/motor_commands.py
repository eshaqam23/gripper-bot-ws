import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from std_msgs.msg import String
import serial
# import math
# import numpy as np


class Motor_Commands(Node):
    """ROS2 node for communicating motor commands via USB"""

    def __init__(self):
        super().__init__("motor_commands")

        # cmd_vel subscriber
        self.cmd_vel_sub = self.create_subscription(
            Twist, "cmd_vel", self.cmd_vel_callback, 10
        )

        self.module_pub = self.create_publisher(String, "usb_data", 10)
        self.ros_pub = self.create_publisher(String, "ros_data", 10)

        # Hardware settings: placeholders until the real metrics are known
        # (override in config/params.yaml)
        self.declare_parameter("port", "/dev/rfcomm0")
        self.declare_parameter("baud", 9600)
        self.declare_parameter("wheel_base", 0.2032)  # meters (8 inches)
        self.declare_parameter("max_speed", 0.5)  # m/s that maps to PWM 255

        port = self.get_parameter("port").value
        baud = self.get_parameter("baud").value
        self.wheel_base = self.get_parameter("wheel_base").value
        self.max_speed = self.get_parameter("max_speed").value

        # Microcontroller connection
        self.ser = serial.Serial(port, baud, timeout=1)

    def cmd_vel_callback(self, msg):
        linear_velocity = msg.linear.x
        angular_velocity = msg.angular.z
        wheel_base = self.wheel_base

        # calculate velocity per wheel
        left_velocity = linear_velocity - (angular_velocity * wheel_base) / 2
        right_velocity = linear_velocity + (angular_velocity * wheel_base) / 2

        # determine wheel speed
        left_speed = abs(left_velocity)
        right_speed = abs(right_velocity)

        # determine wheel direction
        left_forward = 0 if left_velocity < 0 else 1
        right_forward = 0 if right_velocity < 0 else 1

        # normalize wheel velocity between 0 and 255
        # max(left_speed) = max_speed, max(right_speed) = max_speed
        left_speed_normalized = min(int(left_speed / self.max_speed * 255), 255)
        right_speed_normalized = min(int(right_speed / self.max_speed * 255), 255)

        message = f"{left_speed_normalized},{left_forward},{right_speed_normalized},{right_forward}\n"
        self.ser.write(message.encode("utf-8"))

        # Message sent by ESP
        ros_msg = String()
        ros_msg.data = message
        self.ros_pub.publish(ros_msg)

        # Message received by ESP
        if self.ser.in_waiting > 0:
            line = self.ser.readline().decode("utf-8").rstrip()
            module_msg = String()
            module_msg.data = line
            self.module_pub.publish(module_msg)
            self.get_logger().info(f"Publishing: {line}")


def main(args=None):
    rclpy.init(args=args)
    node = Motor_Commands()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("Shutting down Motor Commands node")
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from std_msgs.msg import String
import serial

from usb_bridge.protocol import drive_message, STOP_MESSAGE


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

        # Resend the current drive command at 10 Hz so the microcontroller's
        # safety timeout only stops the car if this node or the link dies
        self.drive_msg = STOP_MESSAGE
        self.heartbeat_timer = self.create_timer(0.1, self.heartbeat_callback)

    def send(self, message):
        self.ser.write(message.encode("utf-8"))

    def cmd_vel_callback(self, msg):
        self.drive_msg = drive_message(
            msg.linear.x, msg.angular.z, self.wheel_base, self.max_speed
        )
        self.send(self.drive_msg)

        # Message sent to ESP
        ros_msg = String()
        ros_msg.data = self.drive_msg
        self.ros_pub.publish(ros_msg)

    def heartbeat_callback(self):
        self.send(self.drive_msg)

        # Messages received from ESP
        while self.ser.in_waiting > 0:
            line = self.ser.readline().decode("utf-8", errors="replace").rstrip()
            module_msg = String()
            module_msg.data = line
            self.module_pub.publish(module_msg)
            if line == "ERR":
                self.get_logger().warn("Microcontroller rejected a command")

    def stop(self):
        self.send(STOP_MESSAGE)


def main(args=None):
    rclpy.init(args=args)
    node = Motor_Commands()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info("Shutting down Motor Commands node")
    finally:
        node.stop()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == "__main__":
    main()

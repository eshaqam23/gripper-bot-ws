from geometry_msgs.msg import Twist
import rclpy
from rclpy.node import Node
import serial
from std_msgs.msg import Float32MultiArray, String
from std_srvs.srv import Trigger
from usb_bridge.protocol import drive_message, gripper_message, STOP_MESSAGE


class Motor_Commands(Node):
    """ROS2 node for communicating motor commands via USB."""

    def __init__(self):
        super().__init__('motor_commands')

        # cmd_vel subscriber
        self.cmd_vel_sub = self.create_subscription(
            Twist, 'cmd_vel', self.cmd_vel_callback, 10
        )

        self.module_pub = self.create_publisher(String, 'usb_data', 10)
        self.ros_pub = self.create_publisher(String, 'ros_data', 10)

        # Hardware settings: placeholders until the real metrics are known
        # (override in config/params.yaml)
        self.declare_parameter('port', '/dev/rfcomm0')
        self.declare_parameter('baud', 9600)
        self.declare_parameter('wheel_base', 0.2032)  # meters (8 inches)
        self.declare_parameter('max_speed', 0.5)  # m/s that maps to PWM 255

        port = self.get_parameter('port').value
        baud = self.get_parameter('baud').value
        self.wheel_base = self.get_parameter('wheel_base').value
        self.max_speed = self.get_parameter('max_speed').value

        # Gripper [claw, joint] angles in degrees for grab/release
        self.declare_parameter('gripper_open_angles', [90.0, 90.0])
        self.declare_parameter('gripper_closed_angles', [20.0, 90.0])

        self.gripper_open = self.get_parameter('gripper_open_angles').value
        self.gripper_closed = self.get_parameter('gripper_closed_angles').value

        # Gripper control: exact angles on a topic, or grab/release services
        self.gripper_sub = self.create_subscription(
            Float32MultiArray, 'gripper_cmd', self.gripper_callback, 10
        )
        self.grab_srv = self.create_service(Trigger, 'gripper/grab', self.grab_callback)
        self.release_srv = self.create_service(
            Trigger, 'gripper/release', self.release_callback
        )

        # Microcontroller connection
        self.ser = serial.Serial(port, baud, timeout=1)

        # Resend the current drive command at 10 Hz so the microcontroller's
        # safety timeout only stops the car if this node or the link dies
        self.drive_msg = STOP_MESSAGE
        self.heartbeat_timer = self.create_timer(0.1, self.heartbeat_callback)

    def send(self, message):
        self.ser.write(message.encode('utf-8'))

    def set_gripper(self, angles):
        message = gripper_message(angles)
        self.send(message)

        ros_msg = String()
        ros_msg.data = message
        self.ros_pub.publish(ros_msg)

    def gripper_callback(self, msg):
        try:
            self.set_gripper(list(msg.data))
        except ValueError as e:
            self.get_logger().warning(f'Ignoring gripper_cmd: {e}')

    def grab_callback(self, request, response):
        self.set_gripper(self.gripper_closed)
        response.success = True
        response.message = 'Gripper closing'
        return response

    def release_callback(self, request, response):
        self.set_gripper(self.gripper_open)
        response.success = True
        response.message = 'Gripper opening'
        return response

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
            line = self.ser.readline().decode('utf-8', errors='replace').rstrip()
            module_msg = String()
            module_msg.data = line
            self.module_pub.publish(module_msg)
            if line == 'ERR':
                self.get_logger().warning('Microcontroller rejected a command')

    def stop(self):
        self.send(STOP_MESSAGE)


def main(args=None):
    rclpy.init(args=args)
    node = Motor_Commands()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info('Shutting down Motor Commands node')
    finally:
        node.stop()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()

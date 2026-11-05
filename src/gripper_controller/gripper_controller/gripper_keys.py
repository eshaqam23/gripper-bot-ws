"""Keyboard control for the gripper: g = grab, r = release, q = quit."""

import select
import sys
import termios
import tty

import rclpy
from rclpy.node import Node
from std_srvs.srv import Trigger

HELP = """
Gripper keys
------------
g : grab (close)
r : release (open)
q : quit
"""


class GripperKeys(Node):

    def __init__(self):
        super().__init__('gripper_keys')
        self.grab_client = self.create_client(Trigger, 'gripper/grab')
        self.release_client = self.create_client(Trigger, 'gripper/release')

    def call(self, client):
        if not client.service_is_ready():
            self.get_logger().warning(
                f'{client.srv_name} not available (is motor_commands running?)'
            )
            return

        future = client.call_async(Trigger.Request())
        future.add_done_callback(self.on_response)

    def on_response(self, future):
        result = future.result()
        if result is not None:
            self.get_logger().info(result.message)


def read_key(timeout):
    ready, _, _ = select.select([sys.stdin], [], [], timeout)
    return sys.stdin.read(1) if ready else ''


def main(args=None):
    rclpy.init(args=args)
    node = GripperKeys()
    settings = termios.tcgetattr(sys.stdin)
    print(HELP)

    try:
        # cbreak: read single key presses without Enter, Ctrl+C still works
        tty.setcbreak(sys.stdin)
        while rclpy.ok():
            key = read_key(0.1)
            if key == 'g':
                node.call(node.grab_client)
            elif key == 'r':
                node.call(node.release_client)
            elif key == 'q':
                break
            rclpy.spin_once(node, timeout_sec=0)
    except KeyboardInterrupt:
        pass
    finally:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, settings)
        node.destroy_node()
        rclpy.try_shutdown()


if __name__ == '__main__':
    main()

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    config_file = os.path.join(
        get_package_share_directory("robot_bringup"), "config", "params.yaml"
    )

    # -------------------- pub_cmds --------------------- #
    teleop_twist_keyboard = Node(
        package="teleop_twist_keyboard",
        executable="teleop_twist_keyboard",
        output="screen",
        prefix="xterm -e",
    )

    # --------------------- control --------------------- #
    motor_commands = Node(
        package="usb_bridge",
        executable="motor_commands",
        output="screen",
        parameters=[config_file],
    )

    return LaunchDescription([teleop_twist_keyboard, motor_commands])

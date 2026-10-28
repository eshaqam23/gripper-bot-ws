import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():

    config_file = os.path.join(
        get_package_share_directory('robot_bringup'), 'config', 'params.yaml'
    )

    # port:=... picks the serial port (e.g. port:=/tmp/fake_arduino)
    # or the ESP's WiFi address (e.g. port:=socket://192.168.1.50:3333)
    port = DeclareLaunchArgument(
        'port', default_value='/dev/ttyACM0',
        description='Serial port to the microcontroller',
    )

    # -------------------- pub_cmds --------------------- #
    teleop_twist_keyboard = Node(
        package='teleop_twist_keyboard',
        executable='teleop_twist_keyboard',
        output='screen',
        prefix='xterm -e',
    )

    gripper_keys = Node(
        package='gripper_controller',
        executable='gripper_keys',
        output='screen',
        prefix='xterm -e',
    )

    # --------------------- control --------------------- #
    motor_commands = Node(
        package='esp_bridge',
        executable='motor_commands',
        output='screen',
        parameters=[config_file, {'port': LaunchConfiguration('port')}],
    )

    return LaunchDescription([
        port,
        teleop_twist_keyboard,
        gripper_keys,
        motor_commands,
    ])

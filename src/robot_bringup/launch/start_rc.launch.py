import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, OpaqueFunction
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def launch_setup(context):
    config_file = os.path.join(
        get_package_share_directory('robot_bringup'), 'config', 'params.yaml'
    )

    # port:=... overrides the serial port from params.yaml (e.g. the fake Arduino)
    port = LaunchConfiguration('port').perform(context)
    bridge_params = [config_file] + ([{'port': port}] if port else [])

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
        package='usb_bridge',
        executable='motor_commands',
        output='screen',
        parameters=bridge_params,
    )

    return [teleop_twist_keyboard, gripper_keys, motor_commands]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument(
            'port', default_value='',
            description='Serial port override (default: port in config/params.yaml)',
        ),
        OpaqueFunction(function=launch_setup),
    ])

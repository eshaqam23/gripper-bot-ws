"""
RC Car & Gripper ROS 2 Controller Node
--------------------------------------
Author: Vivian Wright
Date: October 2026
Source / Citation: Developed with assistance from Gemini (Google AI), October 2026.
Original ROS 2 Node Architecture Template generated via AI collaboration.
Modified by: Vivian Wright (October 2026)
"""

#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from std_msgs.msg import Float32MultiArray

class SkeletonNode(Node):
    def __init__(self):
        super().__init__('skeleton_node')
        
        #Declare and get parameters from params.yaml
        self.declare_parameter('message', 'Default hello message')
        startup_message = self.get_parameter('message').value
        self.get_logger().info(f"Param Loaded -> {startup_message}")
        
        #Subscriptions for drive and multi-motor gripper
        self.cmd_vel_sub = self.create_subscription(
            Twist, '/cmd_vel', self.cmd_vel_callback, 10)

        #Utilizing an array for multiple motors    
        self.gripper_sub = self.create_subscription(
            Float32MultiArray, '/gripper_cmd', self.gripper_callback, 10)
            
        self.get_logger().info("Gripper Bot Controller Node Initialized.")

    #Code logic below adapted from Gemini (Google AI) response, October 2026.
    def cmd_vel_callback(self, msg):
        linear = msg.linear.x
        angular = msg.angular.z
        self.get_logger().info(f"[DRIVE] Linear: {linear:.2f}, Angular: {angular:.2f}")

    def gripper_callback(self, msg):
        #Expecting an array for multi-motor gripper (e.g., claw, joint)
        if len(msg.data) >= 2:
            self.get_logger().info(f"[GRIPPER] Motor Angles -> {list(msg.data)}")
            # TODO: Add hardware PWM/GPIO calls here for the physical motors
        else:
            self.get_logger().warning("[GRIPPER] Received invalid motor command array size.")

def main(args=None):
    rclpy.init(args=args)
    node = SkeletonNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
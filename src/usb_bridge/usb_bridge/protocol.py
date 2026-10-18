"""
Serial message format shared with firmware/motor_logic.

One command per line:
    D,leftPwm,leftDir,rightPwm,rightDir   drive (pwm 0-255, dir 1 = forward, 0 = backward)
"""


def drive_message(linear_velocity, angular_velocity, wheel_base, max_speed):
    """Convert a Twist's linear.x / angular.z into a D command line."""
    # calculate velocity per wheel
    left_velocity = linear_velocity - (angular_velocity * wheel_base) / 2
    right_velocity = linear_velocity + (angular_velocity * wheel_base) / 2

    # determine wheel direction
    left_forward = 0 if left_velocity < 0 else 1
    right_forward = 0 if right_velocity < 0 else 1

    # normalize wheel speed between 0 and 255 (max_speed maps to 255)
    left_pwm = min(int(abs(left_velocity) / max_speed * 255), 255)
    right_pwm = min(int(abs(right_velocity) / max_speed * 255), 255)

    return f'D,{left_pwm},{left_forward},{right_pwm},{right_forward}\n'


STOP_MESSAGE = 'D,0,1,0,1\n'

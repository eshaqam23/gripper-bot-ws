#!/usr/bin/env python3
"""
Pretend to be firmware/motor_logic on a virtual serial port.

Lets you run the whole ROS side without hardware:

    python3 tools/fake_arduino.py
    ros2 launch robot_bringup start_rc.launch.py port:=/tmp/fake_arduino

Prints what the motors and gripper would do. Speaks the same protocol as the
firmware (D/G commands, OK/ERR replies, 500 ms drive safety timeout).
"""

import argparse
import os
import pty
import re
import select
import time
import tty

DRIVE = re.compile(r'^D,(-?\d+),(-?\d+),(-?\d+),(-?\d+)$')
GRIPPER = re.compile(r'^G,(-?\d+),(-?\d+)$')

DRIVE_TIMEOUT_S = 0.5  # DRIVE_TIMEOUT_MS in helpers.h
CLAW_OFFSET = 0  # CLAW_OFFSET in helpers.h
CLAW_MIN, CLAW_MAX = -20, 180  # CLAW_MIN/MAX in helpers.h (servo angle, after offset)
JOINT_MIN, JOINT_MAX = 0, 180  # JOINT_MIN/MAX in helpers.h


def clamp(value, low, high):
    return max(low, min(high, value))


def describe_drive(left_pwm, left_dir, right_pwm, right_dir):
    def side(pwm, direction):
        if pwm == 0:
            return 'stopped'
        return f"{'forward' if direction == 1 else 'backward'} {pwm}"
    return f'left {side(left_pwm, left_dir)}, right {side(right_pwm, right_dir)}'


class FakeArduino:

    def __init__(self):
        self.drive = (0, 1, 0, 1)
        self.gripper = (0, 90)
        self.last_drive = time.monotonic()

    def handle(self, line):
        """Process one command line and return the reply."""
        m = DRIVE.match(line)
        if m:
            left_pwm, left_dir, right_pwm, right_dir = (int(g) for g in m.groups())
            drive = (clamp(left_pwm, 0, 255), left_dir, clamp(right_pwm, 0, 255), right_dir)
            if drive != self.drive:
                print(f'[DRIVE]   {describe_drive(*drive)}', flush=True)
            self.drive = drive
            self.last_drive = time.monotonic()
            return 'OK'

        m = GRIPPER.match(line)
        if m:
            claw, joint = (int(g) for g in m.groups())
            servo = clamp(claw + CLAW_OFFSET, CLAW_MIN, CLAW_MAX)
            gripper = (servo, clamp(joint, JOINT_MIN, JOINT_MAX))
            print(f'[GRIPPER] claw {claw} deg (servo {servo} deg), joint {gripper[1]} deg',
                  flush=True)
            self.gripper = gripper
            return 'OK'

        print(f'[ERR]     bad command: {line!r}', flush=True)
        return 'ERR'

    def check_timeout(self):
        moving = self.drive[0] > 0 or self.drive[2] > 0
        if moving and time.monotonic() - self.last_drive > DRIVE_TIMEOUT_S:
            print('[SAFETY]  no drive command for 500 ms, stopping motors', flush=True)
            self.drive = (0, 1, 0, 1)


def main():
    parser = argparse.ArgumentParser(description=__doc__.strip().splitlines()[0])
    parser.add_argument('--link', default='/tmp/fake_arduino',
                        help='symlink to create for the virtual port (default: %(default)s)')
    args = parser.parse_args()

    master, slave = pty.openpty()
    tty.setraw(slave)  # no echo or newline translation, like a real serial port
    port = os.ttyname(slave)

    if os.path.islink(args.link):
        os.remove(args.link)
    os.symlink(port, args.link)

    print(f'Fake Arduino listening on {args.link} -> {port}', flush=True)
    os.write(master, b'Ready\r\n')

    arduino = FakeArduino()
    buffer = b''
    try:
        while True:
            ready, _, _ = select.select([master], [], [], 0.05)
            if ready:
                buffer += os.read(master, 1024)
                while b'\n' in buffer:
                    raw, buffer = buffer.split(b'\n', 1)
                    reply = arduino.handle(raw.decode('utf-8', errors='replace').strip())
                    os.write(master, reply.encode() + b'\r\n')
            arduino.check_timeout()
    except KeyboardInterrupt:
        pass
    finally:
        if os.path.islink(args.link):
            os.remove(args.link)


if __name__ == '__main__':
    main()

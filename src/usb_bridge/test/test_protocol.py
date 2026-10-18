import pytest
from usb_bridge.protocol import drive_message, gripper_message, STOP_MESSAGE

WHEEL_BASE = 0.2032
MAX_SPEED = 0.5


def test_full_forward():
    assert drive_message(0.5, 0.0, WHEEL_BASE, MAX_SPEED) == 'D,255,1,255,1\n'


def test_full_backward():
    assert drive_message(-0.5, 0.0, WHEEL_BASE, MAX_SPEED) == 'D,255,0,255,0\n'


def test_half_forward():
    assert drive_message(0.25, 0.0, WHEEL_BASE, MAX_SPEED) == 'D,127,1,127,1\n'


def test_stopped():
    assert drive_message(0.0, 0.0, WHEEL_BASE, MAX_SPEED) == STOP_MESSAGE


def test_spin_left_in_place():
    # positive angular.z turns left: left wheel backward, right wheel forward
    msg = drive_message(0.0, 2.0, WHEEL_BASE, MAX_SPEED)
    assert msg == 'D,103,0,103,1\n'


def test_speed_is_clamped():
    assert drive_message(5.0, 0.0, WHEEL_BASE, MAX_SPEED) == 'D,255,1,255,1\n'


def test_gripper_message():
    assert gripper_message([20.0, 90.0]) == 'G,20,90\n'


def test_gripper_message_rounds_angles():
    assert gripper_message([44.6, 90.4]) == 'G,45,90\n'


def test_gripper_message_needs_two_angles():
    with pytest.raises(ValueError):
        gripper_message([45.0])

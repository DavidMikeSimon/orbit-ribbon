#!/usr/bin/python

import pygame
from pygame.locals import *

_js = None

# Constants returned by procEvent
AXIS, BUTTON = range(2)
UP, DOWN = range(2)
SELECT, START, LX, LY, L1, L2, L3, RX, RY, R1, R2, R3, UP, LEFT, RIGHT, DOWN, S, X, O, T = range(20)

# Assocation of PS3 button names to button numbers on the PS3 controller.
BUTTON_NUMS = {
	SELECT : 0,
	START : 3,
	L1 : 10,
#	L2 : 8,
	L3 : 1,
	R1 : 11,
#	R2 : 9,
	R3 : 2,
	UP : 4,
	LEFT : 7,
	RIGHT : 5,
	DOWN : 6,
	S : 15,
	X : 14,
	O : 13,
	T : 12,
}
BUTTON_NAMES = dict([ (BUTTON_NUMS[n], n) for n in BUTTON_NUMS.keys() ])

# Assocation of PS3 axis names to axis numbers on the PS3 controller. I consider L2 and R2 to be axes.
AXIS_NUMS = {
	L2 : 12,
	LX : 0,
	LY : 1,
	R2 : 13,
	RX : 2,
	RY : 3,
}
AXIS_NAMES = dict([ (AXIS_NUMS[n], n) for n in AXIS_NUMS.keys() ])


def init():
	"""Must be called before you can start receiving joystick events."""
	global _js
	pygame.joystick.init()
	_js = pygame.joystick.Joystick(0)
	_js.init()


def procEvent(e):
	"""Given a JOYAXISMOTION event, a JOYBUTTONDOWN event, or a JOYBUTTONUP event, returns PS3 controller info.
	
	On JOYAXISMOTION events, returns a tuple like so: (joy.AXIS, axis name string, new axis value as a float in [-1,1])
	On JOYBUTTONDOWN/UP events, returns a tuple like so: (joy.BUTTON, button name string, joy.UP or joy.DOWN)
	Otherwise, returns None if the event wasn't useful or relevant.
	"""
	if e.type == pygame.JOYAXISMOTION:
		if e.axis in AXIS_NAMES:
			return (AXIS, AXIS_NAMES[e.axis], e.value)
	elif e.type == pygame.JOYBUTTONDOWN or e.type == pygame.JOYBUTTONUP:
		if e.button in BUTTON_NAMES:
			value = UP if e.type == pygame.JOYBUTTONUP else DOWN
			return (BUTTON, BUTTON_NAMES[e.button], value)
	return None

def getAxes():
	"""Returns a dictionary mapping axis name to float, describing the current state of all the axes on the joystick."""
	ret = {}
	delta = 0.01 # Dead zone threshold
	for name, num in AXIS_NUMS.iteritems():
		v = _js.get_axis(num)
		if abs(v) >= delta:
			ret[name] = v
		else:
			ret[name] = 0.0
	return ret
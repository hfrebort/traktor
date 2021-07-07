#!/usr/bin/python3

_value = 0

class PWMLED:
    def __init__(self, f):
        self._value = f

class Button:
    is_pressed = False
    def __init__(self, f):
        self._value = f

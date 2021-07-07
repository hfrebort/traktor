#!/usr/bin/python3
from gpiozero import PWMLED, Button
#from demo import PWMLED, Button
import time

lifter = None
detectorLeft = None
detectorRight = None

ledLeft = None
ledRight = None

def stop_all():
    global ledLeft, ledRight
    
    if ledLeft is not None:
      ledLeft.value = 0

    if ledRight is not None:
      ledRight.value = 0

def move(left, right, threshold=30, offset=0):
    if abs(offset) < threshold:
        stop_all()
    else:
        direction = "left" if offset > 0 else "right"
        _execute(0, left, right, direction)

def perform(data):
    print (data)
    
    # 12 .. left 13 .. right
    duration = float(data['duration'])
    left = int(data['left'])
    right = int(data['right'])
    direction = data['direction']

    _execute(duration, left, right, direction)

def centerHarrow(left, right):
  global lifter, detectorLeft, detectorRight
  # lazy initialize of the buttons
  if lifter is None:
    lifter = Button(17)
  if detectorLeft is None:
    detectorLeft = Button(27)
  if detectorRight is None:
    detectorRight = Button(22)

  while lifter.is_pressed:
    if detectorLeft.is_pressed == False and detectorRight.is_pressed == False:
      print ("left and right is false")
      stop_all()
    elif detectorLeft.is_pressed:
      _execute(0, left, right, "left")
      print ("detected left")
    elif detectorRight.is_pressed:
      _execute(0, left, right, "right")
      print ("detected right")
  stop_all()

def _execute(duration, left, right, direction):
    global ledLeft, ledRight
    # initialize the leds    
    if ledLeft is None:
        ledLeft = PWMLED(left)

    if ledRight is None:
        ledRight = PWMLED(right)
    
    # evaluate the direction
    if direction == "left":
        ledRight.value = 0
        ledLeft.value = 1
    elif direction == "right":
        ledLeft.value = 0
        ledRight.value = 1
    else:
        stop_all()
    
    print ("left: %d, right: %d" % (ledLeft.value, ledRight.value))
    
    if direction != "stop" and duration > 0:
        print ("sleep of %s sec was executed " % duration)
        time.sleep(duration)
        stop_all()

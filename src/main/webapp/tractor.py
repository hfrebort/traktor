#!/usr/bin/python3
from gpiozero import PWMLED
from signal import pause
import time
import sys

print (sys.argv)

led = PWMLED(18)
# 0.25 .. left 0.5 .. center 1 .. right
duration = int(sys.argv[1])
left = float(sys.argv[2])
center = float(sys.argv[3])
right = float(sys.argv[4])
direction = sys.argv[5]


if direction == "left":
  led.value = left
elif direction == "center":
  led.value = center
elif direction == "right":
  led.value = right


print ("led.value %s" % (led.value))

if direction != "stop":
  print ("sleep of %s sec was executed " % duration)
  time.sleep(duration)


# stop
led.value = 0

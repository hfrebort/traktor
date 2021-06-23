#!/usr/bin/python3
import time, cv2
import numpy as np
from datetime import datetime

def render(orig, offset=0, threshold=30, markers=0, maxMarkers=10):
    # write offset -> contours
    white = (255,255,255)
    height = int(orig.shape[0] / 3)
    width = int(orig.shape[1] / 3)

    img = np.zeros((height,width,3))
    # statistic rectangle 
    # first line -> warning
    # second line -> offset
    # third line -> arrow (left / right)
    if markers > maxMarkers:
      now = "!!! MANUELL STEUERN !!!"
      img = cv2.putText(img, now, (10,20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,0,255), 2)

    text = f"{offset:+#4.1f}cm {markers:2d}"
    img = cv2.putText(img, text, (10, int(height/2 - 10)), cv2.FONT_HERSHEY_SIMPLEX, 1, white, 2)

    if abs(offset) > threshold:
        # direction depends on offset
        p1 = (width - 10, height - 20)
        p2 = (10, height - 20)
        if offset > 0:
            p1 = (10, height - 20)
            p2 = (width - 10, height - 20)
        img = cv2.arrowedLine(img, p1, p2, white, 3)  

    x_offset=y_offset=10
    orig[y_offset:y_offset+img.shape[0], x_offset:x_offset+img.shape[1]] = img
    return orig
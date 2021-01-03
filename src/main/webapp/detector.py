#!/usr/bin/python3
import cv2, time
import numpy as np


def regionOfInterest(img):
    height = img.shape[0]
    width = img.shape[1]
    region = [
        (0, height),
        (width/2, 0),
        (width, height)
    ]
    vertices = np.array([region], np.int32)
    mask = np.zeros_like(img)
    match_mask_color = 255
    if len(img.shape) > 2:
        channel_cnt = img.shape[2]
        match_mask_color = (255,) * channel_cnt
    cv2.fillPoly(mask, vertices, match_mask_color)
    masked_img = cv2.bitwise_and(img, mask)
    return masked_img

def drawContoursLine(img, contours, color=(0,0,255)):    
    line_image = np.copy(img) * 0
    w,h = 640, 480
    for cnt in contours:
        if cv2.contourArea(cnt) > 10000:
            (vx, vy, cx, cy) = cv2.fitLine(cnt, cv2.DIST_L12, 0, 0.1, 0.1)
            cv2.line(line_image,(int(cx-vx*w),int(cy-vy*h)),(int(cx+vx*w),int(cy+vy*h)),color,2)
    return line_image

def detect(img, filter_green=True, contour_mode='CONT', mode=cv2.RETR_EXTERNAL, method=cv2.CHAIN_APPROX_SIMPLE):    
    gray = None
    if filter_green == True:
        hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        # mask of green (36,25,25) ~ (86, 255,255)
        gray = cv2.inRange(hsv, (36, 25, 25), (100, 255,255))         
    else:
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    
    blur = cv2.GaussianBlur(gray, (3,3), 0)
    thresh = cv2.threshold(blur,0,255, cv2.THRESH_BINARY_INV + cv2.THRESH_OTSU)[1]
    eroded = cv2.erode(thresh, (5,5), iterations=5)
    roi = regionOfInterest(eroded)    
    contours,_ = cv2.findContours(roi, mode, method)
    contour_img = drawContoursLine(img, contours)
    
    return contour_img, len(contours) 

width = 640
height = 480
video_name = 'tests/soja2.mp4'

cap = cv2.VideoCapture(video_name)

while True:
    _, frame = cap.read()

    if frame is not None:
        start = time.perf_counter()

        img = cv2.resize(frame, (width, height))

        detected, _ = detect(img)
        detected = cv2.addWeighted(img, 0.8, detected, 1, 0)
        
        cv2.imshow('orig', img)
        cv2.imshow('detected', detected)

        print(f"Prepare image took: {time.perf_counter() - start:0.3f} ms")
    else:
        cap.release()
        cap = cv2.VideoCapture(video_name)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture 
cap.release()
cv2.destroyAllWindows()
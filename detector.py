#!/usr/bin/python3
import cv2, time, typing
import numpy as np

import util

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

def drawContours(img, contours, color=(0,0,255), contourMode='CONT'):
    height = img.shape[0]
    width = img.shape[1]
    cnt_len = 0
    cont_img = np.copy(img) * 0
    for cnt in contours:
        if cv2.contourArea(cnt) > 130:
            cnt_len += 1
            if contourMode == 'CONT':
                cont_img = cv2.drawContours(cont_img, [cnt], -1, color,2)
            elif contourMode == 'LINE':
                (vx, vy, cx, cy) = cv2.fitLine(cnt, cv2.DIST_L12, 0, 0.1, 0.1)
                cv2.line(cont_img,(int(cx-vx*width),int(cy-vy*height)),(int(cx+vx*width),int(cy+vy*height)),color,2)
            elif contourMode == 'POLY':
                cont_img = cv2.fillPoly(cont_img, [cnt], color)
            elif contourMode == 'RECT':
                x,y,w,h = cv2.boundingRect(cnt)
                cont_img = cv2.rectangle(cont_img,(x,y),(x+w,y+h),color,2)
            else:
                print (f"The mode {contourMode} is not supported")

    return cont_img, cnt_len

def calculateOffset(stats, img, contours, xOfTarget, xRange, threshold=130, color=(0,0,255)):
    offset = 0
    cont_img = np.copy(img) * 0
    centroids = []

    start = time.perf_counter_ns()

    for cnt in contours:
        if cv2.contourArea(cnt) > threshold:
            moments = cv2.moments(cnt, True)
            if moments is not None and moments['m00'] > 0:
                x1 = int(moments['m10']/moments['m00'])
                y1 = int(moments['m01']/moments['m00'])
                cont_img = cv2.drawMarker(cont_img, (x1, y1), color, markerType=cv2.MARKER_CROSS, thickness=3)
                if abs(xOfTarget - x1) < xRange:
                    centroids.append((x1,y1))

    now = time.perf_counter_ns()
    stats.moments = now - start

    start = now
    if centroids:
        points = np.array(centroids)
        # points order by y-coordinate 
        ySorted = points[np.argsort(points[:, 1]), :]
        # get the x-coordinate of the countour on the bottom of the image
        xOfYmax,_ = ySorted[len(ySorted) -1]
        offset = int(xOfTarget - xOfYmax) / 12.8 # relation between width and cm
        xOfYmin,_ = ySorted[0]
        points = np.array((xOfYmin, 0))
        ySorted = np.vstack((points, ySorted, np.array((xOfYmax, img.shape[0]))))
        cont_img = cv2.polylines(cont_img, np.int32([ySorted]), isClosed = False, color = (0,255,255), thickness = 2)

    now = time.perf_counter_ns()
    stats.centroids = now - start

    return cont_img, offset, len(centroids)

def detect(stats, img, filterColorRange=True, colorFrom=(36, 25, 25), colorTo=(110, 255,255), errode=5, dilate=5, contourMode='CONT'):
    gray = None

    duration_ns = 0
    now = 0

    if filterColorRange == True:
        hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        # mask of green (36,25,25) ~ (86, 255,255)
        stats.inRange, now, gray = util.duration_ns( lambda: cv2.inRange(hsv, colorFrom, colorTo), now )
    else:
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    
    #blur = cv2.GaussianBlur(gray, (5,5), 0)
    stats.GaussianBlur, now, blur = util.duration_ns( lambda: cv2.GaussianBlur(gray, (5,5), 0), now )

    #thresh = cv2.threshold(blur,0,255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)[1]
    stats.threshold, now, thresh = util.duration_ns( lambda: cv2.threshold(blur,0,255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)[1], now )

    #eroded = cv2.erode(thresh, (5,5), iterations=errode)
    #dilated = cv2.dilate(eroded, (5,5), iterations=dilate)

    stats.erode,  now, eroded  = util.duration_ns( lambda: cv2.erode (thresh, (5,5), iterations=errode), now )
    stats.dilate, now, dilated = util.duration_ns( lambda: cv2.dilate(eroded, (5,5), iterations=dilate), now )

    #roi = regionOfInterest(eroded)
    start = now
    contours,_ = cv2.findContours(dilated, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
    now = time.perf_counter_ns()
    stats.findContours = now - start
    
    start = now
    contourImg1, _ = drawContours(img, contours, contourMode=contourMode)
    width = img.shape[1]
    now = time.perf_counter_ns()
    stats.drawContours = now - start

    contourImg2, offset, markers = calculateOffset(stats, img, contours, width/2, width/(2*3))

    start = time.perf_counter_ns()

    contourImg = cv2.addWeighted(img, 1, contourImg1, 1, 0)
    contourImg = cv2.addWeighted(contourImg, 1, contourImg2, 1, 0)
    
    stats.addWeighted = time.perf_counter_ns() - start

    return contourImg, offset, markers

#!/usr/bin/python3
import sys, time, uuid, cv2
import urllib.request
import numpy as np

def create_image(data):
    start = time.perf_counter()
    imgResp = urllib.request.urlopen(data['url'] + '/shot.jpg')
    imgNp = np.array(bytearray(imgResp.read()),dtype=np.uint8)
    # Decode the array to OpenCV usable format
    img = cv2.imdecode(imgNp,-1)
    
    # get the gray image and process GaussianBlur
    gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
    
    kernel_size = 5
    blur_gray = cv2.GaussianBlur(gray,(kernel_size, kernel_size),0)
    # process edge detection
    low_threshold = int(data['cannyLowThreshold'])
    high_threshold = int(data['cannyHighThreshold'])
    edges = cv2.Canny(blur_gray, low_threshold, high_threshold)
    
    # use HoughLinesP to get the lines. You can adjust the parameters for better performance.
    rho = 1  # distance resolution in pixels of the Hough grid
    theta = np.pi / 180  # angular resolution in radians of the Hough grid
    threshold = int(data['houghThreshold'])  # minimum number of votes (intersections in Hough grid cell)
    min_line_length = int(data['houghMinLineLength'])  # minimum number of pixels making up a line
    max_line_gap = int(data['houghMaxLineGap'])  # maximum gap in pixels between connectable line segments
    line_image = np.copy(img) * 0  # creating a blank to draw lines on
    
    # Run Hough on edge detected image
    # Output "lines" is an array containing endpoints of detected line segments
    lines = cv2.HoughLinesP(edges, rho, theta, threshold, np.array([]), min_line_length, max_line_gap)
    if lines is not None:
        for line in lines:
            for x1,y1,x2,y2 in line:
                cv2.line(line_image,(x1,y1),(x2,y2),(0,0,255),5)
    	# Draw the lines on the  image
        lines_edges = cv2.addWeighted(img, 0.8, line_image, 1, 0)

        cv2.imwrite('static/tmp/snapShot.jpg', lines_edges)
        print(f"Create and write image: {time.perf_counter() - start:0.2f} ms\n")
    else:	
        print('nolines')

#!/usr/bin/python3
import sys
import urllib.request
import cv2
import numpy as np
import uuid

imgResp = urllib.request.urlopen(sys.argv[1] + 'shot.jpg')
imgNp = np.array(bytearray(imgResp.read()),dtype=np.uint8)
# Decode the array to OpenCV usable format
img = cv2.imdecode(imgNp,-1)

# get the gray image and process GaussianBlur
gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)

kernel_size = 5
blur_gray = cv2.GaussianBlur(gray,(kernel_size, kernel_size),0)

# process edge detection
low_threshold = int(sys.argv[2])
high_threshold = int(sys.argv[3])
edges = cv2.Canny(blur_gray, low_threshold, high_threshold)

# use HoughLinesP to get the lines. You can adjust the parameters for better performance.
rho = 1  # distance resolution in pixels of the Hough grid
theta = np.pi / 180  # angular resolution in radians of the Hough grid
threshold = int(sys.argv[4])  # minimum number of votes (intersections in Hough grid cell)
min_line_length = int(sys.argv[5])  # minimum number of pixels making up a line
max_line_gap = int(sys.argv[6])  # maximum gap in pixels between connectable line segments
line_image = np.copy(img) * 0  # creating a blank to draw lines on

# Run Hough on edge detected image
# Output "lines" is an array containing endpoints of detected line segments
lines = cv2.HoughLinesP(edges, rho, theta, threshold, np.array([]),
                    min_line_length, max_line_gap)
if lines is not None:
	for line in lines:
		for x1,y1,x2,y2 in line:
			cv2.line(line_image,(x1,y1),(x2,y2),(255,0,0),5)
	# Draw the lines on the  image
	lines_edges = cv2.addWeighted(img, 0.8, line_image, 1, 0)

	imageName = 'tmp/' +  str(uuid.uuid4()) + '.jpg'
	cv2.imwrite(imageName, lines_edges)
	print(imageName)
else:	
	print('nolines')

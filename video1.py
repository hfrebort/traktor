#!/usr/bin/python3
import cv2, time
import numpy as np

def region_of_interest(img):
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

def find_green(img):
    global green_range
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    ## mask of green (36,25,25) ~ (86, 255,255)
    mask = cv2.inRange(hsv, (36, 25, 25), (green_range, 255,255))
    ## slice the green
    imask = mask>0
    green = np.zeros_like(img, np.uint8)
    green[imask] = img[imask]    
    return green

def find_vertical_lines(img):    
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    black_and_white = cv2.inRange(hsv, (36, 25, 25), (100, 255,255))
    new_img = cv2.erode(black_and_white, (7,7), iterations=50)
    
    contours,_ = cv2.findContours(new_img, 1, 2)    
    line_img = drawContoursLine(new_img, contours)
    return line_img

def hough_lines_p(img):
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    black_and_white = cv2.inRange(hsv, (36, 25, 25), (100, 255,255))
    gray = cv2.erode(black_and_white, (7,7), iterations=15)
    
    #gray = cv2.cvtColor(img,cv2.COLOR_BGR2GRAY)
    edges = cv2.Canny(gray, 500,550, apertureSize = 5)
    theta = np.pi / 180  # angular resolution in radians of the Hough grid
    line_image = np.copy(img) * 0  # creating a blank to draw lines on   
    lines = cv2.HoughLinesP(edges, 1, theta, 10, np.array([]), 5, 350)
    if lines is not None:
        for line in lines:
            for x1,y1,x2,y2 in line:
                cv2.line(line_image,(x1,y1),(x2,y2),(0,0,255),5)
    return line_image

def hough_lines(img):
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    black_and_white = cv2.inRange(hsv, (36, 25, 25), (100, 255,255))
    gray = cv2.erode(black_and_white, (7,7), iterations=15)

    edges = cv2.Canny(gray, 500,550,apertureSize = 3)

    start = time.perf_counter()
    lines = cv2.HoughLines(edges, 1,np.pi/180, 200, max_theta=350)
    print(f"hough lines took: {time.perf_counter() - start:0.3f} ms")
    line_image = np.copy(img) * 0
    if lines is not None:
        for rho,theta in lines[0]:
            a = np.cos(theta)
            b = np.sin(theta)
            x0 = a*rho
            y0 = b*rho
            x1 = int(x0 + 1000*(-b))
            y1 = int(y0 + 1000*(a))
            x2 = int(x0 - 1000*(-b))
            y2 = int(y0 - 1000*(a))
            cv2.line(line_image,(x1,y1),(x2,y2),(0,0,255),2)

    return line_image

def drawContoursLine(img, contours, color=(0,0,255)):
    global area_thresh
    line_image = np.copy(img) * 0
    for cnt in contours:
        if cv2.contourArea(cnt) > area_thresh:
            moments = cv2.moments(cnt, True)
            if moments is not None and moments['m00'] > 0:
                x1 = int(moments['m10']/moments['m00'])
                y1 = int(moments['m01']/moments['m00'])

                x,y,w,h = cv2.boundingRect(cnt)
                x2, y2 = (int(x + w/2), int(y - h/2))

                line_image = cv2.line(line_image,(x1,y1),(x2,y2),color,5)
    return line_image

def drawContours(img, contours, color=(0,0,255), contour_mode='CONT'):
    global area_thresh

    cnt_len = 0
    cont_img = np.copy(img) * 0
    for cnt in contours:
        if cv2.contourArea(cnt) > area_thresh:
            cnt_len += 1
            if contour_mode == 'CONT':
                cont_img = cv2.drawContours(cont_img, [cnt], -1, color,2)
            elif contour_mode == 'LINE':
                (vx, vy, cx, cy) = cv2.fitLine(cnt, cv2.DIST_L12, 0, 0.1, 0.1)
                cv2.line(cont_img,(int(cx-vx*width),int(cy-vy*height)),(int(cx+vx*width),int(cy+vy*height)),color,2)
            elif contour_mode == 'POLY':
                cont_img = cv2.fillPoly(cont_img, [cnt], color)
            elif contour_mode == 'RECT':
                x,y,w,h = cv2.boundingRect(cnt)
                cont_img = cv2.rectangle(cont_img,(x,y),(x+w,y+h),color,2)
            else:
                print (f"The mode {contour_mode} is not supported")

    return cont_img, cnt_len

def kMeans(img):    
    # k-means 
    Z = img.reshape((-1,3))
    # convert to np.float32
    Z = np.float32(Z)
    # define criteria, number of clusters(K) and apply kmeans()
    criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 10, 1.0)
    K = 3
    ret,label,center=cv2.kmeans(Z,K,None,criteria,10,cv2.KMEANS_RANDOM_CENTERS)
    # Now convert back into uint8, and make original image
    center = np.uint8(center)
    res = center[label.flatten()]
    res2 = res.reshape((img.shape))
    return res2

def search_lines(img, filter_green=True, contour_mode='CONT', mode=cv2.RETR_EXTERNAL, method=cv2.CHAIN_APPROX_SIMPLE):    
    global erode_iteration, dilate_iteration, green_range
    gray = None
    if filter_green == True:
        hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        # mask of green (36,25,25) ~ (86, 255,255)
        gray = cv2.inRange(hsv, (36, 25, 25), (green_range, 255,255))         
    else:
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    blur = cv2.GaussianBlur(gray, (5,5), 0)
    thresh = cv2.threshold(blur,0,255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)[1]
    eroded = cv2.erode(thresh, (5,5), iterations=erode_iteration)
    dilated = cv2.dilate(eroded, (5,5), iterations=dilate_iteration)

    contours,_ = cv2.findContours(dilated, mode, method)
    contour_img, cnt_len = drawContours(img, contours, (0,0,255), contour_mode)
    #contour_img = region_of_interest(contour_img)
    #contour_img = kMeans(contour_img)
    return contour_img, cnt_len 

def draw_middle_line(img):
    half_width = int(img.shape[1]/2)
    cv2.line(img,(half_width, img.shape[0]),(half_width,0),(255,0,0),3)

def draw_on_frame(src_img, calc_img):
    img = cv2.addWeighted(src_img, 0.8, calc_img, 1, 0)
    draw_middle_line(img)
    return img

width, height = 640, 480

def on_erode_trackbar(val):
    global erode_iteration
    erode_iteration = val

def on_dilate_trackbar(val):
    global dilate_iteration
    dilate_iteration = val

def on_green_trackbar(val):
    global green_range
    green_range = val

def on_area_trackbar(val):
    global area_thresh
    area_thresh = val

cv2.namedWindow('Trackbar')
cv2.createTrackbar('Eroded', 'Trackbar', 10, 100, on_erode_trackbar)
cv2.createTrackbar('Dilated', 'Trackbar', 20, 100, on_dilate_trackbar)
cv2.createTrackbar('Green', 'Trackbar', 110, 255, on_green_trackbar)
cv2.createTrackbar('Area', 'Trackbar', 130, 2000, on_area_trackbar)

on_erode_trackbar(10)
on_dilate_trackbar(20)
on_green_trackbar(110)
on_area_trackbar(130)

video_name = 'tests/lavendel1.mp4'
cap = cv2.VideoCapture(video_name)
#cap = cv2.VideoCapture(0)

default_img = None
while(True):
     # Capture frame-by-frame
    ret, frame = cap.read()

    # Display the resulting frame
    if frame is not None:
        start = time.perf_counter()
        img = cv2.resize(frame, (width, height))
        #img = cv2.rotate(img, cv2.ROTATE_90_CLOCKWISE)

        

        default_img, default_cnt = search_lines(img, contour_mode='LINE')        
        default_img = draw_on_frame(img, default_img)

        fill_img, fill_cnt = search_lines(img, True, 'POLY') #, mode=cv2.RETR_TREE, method=cv2.CHAIN_APPROX_NONE)
        fill_img = draw_on_frame(img, fill_img)

        rect_img, rect_cnt = search_lines(img, True, 'RECT')
        rect_img = draw_on_frame(img, rect_img)

        cv2.imshow('default', default_img)
        cv2.imshow('fill', fill_img)
        cv2.imshow('rectangle', rect_img)
        
        print(f"Prepare image took: {time.perf_counter() - start:0.4f} s - first:{default_cnt} | second:{fill_cnt} | third:{rect_cnt}")
#        print(f"Prepare image took: {time.perf_counter() - start:0.3f} s - found contours: {default_cnt}")
    else:
        cap.release()
        cap = cv2.VideoCapture(video_name)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture 
cap.release()
cv2.destroyAllWindows()
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
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    ## mask of green (36,25,25) ~ (86, 255,255)
    mask = cv2.inRange(hsv, (36, 25, 25), (100, 255,255))
    ## slice the green
    imask = mask>0
    green = np.zeros_like(img, np.uint8)
    green[imask] = img[imask]    
    return green

def find_contours(img):
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    _,thresh = cv2.threshold(gray,240,255,0)
    contours,_ = cv2.findContours(thresh, 1, 2)
    rect_img = drawContoursRectangle(img, contours)
    return rect_img

def find_vertical_lines(img):    
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    black_and_white = cv2.inRange(hsv, (36, 25, 25), (100, 255,255))
    new_img = cv2.erode(black_and_white, (7,7), iterations=50)
    
    contours,_ = cv2.findContours(new_img, 1, 2)    
    line_img = drawContoursLine(new_img, contours)
    return line_img

def drawContoursLine(img, contours, color=(0,0,255)):    
    line_image = np.copy(img) * 0
    w,h = 640, 480
    for cnt in contours:
        if cv2.contourArea(cnt) > 1800:
            (vx, vy, cx, cy) = cv2.fitLine(cnt, cv2.DIST_L12, 0, 0.1, 0.1)
            cv2.line(line_image,(int(cx-vx*w),int(cy-vy*h)),(int(cx+vx*w),int(cy+vy*h)),color,2)
    return line_image
#        moments = cv2.moments(cnt, True)
#        #print(f'Moment: {moments}')
#        if moments is not None and moments['m00'] > 0:
#            x1 = int(moments['m10']/moments['m00'])
#            y1 = int(moments['m01']/moments['m00'])
#
#            x,y,w,h = cv2.boundingRect(cnt)
#            x2, y2 = (int(x + w/2), int(y - h/2))

#            line_image = cv2.line(line_image,(x1,y1),(x2,y2),color,5)
    return line_image

def drawContoursRectangle(img, contours, color=(0,0,255)):
    rect_img = np.copy(img) * 0
    for cnt in contours:
        x,y,w,h = cv2.boundingRect(cnt)
        rect_img = cv2.rectangle(rect_img,(x,y),(x+w,y+h),color,2)
    return rect_img

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

def search_lines(img, filter_green=True, contour_mode='CONT', mode=cv2.RETR_EXTERNAL, method=cv2.CHAIN_APPROX_SIMPLE):    
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
    
    contours,_ = cv2.findContours(eroded, mode, method)
    contour_img = np.copy(img) * 0
    if contour_mode == 'RECT':
        contour_img = drawContoursRectangle(img, contours, (255,0,255))
    elif contour_mode == 'LINE':
        contour_img = drawContoursLine(img, contours)
    elif contour_mode == 'FILL':
        contour_img = cv2.fillPoly(contour_img, contours, (0,0,255))
    else:
        cv2.drawContours(contour_img, contours, -1, (0,0,255), 2)

    #contour_img = region_of_interest(contour_img)
    return contour_img, len(contours) 

def draw_middle_line(img):
    half_width = int(img.shape[1]/2)
    cv2.line(img,(half_width, img.shape[0]),(half_width,0),(255,0,0),3)

def draw_on_frame(src_img, calc_img):
    img = cv2.addWeighted(src_img, 0.8, calc_img, 1, 0)
    draw_middle_line(img)
    return img

video_name = 'tests/soja2.mp4'

cap = cv2.VideoCapture(video_name)

default_img = None
while(True):
     # Capture frame-by-frame
    ret, frame = cap.read()

    # Display the resulting frame
    if frame is not None:
        start = time.perf_counter()
        img = cv2.resize(frame, (640, 480))

        fill_img, fill_cnt = search_lines(img, True, 'FILL', mode=cv2.RETR_TREE, method=cv2.CHAIN_APPROX_NONE)
        fill_img = draw_on_frame(img, fill_img)

        default_img, default_cnt = search_lines(img)        
        default_img = draw_on_frame(img, default_img)

        fitline_img, fitline_cnt = search_lines(img, True, 'LINE') #, mode=cv2.RETR_TREE, method=cv2.CHAIN_APPROX_NONE)
        fitline_img = draw_on_frame(img, fitline_img)

        cv2.imshow('fillPoly', fill_img)
        cv2.imshow('contours', default_img)
        cv2.imshow('fitLine', fitline_img)
        
        print(f"Prepare image took: {time.perf_counter() - start:0.3f} ms - first:{fill_cnt} | second:{default_cnt} | third:{fitline_cnt}")
        #print(f"Prepare image took: {time.perf_counter() - start:0.3f} ms - fitline:{fitline_cnt}")
    else:
        cap.release()
        cap = cv2.VideoCapture(video_name)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture 
cap.release()
cv2.destroyAllWindows()
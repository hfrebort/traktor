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
    rect_img = np.copy(img) * 0

    for cnt in contours:
        x,y,w,h = cv2.boundingRect(cnt)
        rect_img = cv2.rectangle(rect_img,(x,y),(x+w,y+h),(0,255,0),20)
    return rect_img

def find_vertical_lines(img):    
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
    black_and_white = cv2.inRange(hsv, (36, 25, 25), (100, 255,255))
    new_img = cv2.erode(black_and_white, (7,7), iterations=50)
    
    contours,_ = cv2.findContours(new_img, 1, 2)    
    line_image = np.copy(img) * 0

    x2, y2 = (0, 0)
    for cnt in contours:
        x,y,w,h = cv2.boundingRect(cnt)
        x1, y1 = (int(x + w/2), int(y + h/2))
        if x2 != 0 and y2 != 0:
            line_image = cv2.line(line_image,(x1,y1),(x2,y2),(0,0,255),5)
        x2, y2 = (x1, y1)
    return line_image

def draw_lines_with_contours(img):    
    contours,_ = cv2.findContours(img, 1, 2)
    line_image = np.copy(img) * 0

    x2, y2 = (0, 0)
    for cnt in contours:
        x,y,w,h = cv2.boundingRect(cnt)
        x1, y1 = (int(x + w/2), int(y + h/2))
        if x2 != 0 and y2 != 0:
            line_image = cv2.line(line_image,(x1,y1),(x2,y2),(0,0,255),5)
        x2, y2 = (x1, y1)
    return line_image

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

def search_lines(img, filter_green=True, mode=cv2.RETR_EXTERNAL, method=cv2.CHAIN_APPROX_SIMPLE):    
    gray = None
    if filter_green == True:
        hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)
        # mask of green (36,25,25) ~ (86, 255,255)
        gray = cv2.inRange(hsv, (36, 25, 25), (100, 255,255))         
    else:
        gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)

    new_img = cv2.GaussianBlur(gray, (3,3), 0)
    new_img = cv2.threshold(new_img,0,255, cv2.THRESH_BINARY + cv2.THRESH_OTSU)[1]
    new_img = cv2.erode(new_img, (5,5), iterations=5)
    
    contours,_ = cv2.findContours(new_img, mode, method)      
    contour_img = np.copy(img) * 0
    cv2.drawContours(contour_img, contours, -1, (0,0,255), 2)

    contour_img = region_of_interest(contour_img)
    return contour_img, len(contours) 

def detect_feature(img):
    #new_img = find_green(img)
    #new_img = find_vertical_lines(img)
    #new_img = find_contours(new_img)
    #new_img = hough_lines(img)
    #new_img = hough_lines_p(img)    
    #new_img = region_of_interest(new_img)
    new_img = search_lines(img)
    return new_img

def draw_middle_line(img):
    half_width = int(img.shape[1]/2)
    cv2.line(img,(half_width, img.shape[0]),(half_width,0),(255,0,0),3)

def draw_on_frame(src_img, calc_img):
    img = cv2.addWeighted(src_img, 0.8, calc_img, 1, 0)
    draw_middle_line(img)
    return img

video_name = 'tests/soja1.mp4'

cap = cv2.VideoCapture(video_name)

lines_img = None
while(True):
     # Capture frame-by-frame
    ret, frame = cap.read()

    # Display the resulting frame
    if frame is not None:
        start = time.perf_counter()
        img = cv2.resize(frame, (640, 480))

        first_img, first_cnt = search_lines(img, True, cv2.RETR_TREE,cv2.CHAIN_APPROX_NONE)
        first_img = draw_on_frame(img, first_img)

        lines_img, lines_cnt = search_lines(img)        
        lines_img = draw_on_frame(img, lines_img)

        cv2.imshow('video1', first_img)
        cv2.imshow('video2', lines_img)
        #time.sleep(0.5)
        print(f"Prepare image took: {time.perf_counter() - start:0.3f} ms - first:{first_cnt} | second:{lines_cnt}")
    else:
        cap.release()
        cap = cv2.VideoCapture(video_name)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture 
cap.release()
cv2.destroyAllWindows()
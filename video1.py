#!/usr/bin/python3
import cv2, time
import numpy as np
import detector, render

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

def drawContours(img, contours, color=(0,0,255), contour_mode='CONT'):
    global area_thresh, width, height

    cnt_len = 0
    cont_img = np.copy(img) * 0
    centroids = []
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
            elif contour_mode == 'MARK':
                moments = cv2.moments(cnt, True)
                if moments is not None and moments['m00'] > 0:
                    x1 = int(moments['m10']/moments['m00'])
                    y1 = int(moments['m01']/moments['m00'])
                    cont_img = cv2.drawMarker(cont_img, (x1, y1), color, markerType=cv2.MARKER_CROSS, thickness=5)
                    # draw line from the marker to the 'target'-line but only if in the same column!!!
                    # FIXME: the two magic numbers 2 and 3 represent 
                    # 2 .. how to get the middle of the image and 
                    # 3 .. the column count
                    if abs(width/2 - x1) < width/(2*3):
                        cont_img = cv2.line(cont_img, (x1, y1), (int(width/2), y1), (0,255,0), thickness=4)
                        centroids.append((x1,y1))
            else:
                print (f"The mode {contour_mode} is not supported")

    if len(centroids) > 0:
        # add points with the first x-coordinate 
        # to draw line from the bottom to the top of the image
        points = np.array(centroids)
        ySorted = points[np.argsort(points[:, 1]), :]
        xOfYmin,_ = ySorted[0]
        xOfYmax,_ = ySorted[len(ySorted) -1 ]
        points = np.array((xOfYmin, 0))
        ySorted = np.vstack((points, ySorted, np.array((xOfYmax, height))))

        offset = int(width/2 - xOfYmax)
        cont_img = cv2.polylines(cont_img, np.int32([ySorted]), isClosed = False,color = (0,255,255),thickness = 2)
        #cont_img = render.render(cont_img, offset)

    return cont_img, cnt_len

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
    #contour_img = drawCentroids(img, contours, (255,0,255))
    #contour_img = region_of_interest(contour_img)

    return contour_img, cnt_len

def draw_middle_line(img):
    half_width = int(img.shape[1]/2)
    cv2.line(img,(half_width, img.shape[0]),(half_width,0),(0,255,0),1)

def draw_on_frame(src_img, calc_img):
    img = cv2.add(src_img, calc_img)
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

on_erode_trackbar(0)
on_dilate_trackbar(0)
on_green_trackbar(0)
on_area_trackbar(0)

video_name = 'tests/00034.mts'
index = 0
cap = cv2.VideoCapture(video_name)

#cap = cv2.VideoCapture(0)

default_img = None
while(True):
    # Capture frame-by-frame
    ret, frame = cap.read()
    #frame = cv2.imread(f"tests/soja{index}.jpeg")
    # Display the resulting frame
    if frame is not None:
        start = time.perf_counter()
        img = cv2.resize(frame, (width, height))
        #img = cv2.rotate(img, cv2.ROTATE_90_CLOCKWISE)

        #default_img, offset = search_lines(img, contour_mode='MARK')
        default_img, offset, markers = detector.detect(img, errode=erode_iteration, dilate=dilate_iteration, contourMode='POLY')
        default_img = render.render(default_img, offset, 5, markers, 10)

        #default_img = draw_on_frame(img, default_img)
        cv2.imshow('default', default_img)
        print(f"offset: {offset:+#4.1f}, marks: {markers:+#2} took: {int((time.perf_counter() - start)*1000)} ms")
    else:
        cap.release()
        cap = cv2.VideoCapture(video_name)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# When everything done, release the capture 
cap.release()
cv2.destroyAllWindows()
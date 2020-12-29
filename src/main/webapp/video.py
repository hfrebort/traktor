#!/usr/bin/python3
import cv2
import numpy as np

isStreaming = True

def video_on_off(data):
    global isStreaming
    if isStreaming == True:
        release_video()
    else:
        capture_video(data)

def release_video():
    global isStreaming
    isStreaming = False

def capture_video(data):
    global isStreaming
    isStreaming = True
    # Open a sample video available in sample-videos
    vcap = cv2.VideoCapture(data['url'] + '/video')
    #if not vcap.isOpened():
    #    print "File Cannot be Opened"
    
    while(isStreaming):
        # Capture frame-by-frame
        ret, frame = vcap.read()
        #print cap.isOpened(), ret
        if frame is not None:
            # Display the resulting frame
            cv2.imwrite('tmp/frame.mjpg',frame)
        else:
            print ('Frame is None')
            break
    
    # When everything done, release the capture
    vcap.release()
    print ('Video stop')
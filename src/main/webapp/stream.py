#!/usr/bin/python3
import cv2, threading, time, detector

_outputFrame = None
_url = 'http://10.3.141.165:8888/video'
_destroyed = False
_lock = threading.Lock()

def detect():
    global _outputFrame, _lock, _url, _destroyed
    _destroyed = False

    vcap = cv2.VideoCapture(_url)
    while not _destroyed:
        frame = vcap.read()        
                
        if frame is not None:            
            image = cv2.resize(frame[1], (320, 240))
            detected, _ = detector.detect(image, contour_mode='POLY')
            newImage = cv2.addWeighted(image, 0.8, detected, 1, 0)
            with _lock:                
                _outputFrame = newImage 
        else:
            print("frame is none")
    vcap.release()

def generate():
    global _outputFrame, _lock, _destroyed

    while not _destroyed:
        time.sleep(0.0001)
        with _lock:
            if _outputFrame is None:
                continue
            
            (flag, encodedImage) = cv2.imencode(".jpg", _outputFrame)

            if not flag:
                continue

            yield(b'--Ba4oTvQMY8ew04N8dcnM\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(encodedImage) + b'\r\n')

def destroy():
    global _destroyed
    _destroyed = True

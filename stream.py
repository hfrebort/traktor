#!/usr/bin/python3
import cv2, threading, time, detector, render, tractor

_data = {'url': 'http://10.3.141.165:8888/video', 'detecting': True, 'threshold': 30, 'colorFilter': True, 'colorFrom': (36, 25, 25), 'colorTo': (110, 255,255), 'erode': 5, 'dilate': 5, 'contourMode': 'CONT'}
_running = True
_outputFrame = None
_lock = threading.Lock()

def apply(data):
    global _data, _lock

    time.sleep(0.0001)
    with _lock:
        _data = data
        _data['colorFrom'] = tuple(map(int, data['colorFrom'].split(',')))
        _data['colorTo'] = tuple(map(int, data['colorTo'].split(',')))
        if data['url'].isdigit():
            _data['url'] = int(data['url'])
        if isinstance(data['erode'], str):
            _data['erode'] = int(data['erode'])
        if isinstance(data['dilate'], str):
            _data['dilate'] = int(data['dilate'])

def read():
    global _outputFrame, _lock, _data, _running

    try:
        _running = True
        vcap = cv2.VideoCapture(0) #_data['url'])
        while _running:
            frame = vcap.read()
            if frame is not None and frame[1] is not None:
                image = frame[1] #cv2.resize(frame[1], (320, 240))
                if _data['detecting'] == True:
                    start = time.perf_counter()
                    image, offset = detector.detect(image, _data['colorFilter'], _data['colorFrom'], _data['colorTo'], _data['erode'], _data['dilate'], _data['contourMode'])
                    image = render.render(image, offset, _data['threshold'])
                    print(f"detecting took: {time.perf_counter() - start:0.3f} s")
                    tractor.move(_data['left'], _data['right'], _data['threshold'], offset)
                with _lock:
                    _outputFrame = image
            else:
                print("frame is none")
    finally:
        print("something wrong")
        vcap.release()

def generate():
    global _outputFrame, _lock, _running

    while _running:
        time.sleep(0.0001)
        with _lock:
            if _outputFrame is None:
                continue

            (flag, encodedImage) = cv2.imencode(".jpg", _outputFrame)

            if not flag:
                continue

            yield(b'--Ba4oTvQMY8ew04N8dcnM\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(encodedImage) + b'\r\n')

def end():
    global _running
    _running = False
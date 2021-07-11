#!/usr/bin/python3
import cv2, threading, time, detector, render, tractor

_data = {   'url'               : 'http://10.3.141.165:8888/video'
            , 'detecting'       : False
            , 'threshold'       : 30
            , 'colorFilter'     : True
            , 'colorFrom'       : (36, 25, 25)
            , 'colorTo'         : (110, 255,255)
            , 'erode'           : 5
            , 'dilate'          : 5
            , 'contourMode'     : 'CONT'
            , 'left'            : 12
            , 'right'           : 13
            , 'maximumMarkers'  : 10}

class Stats:
    read_ns = 0
    detect_ns = 0
    render_ns = 0
    move_ns = 0
    lock_ns = 0
    fps = 0
    jpg_ns = 0
    jpg_bytes = 0

_running = True
_outputFrame = None
_lock = threading.Lock()
_imageReady = threading.Event()
_stats = Stats()

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
        if isinstance(data['threshold'], str):
            _data['threshold'] = int(data['threshold'])
        if isinstance(data['maximumMarkers'], str):
            _data['maximumMarkers'] = int(data['maximumMarkers'])

def getData():
  global _data
  return _data

def ms_from_ns(ns):
    return ns / 1000000

def printStats():
    global _stats
    while True:
        print("read detect render move lock jpg fps jpg-kB   {0:8.1f} {1:8.1f} {2:8.1f} {3:8.1f} {4:8.1f} {5:8.1f} {6:4} {7}"
        .format(
                ms_from_ns(_stats.read_ns)
            ,   ms_from_ns(_stats.detect_ns)
            ,   ms_from_ns(_stats.render_ns)
            ,   ms_from_ns(_stats.move_ns)
            ,   ms_from_ns(_stats.lock_ns)
            ,   ms_from_ns(_stats.jpg_ns)
            ,   _stats.fps
            ,   _stats.jpg_bytes // 1024          ))
                
        _stats.fps = 0
        time.sleep(1.0)

def read():
    global _outputFrame, _lock, _data, _running, _imageReady

    _imageReady.clear()
    threading.Thread(target=printStats).start()

    try:
        _running = True
        vcap = cv2.VideoCapture(0)

        #vcap.set( cv2.CAP_PROP_FPS, 15 )
        vcap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
        #vcap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))
        #vcap.set(cv2.CAP_PROP_FRAME_WIDTH, 320)
        #vcap.set(cv2.CAP_PROP_FRAME_HEIGHT, 240)

        print(f"CAP_PROP_BUFFERSIZE   : {vcap.get(cv2.CAP_PROP_BUFFERSIZE)}")
        print(f"CAP_PROP_FPS          : {vcap.get(cv2.CAP_PROP_FPS)}")
        print(f"CAP_PROP_FRAME_WIDTH  : {vcap.get(cv2.CAP_PROP_FRAME_WIDTH)}")
        print(f"CAP_PROP_FRAME_HEIGHT : {vcap.get(cv2.CAP_PROP_FRAME_HEIGHT)}")

        _stats.camera_width = vcap.get(cv2.CAP_PROP_FRAME_WIDTH)
        _stats.camera_height = vcap.get(cv2.CAP_PROP_FRAME_HEIGHT)
        _stats.camera_fps = vcap.get(cv2.CAP_PROP_FPS)

        while _running:

            tractor.centerHarrow(_data['left'], _data['right'])

            start_ns = time.perf_counter_ns()
            bReadSuccess, frame = vcap.read()
            now_ns = time.perf_counter_ns()
            _stats.read_ns = now_ns - start_ns 

            if bReadSuccess:
                
                start_ns = now_ns
                image, offset, markers = detector.detect(frame, _data['colorFilter'], _data['colorFrom'], _data['colorTo'], _data['erode'], _data['dilate'], _data['contourMode'])
                now_ns = time.perf_counter_ns()
                _stats.detect_ns = now_ns - start_ns                             

                start_ns = now_ns
                image = render.render(image, offset, _data['threshold'], markers, _data['maximumMarkers'])
                now_ns = time.perf_counter_ns()
                _stats.render_ns = now_ns - start_ns                             

                _stats.fps += 1

                if _data['detecting'] == True and markers < _data['maximumMarkers']:
                    start_ns = now_ns
                    tractor.move(_data['left'], _data['right'], _data['threshold'], offset)
                    now_ns = time.perf_counter_ns()
                    _stats.move_ns = now_ns - start_ns                             

                start_ns = now_ns
                with _lock:
                    _outputFrame = image
                _imageReady.set()
                now_ns = time.perf_counter_ns()
                _stats.lock_ns = (now_ns - start_ns)
            else:
                print("error reading frame from camera")
    finally:
        print("something wrong")
        vcap.release()


def generate():
    global _outputFrame, _lock, _running, _stats, _imageReady

    encode_param = [int(cv2.IMWRITE_JPEG_QUALITY), 50]

    while _running:

        _imageReady.wait()
        _imageReady.clear()

        with _lock:
            if _outputFrame is None:
                continue

            start_ns = time.perf_counter_ns()    
            (bSucess, encodedImage) = cv2.imencode(".jpg", _outputFrame, encode_param )
            _outputFrame = None

        if bSucess:
            jpegBytes = bytearray(encodedImage)
            _stats.jpg_bytes = len(jpegBytes)
            yield(b'--Ba4oTvQMY8ew04N8dcnM\r\n' b'Content-Type: image/jpeg\r\n\r\n' + jpegBytes + b'\r\n')
            now_ns = time.perf_counter_ns()
            _stats.jpg_ns = now_ns - start_ns
        else:
            _stats.jpg_ns = -1


def end():
    global _running
    _running = False
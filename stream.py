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
    camera_fps = 0

_running = True
_outputFrame = None
_lock = threading.Lock()
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

def read():
    global _outputFrame, _lock, _data, _running, _stats

    try:
        _running = True
        vcap = cv2.VideoCapture(0)
        #vcap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))
        #vcap.set(cv2.CAP_PROP_FRAME_WIDTH, 320)
        #vcap.set(cv2.CAP_PROP_FRAME_HEIGHT, 240)
        vcap.set(cv2.CAP_PROP_FPS,15)

        while _running:
            _stats.camera_fps = vcap.get(cv2.CAP_PROP_FPS)
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
                image                  = render.render(image, offset, _data['threshold'], markers, _data['maximumMarkers'])
                now_ns = time.perf_counter_ns()
                _stats.render_ns = now_ns - start_ns                             

                if _data['detecting'] == True and markers < _data['maximumMarkers']:
                    start_ns = now_ns
                    tractor.move(_data['left'], _data['right'], _data['threshold'], offset)
                    now_ns = time.perf_counter_ns()
                    _stats.move_ns = now_ns - start_ns                             

                start_ns = now_ns
                with _lock:
                    _outputFrame = image
                    now_ns = time.perf_counter_ns()
                    _stats.lock_ns += (now_ns - start_ns)
                #print(f"detect: {(time.time() - start) * 1000:0.3f} ms")
            else:
                print("error reading frame from camera")
    finally:
        print("something wrong")
        vcap.release()

def ms_from_ns(ns):
    return ns / 1000000

def generate():
    global _outputFrame, _lock, _running, _stats

    _stats.fps = 0
    start = time.time()

    while _running:
        with _lock:
            if _outputFrame is None:
                continue

            (bSucess, encodedImage) = cv2.imencode(".jpg", _outputFrame)
            _outputFrame = None

        if bSucess:
            yield(b'--Ba4oTvQMY8ew04N8dcnM\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(encodedImage) + b'\r\n')

        _stats.fps += 1
        #print(f"imencode(jpg): {(time.time() - start) * 1000:0.3f} ms")

        durationSec = time.time() - start
        if durationSec > 1.0:
            #class Stats:
            #    read_ns = 0
            #    detect_ns = 0
            #    render_ns = 0
            #    move_ns = 0
            #    lock_ns = 0
            #    fps = 0
            print(f"read detect render move lock jpg-fps   {ms_from_ns(_stats.read_ns):10.3f} {ms_from_ns(_stats.detect_ns):10.3f} {ms_from_ns(_stats.render_ns):10.3f} {ms_from_ns(_stats.move_ns):10.3f} {ms_from_ns(_stats.lock_ns):10.3f} {_stats.fps:2}({_stats.camera_fps})")
            start = time.time()
            _stats.fps = 0
                

def end():
    global _running
    _running = False
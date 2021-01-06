#!/usr/bin/python3
import cv2, threading, time, stream

_isStreaming = False
_thread = None

def videoOnOff(data):
    global _isStreaming
    if _isStreaming == False:        
        capture(data)
    else:
        release()

def capture(data):
    global _isStreaming, _thread
    _isStreaming = True
    _thread = threading.Thread(target=stream.detect, daemon=True)
    _thread.start() # start a thread which reads from camera (url) and applys detection
    print(f"started thread from url: {data['url']}")

def release():
    global _isStreaming, _thread
    _isStreaming = False
    stream.destroy()
    try:
        _thread.join()
    finally:
        print(f"Stopped read from")
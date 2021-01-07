#!/usr/bin/python3
import cv2, threading, time, stream

_isStreaming = False
_thread = None

def videoOnOff():
    global _isStreaming, _thread
    if _isStreaming == False:        
        _thread = threading.Thread(target=stream.read, daemon=True)
        _thread.start() # start a thread which reads from camera (url) and applys detection
        _isStreaming = True
        print(f"Started streaming thread")
    else:
        try:
            stream.end()
            _thread.join()
        finally:
            print(f"Stopped read from")
        _isStreaming = False

def applyChanges(data):
    stream.apply(data)

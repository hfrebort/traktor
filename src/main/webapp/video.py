#!/usr/bin/python3
import cv2, threading, time, stream

isStreaming = False
thread = None

def videoOnOff(data):
    global isStreaming
    if isStreaming == False:        
        capture(data)
    else:
        release()

def capture(data):
    global isStreaming, thread
    isStreaming = True
    thread = threading.Thread(target=stream.detect, daemon=True)
    thread.start() # start a thread that will perform motion detection
    print(f"started thread from url: {data['url']}")

def release():
    global isStreaming, thread
    isStreaming = False
    stream.destroy()
    try:
        thread.join()
    finally:
        print(f"Stopped read from")
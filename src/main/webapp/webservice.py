#!/usr/bin/python3
import sys, json, threading, web
import snapshot, tractor, video, stream

urls = (
    '/(.*)', 'ImageHandler'
)
app = web.application(urls, globals())

def init_header():
    web.header('Access-Control-Allow-Origin', '*')
    web.header('Access-Control-Allow-Headers', '*')
    web.header('Access-Control-Allow-Credentials', 'true')

class ImageHandler:

    def GET(self, path):
        if path == 'stop':
            tractor.stop_all()
        elif path == 'video':
            web.header('Content-Type', 'multipart/x-mixed-replace;boundary=Ba4oTvQMY8ew04N8dcnM')
            return stream.generate()

    def POST(self, path):
        postInput = json.loads(web.data())
        if path == 'prepareImage':
            snapshot.create_image(postInput)
        elif path == 'videoOnOff':
            video.videoOnOff(postInput)
        elif path == 'perform':
            tractor.perform(postInput)
        else:
            print ('Nothing')
        init_header()
        return {'success': 'true'}

    def OPTIONS(self, path):
        init_header()
        return {'success': 'true'}

if __name__ == "__main__":
    # start a thread that will perform motion detection
    #threading.Thread(target=stream.detect, daemon=True).start()
    app.run()
	
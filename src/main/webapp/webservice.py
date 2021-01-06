#!/usr/bin/python3
import sys, json, threading, web
import snapshot, video, stream #tractor

urls = (
    '/(.*)', 'RequestHandler'
)
app = web.application(urls, globals())

def init_header():
    web.header('Access-Control-Allow-Origin', '*')
    web.header('Access-Control-Allow-Headers', '*')
    web.header('Access-Control-Allow-Credentials', 'true')

class RequestHandler:

    def GET(self, path):
        if path == 'stop':
            print ('tractor.stop_all()') #tractor.stop_all()
        elif path == 'video':
            web.header('Content-Type', 'multipart/x-mixed-replace;boundary=Ba4oTvQMY8ew04N8dcnM')
            return stream.generate()
        else:
            raise web.seeother('/static/index.html')

    def POST(self, path):
        inputData = json.loads(web.data())
        if path == 'prepareImage':
            snapshot.create_image(inputData)
        elif path == 'videoOnOff':
            video.videoOnOff(inputData)
        elif path == 'perform':
            print ('tractor.perform(inputData)') #tractor.perform(inputData)
        else:
            print ('Nothing')
        init_header()
        return {'success': 'true'}

if __name__ == "__main__":
    app.run()
	
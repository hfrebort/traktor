#!/usr/bin/python3
import sys, json, threading, web
import video, stream, tractor

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
        if path == '':
            raise web.seeother('/static/index.html')
        elif path == 'stop':
            print ('tractor.stop_all()') 
            tractor.stop_all()
        elif path == 'video':
            web.header('Content-Type', 'multipart/x-mixed-replace;boundary=Ba4oTvQMY8ew04N8dcnM')
            return stream.generate()

    def POST(self, path):
        inputData = json.loads(web.data())
        if path == 'applyChanges':
            print (f'tractor.perform({inputData})') 
            video.applyChanges(inputData)
        elif path == 'perform':
            print (f'tractor.perform({inputData})') 
            tractor.perform(inputData)
        else:
            print ('Nothing')
        init_header()
        return {'success': 'true'}

if __name__ == "__main__":
    video.videoOnOff()
    print ('started application')
    app.run()
	
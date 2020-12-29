#!/usr/bin/python3
import sys, web, json, snapshot, tractor, video

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

    def POST(self, path):
        postInput = json.loads(web.data())
        if path == 'prepareImage':
            snapshot.create_image(postInput)
        elif path == 'videoOnOff':
            video.video_on_off(postInput)
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
    app.run()
	
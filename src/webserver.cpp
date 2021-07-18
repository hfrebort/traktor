#include <sys/stat.h>

#include "webserver.h"

void loop_sendJPEG(std::function<void(std::vector<unsigned char>&)> sendJPEGbytes);

void TraktorHandler::onRequest(const Pistache::Http::Request& req, Pistache::Http::ResponseWriter response)
{
            if ( req.resource() == "/video" ) {
            response
                .headers()
                .add<Pistache::Http::Header::Server>("pistache/0.1")
                .add<Pistache::Http::Header::ContentType>("multipart/x-mixed-replace;boundary=Ba4oTvQMY8ew04N8dcnM");

            static const std::string boundary("--Ba4oTvQMY8ew04N8dcnM\r\nContent-Type: image/jpeg\r\n\r\n");
            static const std::string CRLF("\r\n");

            Pistache::Http::ResponseStream stream = response.stream(Pistache::Http::Code::Ok);

            loop_sendJPEG([&stream](std::vector<unsigned char>& jpegBytes) {
                    // yield(b'--Ba4oTvQMY8ew04N8dcnM\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(encodedImage) + b'\r\n')
                    stream.write( boundary.data(), boundary.length() );
                    jpegBytes.insert(jpegBytes.end(), CRLF.begin(), CRLF.end());
                    stream.write( (char*)jpegBytes.data(), jpegBytes.size() );
                    stream.flush();
            });
    }
    else if ( req.resource() == "/data" ) {
    }
    else if ( req.resource() == "/applyChanges" ) {
        response.send(Pistache::Http::Code::Ok);
    }
    else {
        if (req.method() == Pistache::Http::Method::Get) {
            std::string toServe("static");

            if ( req.resource() == "/" ) {
                toServe.append("/index.html");
            }
            else {
                toServe.append(req.resource());
            }

            struct stat buffer;
            if ( stat(toServe.c_str(), &buffer) == 0) {
                Pistache::Http::serveFile(response, toServe);
            }
            else {
                response.send(Pistache::Http::Code::Not_Found);
            }
        }
    }
}

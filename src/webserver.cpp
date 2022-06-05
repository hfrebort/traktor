//#include <stdio.h>
//#include <vector>
//#include <functional>
//
//#include "httplib.h"

#include "shared.h"
#include "util.h"

void thread_send_jpeg(Shared* shared, std::function<bool(std::vector<uchar>&)> sendJPEGbytes);

int thread_webserver(int port, Shared* shared)
{
    using namespace httplib;
    Server svr;

    shared->webSvr = &svr;

    const char* webroot = "./static";

    if ( !svr.set_mount_point("/", webroot)) {
        printf("E: web root dir does not exist\n");
        return 1;
    }

    svr.Get("/video", [&](const Request &req, Response &res) {

        static const std::string boundary("--Ba4oTvQMY8ew04N8dcnM\r\nContent-Type: image/jpeg\r\n\r\n");
        static const std::string CRLF("\r\n");

        Stats &stats = shared->stats;

        res.set_content_provider(
            "multipart/x-mixed-replace;boundary=Ba4oTvQMY8ew04N8dcnM", // Content type
            [&](size_t offset, DataSink &sink) {
                //
                // begin of JPEG streaming
                //
                thread_send_jpeg(
                    shared,
                    [&sink,&stats](std::vector<unsigned char>& jpegBytes) {
                        // yield(b'--Ba4oTvQMY8ew04N8dcnM\r\n' b'Content-Type: image/jpeg\r\n\r\n' + bytearray(encodedImage) + b'\r\n')
                        jpegBytes.insert(jpegBytes.end(), CRLF.begin(), CRLF.end());
                        if ( !sink.write( boundary.data(), boundary.length() ) )
                        {
                            return false;
                        }
                        else if ( !sink.write( (char*)jpegBytes.data(), jpegBytes.size() ) )
                        {
                            return false;
                        }

                        stats.jpeg_bytes_sent += boundary.length() + jpegBytes.size();

                        return true;
                    });

                return true; // return 'false' if you want to cancel the process.
            },
            [](bool success) {}
        );
    });
    /*
        /applyChanges
    {
     "duration":1
    ,"left":12
    ,"right":13
    ,"direction":"center"
    ,"url":"http://10.3.141.165:8888/video"
    ,"colorFilter":true
    ,"detecting":true
    ,"colorFrom":"36,80,25"
    ,"colorTo":"80,255,255"
    ,"erode":10
    ,"dilate":10
    ,"contourMode":"POLY"
    ,"threshold":5
    ,"maximumMarkers":10
    ,"rowCount": 1
    ,"rowSpacePx": 160
    ,"rowPerspectivePx": 0
    }
    */
    svr.Post("/applyChanges", [&](const Request &req, Response &res)
    {
        try
        {
            DetectSettings& settings = shared->detectSettings;
            settings.set_fromJson(req.body);

            nlohmann::json data = nlohmann::json::parse(req.body);
            
            std::string detecting = data["detecting"];
            if ( detecting.compare("start") == 0) 
            {
                settings.detecting.store ( true );
                printf("I: detecting is now: ON\n");
            }
            else if ( detecting.compare("stop") == 0) 
            {
                settings.detecting.store ( false );
                printf("I: detecting is now: OFF\n");
            }
            
            data.erase("detecting");

            trk::write_to_file("./detect/lastSettings.json", data.dump());

        }
        catch(const std::exception& e)
        {
            fprintf(stderr, "/applyChanges: %s\n", e.what());
        }
    });
    //
    // ------------------------------------------------------------------------
    //
    /*
    {
        "colorFrom": [ 36,  15,  33 ],
        "colorTo":   [ 80, 201, 180 ],
        "dilate": 0,
        "erode": 0,
        "maxRows": 0,
        "minimalContourArea": 130,
        "rowPerspectivePx": 300,
        "rowSpacingPx": 160,
        "rowThresholdPx": 5
    }
    */
    svr.Get("/current", [&](const Request &req, Response &res) {

        DetectSettings& settings = shared->detectSettings;

        nlohmann::json data;
        {
            const auto from = settings.getImageSettings().colorFrom;
            const auto to   = settings.getImageSettings().colorTo;
            data["colorFrom"] = { (int)from[0], (int)from[1], (int)from[2] };
            data["colorTo"]   = { (int)to  [0], (int)to  [1], (int)to  [2] };
        }
        data["erode"]               = settings.getImageSettings().erode_iterations;
        data["dilate"]              = settings.getImageSettings().dilate_iterations;
        data["minimalContourArea"]  = settings.getImageSettings().minimalContourArea;

        data["maxRows"]             = settings.getReflineSettings().rowMax;
        data["rowThresholdPx"]      = settings.getReflineSettings().rowThresholdPx;
        data["rowSpacingPx"]        = settings.getReflineSettings().rowSpacingPx;
        data["rowPerspectivePx"]    = settings.getReflineSettings().rowPerspectivePx;
        
        res.set_content(data.dump(), "application/json");
        res.status = 200;
    });
    //
    // ------------------------------------------------------------------------
    //
    svr.Get("/list",       [&](const Request &req, Response &res) {
        errno = 0;
        DIR* dp = opendir("./detect");
        if (dp != NULL) {
            auto arr = nlohmann::json::array();
            
            dirent* dir;
            while((dir = readdir(dp)) != NULL)  
            {
                if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) {
                    continue;
                }
                arr.push_back(dir->d_name);
            }
            closedir(dp);

            nlohmann::json list;
            list["entries"] = arr;
            res.set_content(list.dump(), "application/json");
            res.status = 200;
        }

        if ( errno != 0 ) {
            res.status = 400;
            res.set_content(strerror(errno), "text/plain");
        }

    });
    //
    // ------------------------------------------------------------------------
    //
    svr.Get("/load/(.+)",  [&](const Request &req, Response &res) {
        std::string filename_to_load("./detect/");
        filename_to_load.append(req.matches[1].str());

        std::string content;
        if ( !trk::load_file_to_string(filename_to_load, &content) ) {
            res.status = 400;
            res.set_content(strerror(errno), "text/plain");
        }
        else {
            res.status = 200;
            res.set_content(content, "application/json");
        }
    });
    //
    // ------------------------------------------------------------------------
    //
    svr.Post("/save/(.+)", [&](const Request &req, Response &res) {
        std::string filename_to_save("./detect/");
        filename_to_save.append(req.matches[1].str());

        if ( ! trk::write_to_file(filename_to_save, req.body) ) {
            res.status = 400;
            res.set_content(strerror(errno), "text/plain");
        }
        else {
            res.status = 200;
        }
    });

    const char* host = "0.0.0.0";
    printf("I: webserver start listening on %s:%d\n", host, port);
    //svr.listen("0.0.0.0", port);
    svr.listen(host, port);

    return 0;
}


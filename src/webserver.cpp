#include <cstdio>
#include <vector>
#include <functional>
#include <dirent.h>

#include "httplib.h"
#include "json.hpp"

#include "shared.h"
#include "util.h"
#include "pipeline/ImagePipeline.hpp"
#include "encode.h"

using namespace httplib;

void thread_send_jpeg(Shared* shared, std::function<bool(std::vector<uchar>&)> sendJPEGbytes);

WORKER_RC encode_main(Workitem* work, EncodeContext* ctx);

/*
 * 2023-05-21 Spindler
 *  in all the URL callbacks sample: "svr->Get("/video", [=](const Request &req, Response &res)"
 *  the "[=]" is important! Giving [&] crashes the program.
 *  Don't know right now why.
 */

void URL_video(httplib::Server* svr, Shared* shared, ImagePipeline* pipeline, EncodeCounter* encode_stats)
{
    svr->Get("/video", [=](const Request &req, Response &res) {

        static const std::string boundary("--Ba4oTvQMY8ew04N8dcnM\r\nContent-Type: image/jpeg\r\n\r\n");
        static const std::string CRLF("\r\n");
        static std::atomic<int32_t> thread_count_video{0};

        res.set_content_provider(
            "multipart/x-mixed-replace;boundary=Ba4oTvQMY8ew04N8dcnM", // Content type
            [&](size_t offset, DataSink &sink) {

                int thread_count_before_add = thread_count_video.fetch_add(1);
                if ( thread_count_before_add > 0)
                {
                    thread_count_video.fetch_add(-1);
                    printf("W: webserver: there can be only ONE encoding/sending thread. exiting. count was: %d\n", thread_count_before_add);
                    return false;
                }
                else
                {
                    printf("I: webserver: start encoding/sending thread. count was: %d\n", thread_count_before_add);
                }

                //
                // begin of JPEG streaming
                //
                EncodeContext ctx(encode_stats, shared,
                    [&sink](std::vector<unsigned char>& jpegBytes, uint64_t* bytes_sent) {
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

                        *bytes_sent = boundary.length() + jpegBytes.size();

                        return true;
                    });

                pipeline->run_encode_3( encode_main, &ctx );
                thread_count_before_add = thread_count_video.fetch_add(-1);
                printf("I: webserver: URL /video... leaving. thread_count was %d before decrementing by 1\n", thread_count_before_add);

                return false; // return 'false' if you want to cancel the process.
            },
            [](bool success) {}
        );
    });

}

void URL_applyChanges(httplib::Server* svr, DetectSettings* detect_settings)
{
    svr->Post("/applyChanges", [=](const Request &req, Response &res)
    {
        try
        {
            detect_settings->set_fromJson(req.body);

            nlohmann::json data = nlohmann::json::parse(req.body);

            const std::string jsonData = data.dump();
            trk::write_to_file("./detect/lastSettings.json", jsonData);
            res.status = 200;
            printf("I: applyChanges: %s\n", jsonData.c_str());
        }
        catch(const std::exception& e)
        {
            fprintf(stderr, "/applyChanges: %s\n", e.what());
            res.status = 500;
            res.set_content(e.what(), "text/plain");
        }
    });
}

void URL_current(httplib::Server* svr, DetectSettings* settings)
{
    svr->Get("/current", [=](const Request &req, Response &res) {

        nlohmann::json data;
        {
            const auto from = settings->getImageSettings().colorFrom;
            const auto to   = settings->getImageSettings().colorTo;
            data["colorFrom"] = { (int)from[0], (int)from[1], (int)from[2] };
            data["colorTo"]   = { (int)to  [0], (int)to  [1], (int)to  [2] };
        }
        data["erode"]               = settings->getImageSettings().erode_iterations;
        data["dilate"]              = settings->getImageSettings().dilate_iterations;
        data["minimalContourArea"]  = settings->getImageSettings().minimalContourArea;

        data["maxRows"]             = settings->getReflineSettings().rowMax;
        data["rowThresholdPx"]      = settings->getReflineSettings().rowThresholdPx;
        data["rowSpacingPx"]        = settings->getReflineSettings().rowSpacingPx;
        data["rowPerspectivePx"]    = settings->getReflineSettings().rowPerspectivePx;
        data["rowRangePx"]          = settings->getReflineSettings().rowRangePx;

        
        res.set_content(data.dump(), "application/json");
        res.status = 200;
    });
}

void URL_stats(httplib::Server* svr, const Stats* diff)
{
    svr->Get("/stats", [=](const Request &req, Response &res) {

        using namespace std::chrono;

        nlohmann::json data;
        data["camera"]["fps"] = diff->camera.frames / Stats::pause.count();

        const auto overall_ms = duration_cast<milliseconds>(diff->detect.overall).count();
        data["detect"]["time_milliseconds"]["0_overall"]      = overall_ms;
        data["detect"]["time_milliseconds"]["1_cvtColor"]     = duration_cast<milliseconds>(diff->detect.cvtColor).count();
        data["detect"]["time_milliseconds"]["2_GaussianBlur"] = duration_cast<milliseconds>(diff->detect.GaussianBlur).count();
        data["detect"]["time_milliseconds"]["3_inRange"]      = duration_cast<milliseconds>(diff->detect.inRange).count();
        data["detect"]["time_milliseconds"]["4_erode"]        = duration_cast<milliseconds>(diff->detect.erode).count();
        data["detect"]["time_milliseconds"]["5_dilate"]       = duration_cast<milliseconds>(diff->detect.dilate).count();
        data["detect"]["time_milliseconds"]["6_findContours"] = duration_cast<milliseconds>(diff->detect.findContours).count();

        const auto fps = diff->detect.frames / Stats::pause.count();
        data["detect"]["image"]["fps"]                        = fps;
        data["detect"]["image"]["MB/s"]                       = diff->detect.frame_bytes / 1024 / 1024 / Stats::pause.count();
        if ( fps != 0 )
        {
            const auto available_ms = 1000 / fps;
            data["detect"]["image"]["available_ms_one_frame"]   = available_ms;
            data["detect"]["image"]["time_used_%"]              = (int)( (float)overall_ms / (float)available_ms * 100.0 );
        }

        data["encode"]["kB/s"]      = diff->encode.bytes_sent  / 1024 / Stats::pause.count();
        data["encode"]["images/s"]  = diff->encode.images_sent / Stats::pause.count(); 
        data["encode"]["draw"]      = duration_cast<milliseconds>( nanoseconds(diff->encode.draw) ).count();
        data["encode"]["overall"]   = duration_cast<milliseconds>( nanoseconds(diff->encode.overall) ).count();


        res.set_content(data.dump(), "application/json");
        res.status = 200;
    });
}
int thread_webserver(int port, Shared* shared, ImagePipeline* pipeline, EncodeCounter* encoder_stats, Stats* stats_diff)
{
    Server svr;

    shared->webSvr = &svr;

    const char* webroot = "./static";

    if ( !svr.set_mount_point("/", webroot)) {
        printf("E: web root dir does not exist\n");
        return 1;
    }

    URL_video(&svr, shared, pipeline, encoder_stats);
    URL_applyChanges(&svr, &shared->detectSettings );
    URL_current(&svr, &shared->detectSettings);
    URL_stats(&svr, stats_diff);
    
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
    //
    // ------------------------------------------------------------------------
    //
    svr.Get("/debug/lift",  [&](const Request &req, Response &res) {
        shared->harrowLifted.store(true);
        puts("/debug/lift - shared->harrowLifted.store(true)");
        res.status = 200;
    });
    //
    // ------------------------------------------------------------------------
    //
    svr.Get("/debug/unlift",  [&](const Request &req, Response &res) {
        shared->harrowLifted.store(false);
        puts("/debug/lift - shared->harrowLifted.store(false)");
        res.status = 200;
    });
    const char* host = "0.0.0.0";
    printf("I: webserver start listening on %s:%d\n", host, port);
    //svr.listen("0.0.0.0", port);
    svr.listen(host, port);

    return 0;
}


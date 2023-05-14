//#include <signal.h>
//#include <functional>
//#include <memory>
//#include <thread>
//
//#include "httplib.h"

#include "shared.h"
#include "harrow.h"
#include "pipeline/ImagePipeline.hpp"

int wait_for_signal(void)
{
    sigset_t signals;
    if (sigemptyset(&signals) != 0
        || sigaddset(&signals, SIGTERM) != 0
        || sigaddset(&signals, SIGINT) != 0
        || sigaddset(&signals, SIGHUP) != 0
        || pthread_sigmask(SIG_BLOCK, &signals, nullptr) != 0)
    {
        perror("install signal handler failed");
        return 1;
    }

    int signal = 0;
    int status = sigwait(&signals, &signal);
    if (status == 0)
    {
        printf("received signal: %d\n",signal);
    }
    else
    {
        printf("sigwait returns: %d\n", status);
    }


    return 0;
}

void join_thread(std::thread* t, const char* name)
{
    if ( t != nullptr && t->joinable() )
    {
        t->join();
        printf("I: thread ended: %s\n", name);
    }
}

void shutdown_all_threads(Shared& shared, std::thread* camera, std::thread* stats, std::thread* web, std::thread* detect, std::thread* center)
{
    shared.shutdown_requested.store(true);

    if (shared.webSvr != nullptr)
    {
        shared.webSvr->stop();
    }

    join_thread(center, "center_harrow");
    join_thread(web,    "webserver");
    join_thread(camera, "camera");
    join_thread(detect, "detect");
    join_thread(stats,  "stats");
}

int  thread_webserver(int port, Shared* shared);
void thread_camera(const Options& options, Shared* shared);
void thread_stats(Shared* ,Stats*);
void thread_detect(Shared*, Stats*, Harrow* harrow, bool showDebugWindows);
void thread_center_harrow(Harrow* harrow, std::atomic<bool>* harrowLifted, const std::atomic<bool>* shutdown_requested);

int parser_commandline(int argc, char* argv[], Options* options)
{
    const cv::String keys =
        "{help h usage ? |     | print this message                        }"
        "{c camindex     |   0 | index USB camera                          }"
        "{width          | 640 | cv::CAP_PROP_FRAME_WIDTH                  }"
        "{height         | 480 | cv::CAP_PROP_FRAME_HEIGHT                 }"
        "{fps            |  25 | cv::CAP_PROP_FPS                          }"
        "{f file         |     | filename video                            }"
        "{slow           |   1 | slow down playback of videofile by factor }"
        "{p port         |9080 | port web server                           }"
        "{s showwindow   |     | show intermediate results of processing   }";

    cv::CommandLineParser cmd_parser(argc, argv, keys);
    cmd_parser.about("Traktor v0.3");

    if (!cmd_parser.check())
    {
        cmd_parser.printErrors();
        return 2;
    }

    if ( cmd_parser.has("help") )
    {
        cmd_parser.printMessage();
        return 1;
    }

    options->showDebugWindows               = cmd_parser.has             ("showwindow");
    options->cameraIndex                    = cmd_parser.get<int>        ("camindex");
    options->filename                       = cmd_parser.get<std::string>("file");
    options->httpPort                       = cmd_parser.get<int>        ("port");
    options->video_playback_slowdown_factor = cmd_parser.get<int>        ("slow");
    options->camera_width                   = cmd_parser.get<int>        ("width");
    options->camera_height                  = cmd_parser.get<int>        ("height");
    options->camera_fps                     = cmd_parser.get<int>        ("fps");

    return 0;
}

void ensure_directories(void)
{
    DIR* dir = opendir("./detect");
    if (dir) {
        /* Directory exists. */
        puts("I: directory ./detect exists");
        closedir(dir);
    } else if (ENOENT == errno) {
        /* Directory does not exist. */
        mkdir("./detect", 0775);
        puts("I: directory ./detect created");
    } else {
        fprintf(stderr, "E: could not create dir ./detect\n");
    }
}

void load_lastSettings(DetectSettings& detectSettings)
{
    std::string lastSettings;
    if ( !trk::load_file_to_string("./detect/lastSettings.json", &lastSettings) ) {
        fprintf(stderr, "E: could not load lastSettings: %s\n", strerror(errno));
    }
    else {
        detectSettings.set_fromJson(lastSettings);
        puts("I: ./detect/lastSettings.json loaded");
        puts(lastSettings.c_str());
    }
}

int main(int argc, char* argv[])
{
    int rc =0;

    Options options;
    if ( (rc=parser_commandline(argc, argv, &options)) != 0 )
    {
        return rc;
    }

    ensure_directories();

    printf("I: opencv version: %s\n", cv::getVersionString().c_str());

    if (options.filename.empty())
    {
        printf("I: using camera #%d\n", options.cameraIndex);
    }
    else
    {
        printf("I: using file: %s\n", options.filename.c_str() );
    }

    std::unique_ptr<Harrow> harrow;
    
    try {
        harrow = std::make_unique<Harrow>();
    }
    catch (std::exception& ex) {
        harrow.reset(nullptr);
        fprintf(stderr, "E: Hack Steuerung sagt: %s\n", ex.what());
    }

    Stats  stats;
    Shared shared;
    load_lastSettings(shared.detectSettings);

    std::thread camera(thread_camera, options, &shared);
    std::thread detect(thread_detect, &shared, &shared.stats, harrow.get(), options.showDebugWindows);
    //std::thread stats (thread_stats, &shared, &shared.stats);
    std::thread web   (thread_webserver, options.httpPort, &shared);
    std::thread center(thread_center_harrow, harrow.get(), &(shared.harrowLifted), &(shared.shutdown_requested));

    rc = wait_for_signal();
    shutdown_all_threads(shared, &camera, /*&stats*/ nullptr, &web, &detect, &center);

    return rc;
}

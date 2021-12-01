//#include <signal.h>
//#include <functional>
//#include <memory>
//#include <thread>
//
//#include "httplib.h"

#include "shared.h"
#include "harrow.h"

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

void shutdown_all_threads(Shared& shared, std::thread* camera, std::thread* stats, std::thread* web, std::thread* detect)
{
    shared.shutdown_requested.store(true);

    if (web != nullptr)
    {
        printf("I: stopping webserver... \n");
        shared.webSvr->stop();
        printf("I: stopping webserver done\n");
        web->join();
        printf("I: thread ended: webserver\n");
    }

    if (camera != nullptr && camera->joinable() )
    {
        camera->join();
        printf("I: thread ended: camera\n");
    }

    if (detect != nullptr)
    {
        detect->join();
        printf("I: thread ended: detect\n");
    }

    if (stats != nullptr)
    {
        stats->join();
        printf("I: thread ended: stats\n");
    }
}

int  thread_webserver(int port, Shared* shared);
void thread_camera(const Options& options, Shared* shared);
void thread_stats(Shared* ,Stats*);
void thread_detect(Shared*, Stats*, Harrow* harrow, bool showDebugWindows);

int parser_commandline(int argc, char* argv[], Options* options)
{
    const cv::String keys =
        "{help h usage ? |     | print this message                        }"
        "{c camindex     |   0 | index USB camera                          }"
        "{f file         |     | filename video                            }"
        "{slow           |   1 | slow down playback of videofile by factor }"
        "{p port         |9080 | port web server                           }"
        "{s showwindow   |     | show intermediate results of processing   }";

    cv::CommandLineParser cmd_parser(argc, argv, keys);
    cmd_parser.about("Traktor v0.2");

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

    return 0;
}

int main(int argc, char* argv[])
{
    int rc =0;

    Options options;
    if ( (rc=parser_commandline(argc, argv, &options)) != 0 )
    {
        return rc;
    }

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
        fprintf(stderr, "Hack Steuerung sagt: %s\n", ex.what());
    }

    Shared shared;

    std::thread camera(thread_camera, options, &shared);
    std::thread detect(thread_detect, &shared, &shared.stats, harrow.get(), options.showDebugWindows);
    std::thread stats (thread_stats, &shared, &shared.stats);
    std::thread web   (thread_webserver, options.httpPort, &shared);

    rc = wait_for_signal();
    shutdown_all_threads(shared, &camera, &stats, &web, &detect);

    return rc;
}

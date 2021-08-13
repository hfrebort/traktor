#include <signal.h>
#include <iostream>
#include <functional>
#include <memory>
#include <thread>

#include "httplib.h"

#include "shared.h"

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
        std::cout << "received signal " << signal << std::endl;
    }
    else
    {
        std::cerr << "sigwait returns " << status << std::endl;
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

    if (camera != nullptr)
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
void thread_camera(int cameraIdx, Shared* shared);
void thread_stats(Shared* ,Stats*);
void thread_detect(Shared*, Stats*);

int main(int argc, char* argv[])
{
    int rc =0;

    int cameraIdx = 0;
    if (argc == 2)
    {
        cameraIdx = std::stoi(argv[1]);
    }

    printf("I: using camera #%d\n", cameraIdx);

    Shared shared;

    std::thread camera(thread_camera, cameraIdx, &shared);
    std::thread detect(thread_detect, &shared, &shared.stats);
    std::thread stats (thread_stats, &shared, &shared.stats);
    std::thread web   (thread_webserver, 9080, &shared);

    rc = wait_for_signal();
    shutdown_all_threads(shared, &camera, &stats, &web, &detect);
    //shutdown_all_threads(shared, &camera, &stats, &web, nullptr);

    return rc;
}

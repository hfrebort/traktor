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

    printf("press CTRL-C to quit\n");
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

int  thread_webserver(int port, Shared* shared);
void thread_camera(int cameraIdx, Shared* shared);
void thread_stats(Stats* stats);

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
    std::thread stats (thread_stats, &shared.stats);
    std::thread web   (thread_webserver, 9080, &shared);

    rc = wait_for_signal();
    

    return rc;
}

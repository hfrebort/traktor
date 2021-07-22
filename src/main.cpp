#include <signal.h>
#include <iostream>
#include <functional>
#include <memory>
#include <thread>

#include "pistache/endpoint.h"

#include "webserver.h"
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
std::unique_ptr<Pistache::Http::Endpoint> startServer(int port, Shared* shared)
{
    Pistache::Address addr4(Pistache::Ipv4::any(),     Pistache::Port(port));

    auto opts = Pistache::Http::Endpoint::options()
        .threads(2);

    auto server4 = std::make_unique<Pistache::Http::Endpoint>(addr4);
    
    auto handler = Pistache::Http::make_handler<TraktorHandler>();
    handler->_shared = shared;

    server4->init(opts);
    server4->setHandler(handler);
    server4->serveThreaded();
    
    std::cout << "listening on IPv4: " << addr4 << std::endl;

    return server4;
}

void thread_camera(int cameraIdx, Stats* stats, cv::Mat frame_buf[3], std::atomic<int>* sharedFrameBufSlot);
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

    std::thread camera(thread_camera, cameraIdx, &shared.stats, shared.frame_buf, &shared.frame_buf_slot);
    std::thread stats (thread_stats, &shared.stats);
    auto server4 = startServer(9080, &shared);

    rc = wait_for_signal();
    
    printf("I: shutting down webserver... \n");
    server4->shutdown();
    printf("I: shutting down webserver done\n");

    return rc;
}

#include <signal.h>


#include <iostream>
#include <functional>

#include "pistache/endpoint.h"

#include "webserver.h"

int main()
{
    Pistache::Address addr4(Pistache::Ipv4::any(),     Pistache::Port(9080));
    Pistache::Address addr6(Pistache::Ipv6::any(true), Pistache::Port(9080));
    
    auto opts = Pistache::Http::Endpoint::options()
        .threads(2);

    auto handler = Pistache::Http::make_handler<TraktorHandler>();

    Pistache::Http::Endpoint server6(addr6);
    //server6.init(opts);
    //server6.setHandler(handler);
    //server6.serveThreaded();
    //std::cout << "listening on IPv6: " << addr6 << std::endl;

    Pistache::Http::Endpoint server4(addr4);
    server4.init(opts);
    server4.setHandler(handler);
    server4.serveThreaded();
    std::cout << "listening on IPv4: " << addr4 << std::endl;


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

    server4.shutdown();
    server6.shutdown();

}

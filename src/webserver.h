#include "pistache/endpoint.h"

class TraktorHandler : public Pistache::Http::Handler
{
public:
    HTTP_PROTOTYPE(TraktorHandler)

    void onRequest(const Pistache::Http::Request& req, Pistache::Http::ResponseWriter response) override;
};
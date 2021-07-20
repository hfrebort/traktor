#pragma once

#include "pistache/endpoint.h"

#include "shared.h"

class TraktorHandler : public Pistache::Http::Handler
{
public:
    HTTP_PROTOTYPE(TraktorHandler)

    void onRequest(const Pistache::Http::Request& req, Pistache::Http::ResponseWriter response) override;

    Shared* _shared;
};
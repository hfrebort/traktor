#include "shared.h"

#include "json.hpp"

void DetectSettings::set_fromJson(const std::string& jsonString)
{
    nlohmann::json data = nlohmann::json::parse(jsonString);
            
    this->set_color_from(        data["colorFrom"][0].get<int>(),
                                 data["colorFrom"][1].get<int>(), 
                                 data["colorFrom"][2].get<int>() );

    this->set_color_to(          data["colorTo"][0].get<int>(),
                                 data["colorTo"][1].get<int>(), 
                                 data["colorTo"][2].get<int>() );

    this->set_erode_dilate(      data["erode"].get<int>(),
                                 data["dilate"].get<int>() );

    this->set_maxRows           ( data["maxRows"]            .get<int>() );
    this->set_rowSpacingPx      ( data["rowSpacingPx"]       .get<int>() );
    this->set_rowPerspectivePx  ( data["rowPerspectivePx"]   .get<int>() );
    this->set_rowThresholdPx    ( data["rowThresholdPx"]     .get<int>() );
    this->set_rowRangePx        ( data["rowRangePx"]         .get<int>() );
    this->set_minimalContourArea( data["minimalContourArea"] .get<int>() );
}
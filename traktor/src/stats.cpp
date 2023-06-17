#include <chrono>

#include "stats.h"

const std::chrono::seconds Stats::pause = std::chrono::seconds(2);

void Stats::diff(const Stats &incremented, const Stats &last, Stats *diff)
{
    diff->camera.frames = incremented.camera.frames - last.camera.frames;

    diff->detect.frames = incremented.detect.frames - last.detect.frames;
    diff->detect.frame_bytes = incremented.detect.frame_bytes - last.detect.frame_bytes;

    if (diff->detect.frames > 0)
    {
        diff->detect.cvtColor = (incremented.detect.cvtColor - last.detect.cvtColor) / diff->detect.frames;
        diff->detect.GaussianBlur = (incremented.detect.GaussianBlur - last.detect.GaussianBlur) / diff->detect.frames;
        diff->detect.inRange = (incremented.detect.inRange - last.detect.inRange) / diff->detect.frames;
        diff->detect.erode = (incremented.detect.erode - last.detect.erode) / diff->detect.frames;
        diff->detect.dilate = (incremented.detect.dilate - last.detect.dilate) / diff->detect.frames;
        diff->detect.findContours = (incremented.detect.findContours - last.detect.findContours) / diff->detect.frames;
        diff->detect.overall = (incremented.detect.overall - last.detect.overall) / diff->detect.frames;
    }

    diff->encode.bytes_sent  = incremented.encode.bytes_sent - last.encode.bytes_sent;
    diff->encode.images_sent = incremented.encode.images_sent - last.encode.images_sent;
    //printf("inc images_sent: %lu, last images_sent: %lu\n", incremented.encode.images_sent.load(), last.encode.images_sent.load());

    if (diff->encode.images_sent > 0)
    {
        diff->encode.draw = (incremented.encode.draw.load() - last.encode.draw.load()) / diff->encode.images_sent;
        diff->encode.overall = (incremented.encode.overall.load() - last.encode.overall.load()) / diff->encode.images_sent;
        //printf("inc overall: %lu, last overall: %lu\n", incremented.encode.overall.load(), last.encode.overall.load());
    }
}

Stats &Stats::operator=(const Stats &rhs)
{
    this->camera.frames = rhs.camera.frames;

    this->detect.cvtColor = rhs.detect.cvtColor;
    this->detect.dilate = rhs.detect.dilate;
    this->detect.erode = rhs.detect.erode;
    this->detect.findContours = rhs.detect.findContours;
    this->detect.frame_bytes = rhs.detect.frame_bytes;
    this->detect.frames = rhs.detect.frames;
    this->detect.GaussianBlur = rhs.detect.GaussianBlur;
    this->detect.inRange = rhs.detect.inRange;
    this->detect.overall = rhs.detect.overall;

    this->encode.bytes_sent = rhs.encode.bytes_sent.load();
    this->encode.draw = rhs.encode.draw.load();
    this->encode.images_sent = rhs.encode.images_sent.load();
    this->encode.overall = rhs.encode.overall.load();

    return *this;
}

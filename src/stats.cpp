#include "stats.h"

const std::chrono::seconds Stats::pause = std::chrono::seconds(2);

void Stats::diff(const Stats& incremented, const Stats& last, Stats* diff)
{
    diff->camera.frames = incremented.camera.frames - last.camera.frames;

    diff->detect.frames      = incremented.detect.frames      - last.detect.frames;
    diff->detect.frame_bytes = incremented.detect.frame_bytes - last.detect.frame_bytes;

    if ( diff->detect.frames > 0 )
    {
        diff->detect.cvtColor     = (incremented.detect.cvtColor     - last.detect.cvtColor)     / diff->detect.frames;
        diff->detect.GaussianBlur = (incremented.detect.GaussianBlur - last.detect.GaussianBlur) / diff->detect.frames;
        diff->detect.inRange      = (incremented.detect.inRange      - last.detect.inRange)      / diff->detect.frames;
        diff->detect.erode        = (incremented.detect.erode        - last.detect.erode)        / diff->detect.frames;
        diff->detect.dilate       = (incremented.detect.dilate       - last.detect.dilate)       / diff->detect.frames;
        diff->detect.findContours = (incremented.detect.findContours - last.detect.findContours) / diff->detect.frames;
        diff->detect.overall      = (incremented.detect.overall      - last.detect.overall)      / diff->detect.frames;
    }

    diff->encode.bytes_sent  = incremented.encode.bytes_sent  - last.encode.bytes_sent;
    diff->encode.images_sent = incremented.encode.images_sent - last.encode.images_sent;
    if ( diff->encode.images_sent > 0 )
    {
        diff->encode.draw        = ( incremented.encode.draw        - last.encode.draw    ) / diff->encode.images_sent;
        diff->encode.overall     = ( incremented.encode.overall     - last.encode.overall ) / diff->encode.images_sent;
    }
}

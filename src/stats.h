#pragma once

struct CameraCounter
{
    uint32_t                   frames{0};
};

struct DetectCounter
{
    uint32_t                   frames{};
    uint64_t                   frame_bytes{};
    std::chrono::nanoseconds   overall{};
    std::chrono::nanoseconds   cvtColor{};
    std::chrono::nanoseconds   GaussianBlur{};
    std::chrono::nanoseconds   inRange{};
    std::chrono::nanoseconds   erode{};
    std::chrono::nanoseconds   dilate{};
    std::chrono::nanoseconds   findContours{};
};

struct EncodeCounter
{
    uint64_t   images_sent{0};
    uint64_t   bytes_sent{0};
};

struct Stats
{
    CameraCounter camera;
    DetectCounter detect;
    EncodeCounter encode;

    static const std::chrono::seconds pause;

    static void diff(const Stats& incremented, const Stats& last, Stats* current);

};
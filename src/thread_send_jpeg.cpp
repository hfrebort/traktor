#include <functional>
#include <atomic>
#include <opencv2/opencv.hpp>

#include "shared.h"

void thread_send_jpeg(Shared* shared, std::function<void(std::vector<uchar>&)> sendJPEGbytes)
{
    const std::vector<int> JPEGparams = {cv::IMWRITE_JPEG_QUALITY, 50};

    cv::Mat frame;
    std::vector<uchar> jpegImage;

    printf("thread send_jpeg running\n");

    for (;;)
    {
        int frameReadySlot;
        while ( (frameReadySlot=std::atomic_exchange( &(shared->frame_buf_slot), 0)) == 0 )
        {
            // wait for a frame ready in the frameBuffer
        }

        cv::Mat& frame = shared->frame_buf[frameReadySlot];
        if ( !cv::imencode(".jpg", frame, jpegImage, JPEGparams))
        {

            printf("E: imencode(jpg)\n");
        }
        else
        {
            printf("I: sending JPEG bytes: %lukB\n", jpegImage.size() / 1024);
            sendJPEGbytes(jpegImage);
            shared->stats.jpeg_sent++;
        }
    }
}
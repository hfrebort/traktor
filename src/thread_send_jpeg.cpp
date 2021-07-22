//#include <functional>
//#include <atomic>
//#include <opencv2/opencv.hpp>

#include "shared.h"

void thread_send_jpeg(Shared* shared, std::function<void(std::vector<uchar>&)> sendJPEGbytes)
{
    const std::vector<int> JPEGparams = {cv::IMWRITE_JPEG_QUALITY, 50};

    std::vector<uchar> jpegImage;

    printf("I: thread send_jpeg running\n");

    for (;;)
    {
        int frameReadySlot;
        while ( (frameReadySlot=std::atomic_exchange( &(shared->frame_buf_slot), 0)) == 0 )
        {
            // wait for a frame ready in the frameBuffer
        }

        int arrayIdx = frameReadySlot - 1;
        cv::Mat& frame = shared->frame_buf[arrayIdx];
        
        //printf("JPG: arrayIdx: %d, Mat: %dx%d\n", arrayIdx, frame.size().width, frame.size().height );

        if ( !cv::imencode(".jpg", frame, jpegImage, JPEGparams))
        {
            printf("E: imencode(jpg)\n");
        }
        else
        {
            /*
            printf("JPG: sending JPEG %d/%d bytes: %lukB\n"
                , frame.size().width
                , frame.size().height
                , jpegImage.size() / 1024);
                */
            sendJPEGbytes(jpegImage);
            shared->stats.jpeg_sent++;
        }
    }
}
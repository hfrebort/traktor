//#include <functional>
//#include <atomic>
//#include <opencv2/opencv.hpp>

#include "shared.h"

void thread_send_jpeg(Shared* shared, std::function<bool(std::vector<uchar>&)> sendJPEGbytes)
{
    const std::vector<int> JPEGparams = {cv::IMWRITE_JPEG_QUALITY, 50};

    std::vector<uchar> jpegImage;

    printf("I: thread send_jpeg running\n");

    short idx = -1;

    for (;;)
    {
        //
        // wait to be signaled by detection thread for a new frame
        //
        {
            std::unique_lock<std::mutex> ul(shared->analyzed_frame_ready_mutex);
            shared->analyzed_frame_ready.wait(ul, [&idx,&shared]() { return idx != shared->analyzed_frame_ready_idx.load(); });
        }
        //
        // load new idx of buffer
        //
        idx = shared->analyzed_frame_ready_idx.load();
        
        bool encoded;
        {
            //
            // lock the specific buffer for the duration of the encoding
            //
            std::lock_guard<std::mutex> lk(shared->analyzed_frame_buf_mutex[idx]);
            encoded = cv::imencode(".jpg", shared->analyzed_frame_buf[idx], jpegImage, JPEGparams);
        }
        if ( !encoded )
        {
            printf("E: error encoding frame to JPEG\n");
        }
        else
        {
            if ( !sendJPEGbytes(jpegImage) )
            {
                printf("I: send of JPG failed. connection closed. quitting sending thread.\n");
                break;
            }
            shared->stats.jpeg_sent++;
        }

        if ( shared->shutdown_requested.load() )
        {
            printf("I: thread send_jpeg \n");
            break;
        }
    }
}
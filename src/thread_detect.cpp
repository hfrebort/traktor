#include "shared.h"
#include "stats.h"

void run_detection(const cv::Mat& cameraFrame, cv::Mat& outputFrame)
{
    outputFrame = cameraFrame.clone();
}

void thread_detect(Shared* shared, Stats* stats)
{
    printf("I: thread detect running\n");

    short idx = 0;

    for (;;)
    {
        //
        // get new frame from camera buffer
        //
        int frameReadyIdx;
        while ( (frameReadyIdx=std::atomic_exchange( &(shared->frame_buf_slot), -1)) == -1 )
        {
            // wait for a frame ready in the frameBuffer
            std::unique_lock<std::mutex> lk(shared->camera_frame_ready_mutex);
            shared->camera_frame_ready.wait(lk);
        }
        cv::Mat& cameraFrame = shared->frame_buf[frameReadyIdx];
        //
        // lock an output buffer
        //
        {
            std::lock_guard<std::mutex> lk(shared->analyzed_frame_buf_mutex[idx]);
            run_detection(cameraFrame,     shared->analyzed_frame_buf[idx]);
        }
        //
        // set index for other thread
        //
        shared->analyzed_frame_ready_idx.store(idx);
        //
        // notify other thread about ready buffer
        // 2021-08-08 Spindler (Moz'ens Geburtstag)
        //   Still don't know if a lock is required to do .notify_xxx()
        //
        {
            std::unique_lock<std::mutex> ul(shared->analyzed_frame_ready_mutex);
            shared->analyzed_frame_ready.notify_one();
        }
        
        idx = 1 - idx;

        if (shared->shutdown_requested.load())
        {
            printf("I: thread detect quitting...\n");
            break;
        }
    }
}
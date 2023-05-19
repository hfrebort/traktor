#include "ImagePipeline.hpp"

//-----------------------------------------------------------------------------
void ImagePipeline::camera_1(std::function<void(Workitem*,CameraContext*)> process, CameraContext* context) {
//-----------------------------------------------------------------------------

    puts("starting camera loop");

    for ( ;; ) {

        // --- read ---

        const int32_t idx_cam_process = get_free();
        if ( idx_cam_process == NO_FREE_SLOT_FOUND) {
            puts( "camera: get_free() NO_FREE_SLOT_FOUND. exiting." );
            break;
        }

        // --- process ---
        process( &(_workitems[idx_cam_process]) , context );
        
        // --- write ---

        if ( ! write(_CameraToDetect, idx_cam_process) )
        {
            break;
        }

    } 

}

//-----------------------------------------------------------------------------
void ImagePipeline::detect_2(std::function<void(Workitem*,DetectContext*)> process, DetectContext* context) {
//-----------------------------------------------------------------------------
    
    puts("starting detect loop");

    for (;;) {

        int32_t idx;
        if ( ! read(_CameraToDetect, &idx ) )
        {
            break;
        }
        else
        {
            process( &(_workitems[idx]) , context );
            if ( !write(_DetectToEncode, idx) )
            {
                break;
            }
        }
    } 
}

//-----------------------------------------------------------------------------
void ImagePipeline::encode_3(std::function<WORKER_RC(Workitem*,EncodeContext*)> process, EncodeContext* context) {
//-----------------------------------------------------------------------------

    puts("starting encode loop");

    for (;;)
    {
        int32_t idx;
        if ( ! read(_DetectToEncode, &idx ) )
        {
            break;
        }
        else
        {
            const WORKER_RC rc = process( &(_workitems[idx]) , context );
            set_free(idx);

            if ( rc == WORKER_RC::LIKE_TO_EXIT )
            {
                break;
            }
        }
    }

}
//-----------------------------------------------------------------------------
bool ImagePipeline::write(Postbox& postbox, int32_t write_idx) {
//-----------------------------------------------------------------------------

    const int32_t before = postbox.exchange(write_idx);

    if ( before == ACTION_FREE  ) 
    { 
        ; // reader thread was FREE, new work for him/her/it
    } 
    else if ( before == ACTION_SLEEP ) 
    { 
        postbox.signal(); 
    } 
    else if ( before == ACTION_EXIT  ) 
    {
        puts( "PIPE-WRITE: ACTION_EXIT" );
        return false;
    } 
    else if ( before  < ACTION_MIN || before > ACTION_MAX ) 
    { 
        puts( "PIPE-WRITE: <min >max" );
        return false;
    } 
    else 
    {    
        // reader thread missed work-idx
        // mark this work-idx as FREE
        set_free(before);
    }

    return true;
}
//-----------------------------------------------------------------------------
bool ImagePipeline::read(Postbox& postbox, int32_t* new_idx) {
//-----------------------------------------------------------------------------

    int32_t idx;

    for (;;)
    {
        idx = postbox.exchange(ACTION_FREE);

        if ( idx == ACTION_FREE ) {
            // since our last read there is still OUR FREE in the postbox.
            // go to SLEEP.
            if ( postbox.compare_exchange(idx, ACTION_SLEEP) ) { 
                // FREE --> SLEEP successfull. take a nap...
                postbox.wait_for_signal();         
                continue;   // goto begin of loop and get a new "idx"
            }
            else {
                // idx == new work IDX
                // there was work posted for us during the attempt going to SLEEP
                break;
            }
        }
        else
        {
            // there was work for us in the postbox. != ACTION_FREE
            break;
        }
    }

    if ( idx == ACTION_SLEEP )                     
    { 
        puts( "PIPE-READ: idx == ACTION_SLEEP ... should not be possible" );
    } 
    else if ( idx == ACTION_EXIT  )                     
    { 
        puts( "PIPE-READ: ACTION_EXIT" );
    } 
    else if ( idx < ACTION_MIN || idx > ACTION_MAX    ) 
    { 
        printf( "PIPE-READ: action out of range: %d\n", idx );
    } 
    else 
    {
        *new_idx = idx;
        return true;
    }

    return false;

}
//-----------------------------------------------------------------------------
int8_t ImagePipeline::get_free() {
//-----------------------------------------------------------------------------

    for (int8_t i=0; i < 5; ++i) {
        bool expected = true;
        if ( _Free[i].compare_exchange_strong(
                            expected
                            , false
                            , std::memory_order_acq_rel) ) {
            return i;
        }
    }

    return NO_FREE_SLOT_FOUND;

}
//-----------------------------------------------------------------------------
void ImagePipeline::set_free(int8_t idx) {
//-----------------------------------------------------------------------------

    //printf("free: %d\n", idx);
    _Free[ idx ].store(true, std::memory_order_release);

}
//-----------------------------------------------------------------------------
void ImagePipeline::set_all_free() {
//-----------------------------------------------------------------------------

    for (int i=0; i < 5;++i) {
        _Free[i].store(true);    
    }

}
//-----------------------------------------------------------------------------
void ImagePipeline::set_all_full() {
//-----------------------------------------------------------------------------

    for (int i=0; i < 5;++i) {
        _Free[i].store(false);    
    }

}
//-----------------------------------------------------------------------------
ImagePipeline::ImagePipeline()
//-----------------------------------------------------------------------------
{
    set_all_free();
}
//-----------------------------------------------------------------------------
static void join_thread(std::thread* t, const char* name)
//-----------------------------------------------------------------------------
{
    if ( t != nullptr && t->joinable() )
    {
        printf("I: joining thread: %s\n", name);
        t->join();
        printf("I: thread ended  : %s\n", name);
    }
}
//-----------------------------------------------------------------------------
void ImagePipeline::shutdown()
//-----------------------------------------------------------------------------
{
    puts("set all slots busy to exit camera thread");
    set_all_full();
    join_thread(&_threads[0], "camera");

    write(_CameraToDetect, ACTION_EXIT);
    join_thread(&_threads[1], "detect");
    
    write(_DetectToEncode, ACTION_EXIT);
    join_thread(&_threads[2], "encode");

}
void ImagePipeline::start_camera_1(std::function<void(Workitem*,CameraContext*)> process, CameraContext* context)
{
    _threads[0] = std::thread( [=] { camera_1(process,context); } );
}
void ImagePipeline::start_detect_2(std::function<void(Workitem*,DetectContext*)> process, DetectContext* context)
{
    _threads[1] = std::thread( [=] { detect_2(process,context); } );
}
/*
void start_encode_3(std::function<bool(Workitem*,void*)> process, void* context)
{
    _threads[2] = std::thread( [&] { encode_3(process,context); } );
}
*/
void ImagePipeline::run_encode_3(std::function<WORKER_RC(Workitem*,EncodeContext*)> process, EncodeContext* context)
{
    encode_3(process,context);
}
#include <functional>

#include <opencv2/opencv.hpp>

void loop_sendJPEG(std::function<void(std::vector<uchar>&)> sendJPEGbytes)
{
    cv::VideoCapture capture = cv::VideoCapture(0);

    const std::vector<int> JPEGparams = {cv::IMWRITE_JPEG_QUALITY, 50};

    cv::Mat frame;
    std::vector<uchar> jpegImage;
    for (;;)
    {
        if ( !capture.read(frame) )
        {
            printf("E: capture.read()\n");
            break;
        }
        else if ( !cv::imencode(".jpg", frame, jpegImage, JPEGparams))
        {
            printf("E: imencode(jpg)\n");
        }
        else
        {
            printf("I: sending JPEG bytes: %lukB\n", jpegImage.size() / 1024);
            sendJPEGbytes(jpegImage);
        }
    }
}
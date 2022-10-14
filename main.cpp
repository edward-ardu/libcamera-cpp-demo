#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <chrono>

#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>


#include "LibCamera.h"

using namespace cv;

int writeToFile(const char *fileName, uint8_t *buffer, uint32_t bufferSize ) {
    FILE* outF;
    outF = fopen(fileName, "wb");

    if( outF == NULL )
    {
        return 1;
    }

    if( fwrite(buffer, 1, bufferSize, outF) < 0 )
    {
        return 2;
    }

    fclose(outF);

    return 0;
}

int main() {
    time_t start_time = time(0);
    int frame_count = 0;
    LibCamera cam;
    // int width = 640;
    // int height = 480;
    int width = 1280;
    int height = 720;
    char key;

    int ret = cam.initCamera( {StreamRole::Viewfinder}, 1280, 720, formats::RGB888, 1, 0 ); // Test = AF not working too !
    //int ret = cam.initCamera( {StreamRole::Viewfinder}, 9152, 6944, formats::RGB888, 1, 0 ); // Test = AF not working too !
    // int ret = cam.initCamera( {StreamRole::Raw}, 9152, 6944, formats::SRGGB10_CSI2P, 1, 0 ); // AF not working !

    ControlList controls_;
    int64_t frame_time = 1000000 / 30;
    // Set frame rate
	controls_.set(controls::FrameDurationLimits, libcamera::Span<const int64_t, 2>({ frame_time, frame_time }));
    // Adjust the brightness of the output images, in the range -1.0 to 1.0
    controls_.set(controls::Brightness, 0.5);
    // Adjust the contrast of the output image, where 1.0 = normal contrast
    controls_.set(controls::Contrast, 1.5);
    // Set the exposure time
    controls_.set(controls::ExposureTime, 20000);
    cam.set(controls_);

    if (!ret) {
        bool flag, end;
        LibcameraOutData frameData;
        cam.startCamera();
        end = false;
        while (!end) {
            // Wait a frame
            flag = cam.readFrame(&frameData);
            if (!flag)
                continue;
            frame_count++;
            
            Mat im(height, width, CV_8UC3, frameData.imageData);

            imshow("libcamera-demo", im);
            key = waitKey(1);
            if (key == 'q') {
                break;
            } else if (key == 'f') {
                controls_.set(controls::AfTrigger, 0);
                cam.set(controls_);
            }
            // Force autofocus every 3 frames  
            if( frame_count % 10 == 0) {
                controls_.set(controls::AfTrigger, 0);
                cam.set(controls_);
                std::cout << "AE" << std::endl;
            }

            // Save the 20th frame
            // if( frame_count >= 20 ) {
            //     ret = writeToFile( "../frame.dat", frameData.imageData, frameData.size );
            //     if( ret )
            //     {
            //         std::cerr << "Failed to write to file ../frame.dat #" << ret << std::endl;
            //         return 1;
            //     }
            //     end = true;
            //     std::cout << "END" << std::endl;
            // }

            cam.returnFrameBuffer(frameData);
        }
        destroyAllWindows();
        cam.stopCamera();
        
    }
    cam.closeCamera();
    return 0;
}

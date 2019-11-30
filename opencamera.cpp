#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <cstdlib>
using namespace std;
using namespace cv;

// MJPEG
char* get_camerasrc_mjpeg(int devIndex) {
    const int cam_width=432;
    const int cam_height=240;
    const int cam_frames=30;
    static char str[255]={'\0'};
	snprintf(str, sizeof(str)-1
        , "v4l2src device=/dev/video%d ! image/jpeg,width=%d,height=%d,framerate=%d/1 ! jpegdec ! videoconvert ! video/x-raw ! appsink"
		, devIndex
		, cam_width
		, cam_height
		, cam_frames
		);
    printf("%s\n", str);
	return str;
}

// NV12
char* get_camerasrc_nv12(int devIndex) {
    const int cam_width=640;
    const int cam_height=480;
    const int cam_frames=30;
    static char str[255]={'\0'};
	snprintf(str, sizeof(str)-1
        , "v4l2src device=/dev/video%d ! video/x-raw,format=I420,framerate=%d/1,width=%d,height=%d ! appsink"
		, devIndex
		, cam_width
		, cam_height
		, cam_frames
		);
    printf("%s\n", str);
	return str;
}

int main(int argc,char* argv[]) 
{ 
	// check args
    if(argc < 2) {
        printf("Please provide the number of cameras, it must be 1,2 or 3.\n");
        exit(0);
    }
	errno = 0;
	char *endptr;
	long int count = strtol(argv[1], &endptr, 10);
	if (endptr == argv[1] || errno == ERANGE) {
		std::cerr << "Invalid parameter: " << argv[1] << '\n';
		exit (1);
	}
	if (count < 1 || count > 3) {
		std::cerr << "The parameter must be 1,2,3" << '\n';
		exit (1);
	}

	// check qt5 env
	const char* s = getenv("QTDIR");
	if (!s) {
		std::cerr << "Please execute the following command first: " << '\n';
		std::cerr << "\t. /usr/bin/setqt5env-eglfs" << '\n';
		exit (1);
	}

	VideoCapture *pCapture[3]={0};
	char windowName[10];
	int hasError;
	int devIndex=0;   // dev: /dev/video0
	for (int i=0; i<count; i++) {
		// mjpg
		pCapture[i] = new VideoCapture(get_camerasrc_mjpeg(devIndex),cv::CAP_GSTREAMER);
		if (!pCapture[i]->isOpened()) {
			printf("Fail to open camera.\n");
			exit (1);
		}
		// opencv window
		sprintf(windowName,"%d",i+1);
		namedWindow(windowName, 1);

		devIndex += 1;  // dev: /dev/video1, /dev/video2, etc ..
	}
	
	while (1) { 
		Mat frame;
		hasError = 0;
		for (int i=0; i<count; i++) {
			*pCapture[i] >> frame;
			if (frame.empty()) {
				printf("Fail to read frame.\n");
				hasError = 1;
				break;
			}
			sprintf(windowName,"%d",i+1);
			imshow(windowName,frame);
		}
		if (hasError) {
			break;
		}
		if (waitKey(30) & 0xFF == 'q') 
			break;
	} 
	
	// release
	for (int i=0; i<count; i++) {
		pCapture[i]->release();
		delete pCapture[i];
	}
	return 0; 
}


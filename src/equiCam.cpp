#include <ctime>
#include <ratio>
#include <chrono>
#include <thread>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <chrono>

#include "SharedQueue.h"
#include "SharedVideoCapture.h"
#include "Fisheye.h"
#include "Equirectangular.h"




using namespace cv;
using namespace std;
using namespace std::chrono;


Mat origFile;
Fisheye fe;
Equirectangular er;
Mat rotationMatrix;

Point2d PXtoLatLong(Point pos) {

	double ex = (double)(pos.x - er.frame.cols/2) / (er.frame.cols/2);
	double ey = (double)(pos.y - er.frame.rows/2) / (er.frame.rows/2);
	double longitude = ex * CV_PI;
	double latitude = ey * CV_PI/2;
	return Point2d(longitude, latitude);
}
void outCallBack(int event, int x, int y, int flags, void* userdata) {
	if ( event == EVENT_LBUTTONDOWN ) {
		Point2d r = PXtoLatLong(Point2d(x, y));
		Mat rot = fe.genRotMat(Vec3d(-r.y, r.x, 0));
		//rot = rot * er.rotationMatrix;
		rotationMatrix = er.rotationMatrix * rot;
	} else if ( event == EVENT_MOUSEWHEEL ) {
		double r = getMouseWheelDelta(flags) > 0 ? 10*CV_PI/180 : -10*CV_PI/180;
		Mat rot = fe.genRotMat(Vec3d(0, 0, r));
		//rot = rot * er.rotationMatrix;
		rotationMatrix = er.rotationMatrix * rot;
	} else if ( event == EVENT_LBUTTONUP ) {
	} else if ( event == EVENT_MOUSEMOVE ) {
	}
}

class Page {
public:
	Page(Mat m, high_resolution_clock::time_point t) { frame = m; time = t; }
	Mat frame;
	high_resolution_clock::time_point time;
};

SharedVideoCapture cap(1); // open the default camera
SharedQueue<Page> frames;

void cameraThread() {
	Mat frame;
	while (cap.isOpened()) {
		cap.read(frame);
		frames.push_back(Page(frame.clone(), high_resolution_clock::now()));
	}
	cap.release();
}


int main(int argc, char** argv) {
	
	if(!cap.isOpened())  // check if we succeeded
		return -1;

	namedWindow("converted", 1);
	setMouseCallback("converted", outCallBack, NULL);

	//cap.set(CAP_PROP_FRAME_HEIGHT, 1440);
	//cap.set(CAP_PROP_FRAME_WIDTH, 1440);
	cap.set(CAP_PROP_FRAME_HEIGHT, 1200);
	cap.set(CAP_PROP_FRAME_WIDTH, 1200);
	int w = cap.get(CAP_PROP_FRAME_WIDTH);
	int h = cap.get(CAP_PROP_FRAME_HEIGHT);
	
	Mat display;
	//namedWindow("edges",1);

	// prepare required variables for the main loop
	bool is_first_frame = true;
	int sleep;
	high_resolution_clock::time_point start_time;
	high_resolution_clock::time_point end_time;
	duration<double, std::milli> time_span;
	high_resolution_clock::time_point last_frame_time = high_resolution_clock::now();

	// start camera thread
    thread t1(cameraThread);

	int counter = 1;
	
	rotationMatrix = fe.genRotMat(Vec3d(0, 0, 0));
	while (cap.isOpened()) {

		if (frames.size() == 0) {
			//printf("waiting for new frame\n");
			continue;
		}

		// start timer and read next frame with timestamp
		start_time = high_resolution_clock::now();
		if (frames.size() > 1) {
			printf("SKIPPING %d frames\n", frames.size()-1);
			while (frames.size() > 2)
				frames.pop_front();
			last_frame_time = frames.front().time;
			frames.pop_front();
		}
		Page p = frames.front();
		frames.pop_front();
		duration<double, std::milli> frame_duration = p.time - last_frame_time;
		last_frame_time = p.time;

		// prepare frame
		Mat frame;
		resize(p.frame, frame, Size(), 0.25, 0.25);


		fe.frame = frame;
		er = fe.toEquirectangular(rotationMatrix);


		imshow("orig", frame);
		imshow("converted", er.frame);



		end_time = high_resolution_clock::now();
		time_span = end_time - start_time;
		// calculate remaining time to sleep in order to match camera's fps
		sleep = frame_duration.count() - time_span.count() - 1; // substract 1 so we are always ahead of the camera
		//printf("sleep = %d ms\n", sleep);
		if (is_first_frame) {
			sleep = 1;
			is_first_frame = false;
		}
		if (sleep > 0) {
			int val = waitKey(sleep);
			if (val > 0) {
				printf("key = %d\n", val);
				switch (val) {
				case 27:  // esc
				case 'q':
					cap.release();
					break;
				case ' ':
					imwrite("out" + to_string(counter) + ".jpg", frame);
					counter++;
					break;
				}
			}
		} else {
			printf("too many frames expected ----------------------\n");
			printf("we are behind by %d ms\n", sleep);
			waitKey(1);
			//exit(1);
		}
	}
	// the camera will be deinitialized automatically in VideoCapture destructor
	t1.join();
	return 0;




}

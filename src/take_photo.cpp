#include <ctime>
#include <ratio>
#include <chrono>
#include <thread>
#include "SharedQueue.h"
#include "SharedVideoCapture.h"

using namespace std;
using namespace std::chrono;

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

Mat toEquirectangular(Mat orig) {
	int radius = orig.rows/2; // 720
	//int maxAngle = 3200;
	int maxAngle = radius * 4.44;
	Mat out(radius, maxAngle, orig.type());
	for (int a = 0; a < maxAngle; a++) {
		for (int r = 0; r < radius; r++) {
			float x = sin(a*2*CV_PI / maxAngle) * r + orig.cols/2;
			float y = cos(a*2*CV_PI / maxAngle) * r + orig.rows/2;
			
			//cout << "access " << (int)x << " , " << (int)y << "  " << orig.rows << endl;
			out.at<Vec3b>(r, a) = orig.at<Vec3b>(y, x);
			//orig.at<Vec3b>(y, x) = Vec3b(255, 0, 0);
		}
	}
	return out;
}

int main(int args, char** argv) {
	if(!cap.isOpened()) {  // check if we succeeded
		cap.open(0);
		if(!cap.isOpened())
			return -1;
	}

	cap.set(CAP_PROP_FRAME_HEIGHT, 1440);
	cap.set(CAP_PROP_FRAME_WIDTH, 1440);
	cout << "w " << cap.get(CAP_PROP_FRAME_WIDTH) << endl;
	cout << "h " << cap.get(CAP_PROP_FRAME_HEIGHT) << endl;
//	cap.set(CAP_PROP_FRAME_HEIGHT, 1200);
//	cap.set(CAP_PROP_FRAME_WIDTH, 1200);
//	cap.set(CAP_PROP_FRAME_HEIGHT, 720);
//	cap.set(CAP_PROP_FRAME_WIDTH, 1280);


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
	
	bool mkvideo = false;
	VideoWriter video;
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
		
		cout << "fps: " << 1000/frame_duration.count() << endl;

		// prepare frame
		Mat frame = p.frame;
		//flip(frame, frame, 1);
		//Mat game = frame.clone();
		
		//Mat equir = toEquirectangular(frame);
		
		if (mkvideo)
			video.write(frame);

		Mat show;
		resize(frame, show, Size(), 0.5, 0.5);
		imshow("orig", show);
		//imshow("converted", equir);



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
				case 'v':
					mkvideo = !mkvideo;
					if (mkvideo) {
						video = VideoWriter("outcpp.avi",VideoWriter::fourcc('M','J','P','G'),15, Size(w, h));
					}
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
	video.release();
	// the camera will be deinitialized automatically in VideoCapture destructor
	t1.join();
	return 0;
}

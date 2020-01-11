#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <chrono>

using namespace cv;
using namespace std;

using namespace std::chrono;


Mat origFile;
Mat orig;
Mat out;

bool mouseDown = false;
double lastProp = -1;
void update(double);
void origCallBack(int event, int x, int y, int flags, void* userdata) {
	int radius = orig.rows/2;
	int dx = x - orig.cols/2;
	int dy = y - orig.rows/2;
	double prop = sqrt(dx*dx + dy*dy) / radius;
    if ( event == EVENT_LBUTTONDOWN ) {
    	mouseDown = true;
		update(prop);
	} else if ( event == EVENT_LBUTTONUP ) {
		mouseDown = false;
	} else if ( event == EVENT_MOUSEMOVE ) {
		//if (mouseDown)
		//	update(prop);
	}
}
void outCallBack(int event, int x, int y, int flags, void* userdata) {
    if ( event == EVENT_LBUTTONDOWN ) {
    	mouseDown = true;
		update((double)y/out.rows);
	} else if ( event == EVENT_LBUTTONUP ) {
		mouseDown = false;
	} else if ( event == EVENT_MOUSEMOVE ) {
		if (mouseDown)
			update((double)y/out.rows);
	}
}
void update(double prop) {
	if (prop == lastProp) {
		return;
	}
	lastProp = prop;
	orig = origFile.clone();

//	int radius = 720;
//	int maxAngle = 3200;
//	int maxAngle = 2000;
	int radius = orig.rows/2;
	int maxAngle = prop*radius * 2*CV_PI;
//	int maxAngle = (int)radius * 2 * CV_PI;

	out = Mat(radius, maxAngle, orig.type());
	

	float dx = orig.cols/2;
	float dy = orig.rows/2;
	//float dy = orig.rows/4;
	for (int a = 0; a < maxAngle; a++) {
		for (int r = 0; r < radius; r++) {
			float x = sin(a*2*CV_PI / maxAngle) * r + dx;
			float y = cos(a*2*CV_PI / maxAngle) * r + dy;
			
			//cout << "access " << (int)x << " , " << (int)y << "  " << orig.rows << endl;
			out.at<Vec3b>(r, a) = orig.at<Vec3b>(y, x);
			//orig.at<Vec3b>(y, x) = Vec3b(255, 0, 0);
		}
	}
	line(out, Point(0, maxAngle*radius/(2*CV_PI*radius)), Point(out.cols, maxAngle*radius/(2*CV_PI*radius)), Scalar(0, 0, 255));
	circle(orig, Point(orig.cols/2, orig.rows/2), maxAngle*radius/(2*CV_PI*radius), Scalar(0, 0, 255));
	
//	resize(out, out, Size(), 0.25, 0.25);
	imshow("orig", orig);
	imshow("out", out);
}
int main(int argc, char** argv) {
	
	if (argc != 2) {
		cerr << "missing file" << endl;
		return -1;
	}
	
	origFile = imread(argv[1]);
	cout << "orig channels: " << origFile.channels() << endl;
	//circle(origFile, Point(origFile.cols/2, origFile.rows*0/4), origFile.cols/2, Scalar(0, 255, 0), 1);

	namedWindow("out", 1);
	setMouseCallback("out", outCallBack, NULL);
	namedWindow("orig", 1);
	setMouseCallback("orig", origCallBack, NULL);

	update(1.0);

	while (waitKey(0) != 27) {
	}
	destroyAllWindows();
	
	return 0;
}

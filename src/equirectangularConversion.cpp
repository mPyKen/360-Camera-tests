#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <chrono>
#include "Fisheye.h"
#include "Equirectangular.h"

using namespace cv;
using namespace std;

using namespace std::chrono;


Mat origFile;
Fisheye fe;
Equirectangular er;
Fisheye rev;

Point2d PXtoLatLong(Point pos) {

	double ex = (double)(pos.x - er.frame.cols/2) / (er.frame.cols/2);
	double ey = (double)(pos.y - er.frame.rows/2) / (er.frame.rows/2);
	double longitude = ex * CV_PI;
	double latitude = ey * CV_PI/2;
	return Point2d(longitude, latitude);
}
void outCallBack(int event, int x, int y, int flags, void* userdata) {
	if ( event == EVENT_LBUTTONDOWN ) {
		/*
		Point2d r = PXtoLatLong(Point2d(x, y));
		Point3d rot = er.rotation;
		rot.x += r.x;
		rot.y += r.y;
		cout << rot.x*180/CV_PI << ", " << rot.y*180/CV_PI << ", " << rot.z << endl;
		er = fe.toEquirectangular(rot);
		imshow("out", er.frame);
		*/

		Point2d r = PXtoLatLong(Point2d(x, y));
		Mat rot = fe.genRotMat(Vec3d(-r.y, r.x, 0));
		//rot = rot * er.rotationMatrix;
		rot = er.rotationMatrix * rot;
		
		er = fe.toEquirectangular(rot);
		imshow("out", er.frame);
	} else if ( event == EVENT_MOUSEWHEEL ) {
		double r = getMouseWheelDelta(flags) > 0 ? 10*CV_PI/180 : -10*CV_PI/180;
		Mat rot = fe.genRotMat(Vec3d(0, 0, r));
		//rot = rot * er.rotationMatrix;
		rot = er.rotationMatrix * rot;
		
		er = fe.toEquirectangular(rot);
		imshow("out", er.frame);
	} else if ( event == EVENT_LBUTTONUP ) {
	} else if ( event == EVENT_MOUSEMOVE ) {
	}
}

int main(int argc, char** argv) {
	
	if (argc != 2) {
		cerr << "missing file" << endl;
		return -1;
	}
	
	origFile = imread(argv[1]);
	//cout << "orig channels: " << origFile.channels() << endl;
	//circle(origFile, Point(origFile.cols/2, origFile.rows*0/4), origFile.cols/2, Scalar(0, 255, 0), 1);

	fe.frame = origFile.clone();
	fe.ha = 0;
	fe.va = -CV_PI/2;
	//fe.aperture = CV_PI;
	//fe.va = -CV_PI/4;
	//fe.va = 0;
	Mat rot = fe.genRotMat(Vec3d(0, 0, 0));
high_resolution_clock::time_point start_time = high_resolution_clock::now();
	er = fe.toEquirectangular(rot);
high_resolution_clock::time_point end_time = high_resolution_clock::now();
duration<double, std::milli> time_span = end_time - start_time;
	cout << "elapsed: " << time_span.count() << endl;
	
	namedWindow("out", 1);
	setMouseCallback("out", outCallBack, NULL);

	Mat tmp;
	//resize(er.frame, tmp, Size(), 0.25, 0.25);
	resize(er.frame, tmp, Size(), 1.0, 1.0);

	imshow("orig", fe.frame);
	imshow("out", tmp);

	//rev = er.toFisheye(fe.aperture, 0, -CV_PI/2);
	//rev = er.toFisheye(fe.aperture, 0, 0);
	rev = er.toFisheye(90*CV_PI/180, 0, -20*CV_PI/180);
	imshow("reverse", rev.frame);

waitKey(1);
	while (waitKey(0) != 27) {
	}
	destroyAllWindows();
	
	return 0;
}

#pragma once

#include "opencv2/opencv.hpp"

class Equirectangular;
#include "Equirectangular.h"


//#define DEGREE_90


using namespace cv;
using namespace std;

class Fisheye {
public:
	Mat frame;
	double ha; // horizontal angle
	double va; // vertical angle
	double aperture;

	Fisheye();
	
//	void normalize(double&, double&);
	double radius() { return frame.cols/2; }
//	double dist(double, double, double, double);
//	bool inRange(double, double, double, double, double);
//	double mod2PI(double);
//	bool isInFrame(double, double);
	static Vec3d coordToVec(Point2d);
	static Point2d rotateCoordinateArray(Point2d coord, double[3][3]);
	Vec3b at(double, double);
	static Mat genRotMat(Vec3d);
	Equirectangular toEquirectangular(Mat);

};

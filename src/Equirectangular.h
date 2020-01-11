#pragma once

#include "opencv2/opencv.hpp"

class Fisheye;
#include "Fisheye.h"

using namespace cv;
using namespace std;

class Equirectangular {
private:
	double rotationMatrixTransposedArray[3][3];
	double anchorRotationMatrixArray[3][3];

public:
	Mat frame;
	Mat rotationMatrix;

	Equirectangular();
	double radius();
	Fisheye toFisheye(double, double, double);
	Vec3b at(double, double, double);
	
};

#include "Equirectangular.h"

Equirectangular::Equirectangular() {

}

double Equirectangular::radius() {
	return frame.rows/2;
}
Vec3b Equirectangular::at(double aperture, double x, double y) {
	
//	if (x*x+y*y > 1)
//		return Vec3b(255, 0, 0);
	
	double r = sqrt(x*x + y*y) * aperture / 2;
	double theta = atan2(y, x) - CV_PI/2;


	#ifndef DEGREE_90
	double vec[] = {
		-sin(theta) * sin(r),
		cos(theta) * sin(r),
		cos(r)
	};
	//Mat target = anchorRotationMatrix * Mat(v);
	double px = anchorRotationMatrixArray[0][0] * vec[0] + anchorRotationMatrixArray[0][1] * vec[1] + anchorRotationMatrixArray[0][2] * vec[2];
	double py = anchorRotationMatrixArray[1][0] * vec[0] + anchorRotationMatrixArray[1][1] * vec[1] + anchorRotationMatrixArray[1][2] * vec[2];
	double pz = anchorRotationMatrixArray[2][0] * vec[0] + anchorRotationMatrixArray[2][1] * vec[1] + anchorRotationMatrixArray[2][2] * vec[2];


	double longitude = atan2(px, pz);
	double latitude = asin(py);

	#else
	double latitude = r - CV_PI/2;
	double longitude = theta;
	#endif
	
	Point2d origin = Fisheye::rotateCoordinateArray(Point2d(longitude, latitude), rotationMatrixTransposedArray);
	
	double ex = origin.x / CV_PI;
	double ey = origin.y / (CV_PI/2);
	//ex *= -1; // mirror
	double exr = ex * (frame.cols/2) + frame.cols/2;
	double eyr = ey * (frame.rows/2) + frame.rows/2;
	
	if (eyr == frame.rows)
		eyr -= 1.0;
	if (exr == frame.cols)
		exr -= 1.0;
	return frame.at<Vec3b>(eyr, exr);
}
Fisheye Equirectangular::toFisheye(double aperture, double ha, double va) {
	Fisheye fe;
	Mat oframe = Mat(frame.rows, frame.rows, frame.type());


	Mat rotationMatrixTransposed;
	transpose(rotationMatrix, rotationMatrixTransposed);
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			rotationMatrixTransposedArray[y][x] = rotationMatrixTransposed.at<double>(y, x);
		}
	}
	Mat anchorRotationMatrix = Fisheye::genRotMat(Vec3d(-va, ha, 0));
	for (int y = 0; y < 3; y++) {
		for (int x = 0; x < 3; x++) {
			anchorRotationMatrixArray[y][x] = anchorRotationMatrix.at<double>(y, x);
		}
	}

	
	for (int eyr = 0; eyr < oframe.rows; eyr++) {
		for (int exr = 0; exr < oframe.cols; exr++) {
			double ex = (double)(exr - oframe.cols/2) / (oframe.cols/2);
			double ey = (double)(eyr - oframe.rows/2) / (oframe.rows/2);
			oframe.at<Vec3b>(eyr, exr) = at(aperture, ex, ey);
		}
	}
	circle(oframe, Point(oframe.cols/2, oframe.rows/2), oframe.cols/2, Scalar(255, 0, 0), 3);
	
	fe.frame = oframe;
	fe.ha = ha;
	fe.va = va;
	fe.aperture = aperture;
	return fe;
}

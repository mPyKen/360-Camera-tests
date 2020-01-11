#include "Fisheye.h"

Fisheye::Fisheye() {
	aperture = 235*2*CV_PI/360;
}
/*
double Fisheye::dist(double ax, double ay, double bx, double by) {
	return acos(sin(ay)*sin(by) + cos(ay)*cos(by)*cos(ax-bx));
}
bool Fisheye::inRange(double h, double v, double ha, double va, double delta) {
	return dist(h, v, ha, va) <= delta;
}
double Fisheye::mod2PI(double a) {
	double b = a;
	while (b >= 2*CV_PI)
		b -= 2*CV_PI;
	while (b < 0)
		b += 2*CV_PI;
	return b;
}
*/
/*
bool Fisheye::isInFrame(double h, double v) {
	if (inRange(h, v, ha, va, CV_PI/2))
		return true;
	return false;
}
*/
/*
void Fisheye::normalize(double &h, double &v) {
	if (max(v, -v) > CV_PI/2) {
		h = mod2PI(h + CV_PI);
		if (v > 0)
			v = CV_PI - v;
		else
			v = CV_PI + v;
	}
}
*/
Vec3d Fisheye::coordToVec(Point2d coord) {
	Vec3d v;
	v[2] = cos(coord.y) * cos(coord.x);
	v[0] = cos(coord.y) * sin(coord.x);
	v[1] = sin(coord.y);
	return v;
}
Point2d Fisheye::rotateCoordinateArray(Point2d coord, double rotation[3][3]) {

	double vec[] = {
		cos(coord.y) * sin(coord.x),
		sin(coord.y),
		cos(coord.y) * cos(coord.x)
	};
	//Mat mult = rotation * Mat(vec);
	double x = rotation[0][0] * vec[0] + rotation[0][1] * vec[1] + rotation[0][2] * vec[2];
	double y = rotation[1][0] * vec[0] + rotation[1][1] * vec[1] + rotation[1][2] * vec[2];
	double z = rotation[2][0] * vec[0] + rotation[2][1] * vec[1] + rotation[2][2] * vec[2];

	Point2d res;
	res.x = atan2(x, z);
	res.y = asin(y);
	return res;
}

Vec3b Fisheye::at(double longitude, double latitude) {
//	if (!isInFrame(h, v))
//		return Vec3b(0, 0, 0);
//	return Vec3b(255, 0, 0);

	#ifndef DEGREE_90
	double refx = ha, refy = va;
	//cout << endl << "1. " << exr << " / " << out.cols << ", " << eyr << " / " << out.rows << " --> " << ex << ", " << ey << endl;
	double r = 2* acos(sin(refy) * sin(latitude) + cos(refy)*cos(latitude)*cos(longitude-refx)) / aperture;
	double theta = atan2(cos(latitude)*sin(longitude-refx), cos(refy)*sin(latitude) - sin(refy)*cos(latitude)*cos(longitude-refx));
	#else
	// case: 90°
	double r = 2*(CV_PI/2 + latitude) / aperture;
	double theta = longitude;
	#endif

	//double r = 2*atan2(sqrt(px*px + pz*pz), py) / aperture;
	//double theta = atan2(pz, px);
	if (r > 1.0) {
		return Vec3b(0, 255, 0);
	}
	double x = r * cos(theta);
	double y = r * sin(theta);
	//cout << "4. " << x << ", " << y << endl;

	double fx = x*radius()+radius();
	double fy = y*radius()+radius();

	//cout << "5. " << fx << ", " << fy << endl;
	return frame.at<Vec3b>(fy, fx);
}
Mat Fisheye::genRotMat(Vec3d theta) {
	// x - default axis; y - vertical axis; z - depth axis
	
	// Calculate rotation about x axis
	Mat R_x = (Mat_<double>(3,3) <<
			   1,		0,			  0,
			   0,		cos(theta[0]), -sin(theta[0]),
			   0,		sin(theta[0]), cos(theta[0])
			   );
	// Calculate rotation about y axis
	Mat R_y = (Mat_<double>(3,3) <<
			   cos(theta[1]),	0,	sin(theta[1]),
			   0,				1,	0,
			   -sin(theta[1]),	0,	cos(theta[1])
			   );
	 
	// Calculate rotation about z axis
	Mat R_z = (Mat_<double>(3,3) <<
			   cos(theta[2]),	-sin(theta[2]),	0,
			   sin(theta[2]),	cos(theta[2]),	0,
			   0,				0,				1);
	// Combined rotation matrix
	Mat R = R_z * R_y * R_x;
	return R;
}
Equirectangular Fisheye::toEquirectangular(Mat rotation) {
	// http://paulbourke.net/dome/dualfish2sphere/
	Equirectangular out;
	Mat oframe = Mat(CV_PI*radius(), 2*CV_PI*radius(), frame.type());
	double rotationArray[3][3] = {
		{rotation.at<double>(0, 0), rotation.at<double>(0, 1), rotation.at<double>(0, 2)},
		{rotation.at<double>(1, 0), rotation.at<double>(1, 1), rotation.at<double>(1, 2)},
		{rotation.at<double>(2, 0), rotation.at<double>(2, 1), rotation.at<double>(2, 2)}
	};
	
	for (int eyr = 0; eyr < oframe.rows; eyr++) {
		double ey = (double)(eyr - oframe.rows/2) / (oframe.rows/2);
		double latitude = ey * CV_PI/2;
		for (int exr = 0; exr < oframe.cols; exr++) {
			double ex = (double)(exr - oframe.cols/2) / (oframe.cols/2);
			double longitude = ex * CV_PI;
			Point2d viewpoint = rotateCoordinateArray(Point2d(longitude, latitude), rotationArray);
			oframe.at<Vec3b>(eyr, exr) = at(viewpoint.x, viewpoint.y);
		}
	}
	rectangle(oframe, Point(oframe.cols/2-1, oframe.rows/2-1), Point(oframe.cols/2+1, oframe.rows/2+1), Scalar(0, 0, 255), -1);
	out.frame = oframe;
	out.rotationMatrix = rotation;
	return out;
}


#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include "opencv2/opencv.hpp"

using namespace cv;

class SharedVideoCapture {
public:
	SharedVideoCapture(int);
	~SharedVideoCapture();

	bool isOpened();
	int read(Mat&);
	void release();
	
	double get(int);
	bool set(int, double);
	bool open(int);

private:
	VideoCapture cap;
	std::mutex mutex_;
	std::condition_variable cond_;
};
SharedVideoCapture::~SharedVideoCapture() {}
SharedVideoCapture::SharedVideoCapture(int i) {
	std::unique_lock<std::mutex> mlock(mutex_);
	cap = VideoCapture(i);
	mlock.unlock();
}
bool SharedVideoCapture::isOpened() {
	return cap.isOpened();
}
int SharedVideoCapture::read(Mat &mat) {
	std::unique_lock<std::mutex> mlock(mutex_);
	int i = cap.read(mat);
	mlock.unlock();
	return i;
}
void SharedVideoCapture::release() {
	std::unique_lock<std::mutex> mlock(mutex_);
	cap.release();
	mlock.unlock();
}
double SharedVideoCapture::get(int i) {
	return cap.get(i);
}
bool SharedVideoCapture::set(int i, double v) {
	std::unique_lock<std::mutex> mlock(mutex_);
	bool ret = cap.set(i, v);
	mlock.unlock();
	return ret;
}
bool SharedVideoCapture::open(int i) {
	return cap.open(i);
}

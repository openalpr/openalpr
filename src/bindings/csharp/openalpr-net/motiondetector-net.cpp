#include "stdafx.h"
#include "motiondetector-net.h"
#include "lock-net.h"
#include "bitmapmat-net.h"

using namespace openalprnet;

void AlprMotionDetectionNet::ResetMotionDetection(Bitmap^ bitmap)
{
	BitmapMat^ wrapper = gcnew BitmapMat(bitmap);
	ResetMotionDetection(wrapper->Value);
	delete wrapper;
}

void AlprMotionDetectionNet::ResetMotionDetection(String^ filename)
{
	cv::Mat mat = cv::imread(marshal_as<std::string>(filename));
	ResetMotionDetection(mat);
}

void AlprMotionDetectionNet::ResetMotionDetection(MemoryStream^ memoryStream)
{
	return ResetMotionDetection(memoryStream->ToArray());
}

void AlprMotionDetectionNet::ResetMotionDetection(array<Byte>^ byteArray)
{
	std::vector<char> buffer = AlprHelper::ToVector(byteArray);
	cv::Mat mat = cv::imdecode(buffer, CV_LOAD_IMAGE_COLOR);
	ResetMotionDetection(mat);
}

System::Drawing::Rectangle AlprMotionDetectionNet::MotionDetect(Bitmap^ bitmap)
{
	BitmapMat^ wrapper = gcnew BitmapMat(bitmap);
	System::Drawing::Rectangle motion = MotionDetect(wrapper->Value);
	delete wrapper;
	return motion;
}

System::Drawing::Rectangle AlprMotionDetectionNet::MotionDetect(String^ filename)
{
	cv::Mat mat = cv::imread(marshal_as<std::string>(filename));
	System::Drawing::Rectangle motion = MotionDetect(mat);
	return motion;
}

System::Drawing::Rectangle AlprMotionDetectionNet::MotionDetect(MemoryStream^ memoryStream)
{
	return MotionDetect(memoryStream->ToArray());
}

System::Drawing::Rectangle AlprMotionDetectionNet::MotionDetect(array<Byte>^ byteArray)
{
	std::vector<char> buffer = AlprHelper::ToVector(byteArray);
	cv::Mat mat = cv::imdecode(buffer, CV_LOAD_IMAGE_COLOR);
	return MotionDetect(mat);
}

void AlprMotionDetectionNet::ResetMotionDetection(cv::Mat mat)
{
	Lock lock(this);
	this->m_motionDetector->ResetMotionDetection(&mat);
}

System::Drawing::Rectangle AlprMotionDetectionNet::MotionDetect(cv::Mat mat)
{
	Lock lock(this);
	cv::Rect rect = this->m_motionDetector->MotionDetect(&mat);
	return AlprHelper::ToRectangle(rect);
}
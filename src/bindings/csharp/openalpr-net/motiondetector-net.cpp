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
	BitmapMat^ wrapper = gcnew BitmapMat(filename);
	ResetMotionDetection(wrapper->Value);
	delete wrapper;
}

void AlprMotionDetectionNet::ResetMotionDetection(MemoryStream^ memoryStream)
{
	BitmapMat^ wrapper = gcnew BitmapMat(memoryStream);
	ResetMotionDetection(wrapper->Value);
	delete wrapper;
}

void AlprMotionDetectionNet::ResetMotionDetection(array<Byte>^ byteArray)
{
	BitmapMat^ wrapper = gcnew BitmapMat(byteArray);
	ResetMotionDetection(wrapper->Value);
	delete wrapper;
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
	BitmapMat^ wrapper = gcnew BitmapMat(filename);
	System::Drawing::Rectangle motion = MotionDetect(wrapper->Value);
	delete wrapper;
	return motion;
}

System::Drawing::Rectangle AlprMotionDetectionNet::MotionDetect(MemoryStream^ memoryStream)
{
	BitmapMat^ wrapper = gcnew BitmapMat(memoryStream);
	System::Drawing::Rectangle motion = MotionDetect(wrapper->Value);
	delete wrapper;
	return motion;
}

System::Drawing::Rectangle AlprMotionDetectionNet::MotionDetect(array<Byte>^ byteArray)
{
	BitmapMat^ wrapper = gcnew BitmapMat(byteArray);
	System::Drawing::Rectangle motion = MotionDetect(wrapper->Value);
	delete wrapper;
	return motion;
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
#include "stdafx.h"
#include "bitmapmat-net.h"

using namespace openalprnet;

BitmapMat::BitmapMat(array<Byte>^ byteArray)
{
	m_bitmap = new cv::Mat();
	std::vector<char> buffer = AlprHelper::ToVector(byteArray);
	cv::imdecode(buffer, CV_LOAD_IMAGE_COLOR, this->m_bitmap);
}

BitmapMat::BitmapMat(Bitmap^ bitmap)
{
	m_bitmap = new cv::Mat();

	MemoryStream^ ms = gcnew MemoryStream();
	bitmap->Save(ms, ImageFormat::Png);

	std::vector<char> buffer = AlprHelper::ToVector(ms->ToArray());
	cv::imdecode(buffer, CV_LOAD_IMAGE_COLOR, this->m_bitmap);

	delete ms;
}

BitmapMat::BitmapMat(MemoryStream^ memoryStream)
{
	m_bitmap = new cv::Mat();
	std::vector<char> buffer = AlprHelper::ToVector(memoryStream->ToArray());
	cv::imdecode(buffer, CV_LOAD_IMAGE_COLOR, this->m_bitmap);
}

BitmapMat::BitmapMat(String^ filename)
{
	m_bitmap = new cv::Mat();
	array<Byte>^ byteArray = File::ReadAllBytes(filename);
	std::vector<char> buffer = AlprHelper::ToVector(byteArray);
	cv::imdecode(buffer, CV_LOAD_IMAGE_COLOR, this->m_bitmap);
	delete byteArray;
}


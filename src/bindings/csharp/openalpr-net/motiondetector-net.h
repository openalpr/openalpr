#pragma once

#include "motiondetector.h" // alpr
#include "helper-net.h"

using namespace alpr;

namespace openalprnet
{
	public ref class AlprMotionDetectionNet : IDisposable {
	public:
		AlprMotionDetectionNet() : m_motionDetector(new MotionDetector())
		{

		}

		void AlprMotionDetectionNet::ResetMotionDetection(Bitmap^ bitmap);
		void AlprMotionDetectionNet::ResetMotionDetection(String^ filename);
		void AlprMotionDetectionNet::ResetMotionDetection(MemoryStream^ memoryStream);
		void AlprMotionDetectionNet::ResetMotionDetection(array<Byte>^ byteArray);
		System::Drawing::Rectangle AlprMotionDetectionNet::MotionDetect(Bitmap^ bitmap);
		System::Drawing::Rectangle AlprMotionDetectionNet::MotionDetect(String^ filename);
		System::Drawing::Rectangle AlprMotionDetectionNet::MotionDetect(MemoryStream^ memoryStream);
		System::Drawing::Rectangle AlprMotionDetectionNet::MotionDetect(array<Byte>^ byteArray);

	private:

		~AlprMotionDetectionNet()
		{
			if (this->m_disposed)
			{
				return;
			}

			this->!AlprMotionDetectionNet();
			this->m_disposed = true;
		}

		!AlprMotionDetectionNet()
		{
			delete m_motionDetector;
		}

	private:
		void ResetMotionDetection(cv::Mat mat);
		System::Drawing::Rectangle MotionDetect(cv::Mat mat);

	private:
		MotionDetector* m_motionDetector;
		bool m_disposed;
	};
}
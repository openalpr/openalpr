#pragma once

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "helper-net.h"

using namespace System;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::IO;

namespace openalprnet {

	private ref class BitmapMat : IDisposable
	{
	private:
		cv::Mat* m_bitmap;
		bool m_disposed;
	public:
		BitmapMat(array<Byte>^ byteArray);
		BitmapMat(Bitmap^ bitmap);
		BitmapMat(MemoryStream^ memoryStream);
		BitmapMat(String^ filename);

		~BitmapMat()
		{
			if (this->m_disposed)
			{
				return;
			}

			this->!BitmapMat();
			this->m_disposed = true;
		}

		!BitmapMat()
		{
			m_bitmap->release();
			delete m_bitmap;
		}

		property cv::Mat Value {
			cv::Mat get()
			{
				return *m_bitmap;
			}
		}

	};
}
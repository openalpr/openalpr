#pragma once

#include "opencv2/imgproc/imgproc.hpp"

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
			delete[] m_bitmap->data;
		}

		property cv::Mat Value {
			cv::Mat get()
			{
				cv::Mat value = this->m_bitmap->clone();
				return value;
			}
		}

	private:

		static cv::Mat* BitmapToMat(Bitmap^ bitmap)
		{
			int channels = 0;

			switch (bitmap->PixelFormat)
			{
			case PixelFormat::Format8bppIndexed:
			case PixelFormat::Format1bppIndexed:
				channels = 1;
				break;
			case PixelFormat::Format24bppRgb:
				channels = 3;
				break;
			case PixelFormat::Format32bppRgb:
			case PixelFormat::Format32bppArgb:
			case PixelFormat::Format32bppPArgb:
				channels = 4;
				break;
			default:
				throw gcnew NotImplementedException();
			}

			BitmapData^ bitmapData = bitmap->LockBits(
				System::Drawing::Rectangle(0, 0, bitmap->Width, bitmap->Height),
				ImageLockMode::ReadOnly,
				bitmap->PixelFormat
				);

			const int totalBytes = bitmap->Height * bitmapData->Stride;

			char *dst = new char[totalBytes];
			::memcpy(dst, bitmapData->Scan0.ToPointer(), totalBytes);

			cv::Mat* dstMat = new cv::Mat(cv::Size(bitmap->Width, bitmap->Height), CV_8UC(channels), dst);

			bitmap->UnlockBits(bitmapData);

			return dstMat;
		}

		static cv::Mat* MemoryStreamBitmapToMat(MemoryStream^ memoryStream)
		{
			Bitmap^ bitmap = gcnew Bitmap(memoryStream);
			cv::Mat* mat = BitmapToMat(bitmap);
			delete bitmap;
			return mat;
		}

		static cv::Mat* ByteArrayToMat(array<Byte>^ byteArray)
		{
			MemoryStream^ ms = gcnew MemoryStream(byteArray);
			cv::Mat* mat = MemoryStreamBitmapToMat(ms);
			delete ms;
			return mat;
		}

	};
}
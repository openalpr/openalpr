#pragma once

#using <mscorlib.dll>
#include <msclr\marshal_cppstd.h>

#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "alpr.h"
#include "opencv2/imgproc/imgproc.hpp"

#include <vector>

using namespace alpr;

using namespace msclr::interop;

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::IO;
using namespace System::Collections::Generic;

namespace openalprnet
{
	private ref class AlprHelper sealed
	{
	public:

		static std::vector<char> ToVector(array<char>^ src)
		{
			std::vector<char> result(src->Length);
			pin_ptr<char> pin(&src[0]);
			char *first(pin), *last(pin + src->Length);
			std::copy(first, last, result.begin());
			return result;
		}

		static std::vector<char> ToVector(array<Byte>^ src)
		{
			std::vector<char> result(src->Length);
			pin_ptr<Byte> pin(&src[0]);
			char* pch = reinterpret_cast<char*>(pin);
			char *first(pch), *last(pch + src->Length);
			std::copy(first, last, result.begin());
			return result;
		}

		static Bitmap^ MatToBitmap(cv::Mat mat)
		{
			if (mat.empty())
			{
				return nullptr;
			}

			const int width = mat.size().width;
			const int height = mat.size().height;
			const int channels = mat.channels();
			const int totalSize = mat.total();
			void* data = reinterpret_cast<void*>(mat.data);
			Bitmap ^bitmap;

			if (channels == 1)
			{
				bitmap = gcnew Bitmap(width, height, PixelFormat::Format8bppIndexed);

				ColorPalette ^palette = bitmap->Palette;
				for (int i = 0; i < 256; i++)
				{
					palette->Entries[i] = Color::FromArgb(i, i, i);
				}

				bitmap->Palette = palette;
			}
			else
			{
				bitmap = gcnew Bitmap(width, height, PixelFormat::Format24bppRgb);
			}

			System::Drawing::Imaging::BitmapData ^bitmapData = bitmap->LockBits(
				System::Drawing::Rectangle(0, 0, bitmap->Width, bitmap->Height),
				System::Drawing::Imaging::ImageLockMode::ReadWrite,
				bitmap->PixelFormat
				);

			char *src = reinterpret_cast<char*>(bitmapData->Scan0.ToPointer());
			pin_ptr<char> pin(&src[0]);

			::memcpy(pin, data, totalSize);

			bitmap->UnlockBits(bitmapData);

			return bitmap;
		}

		static MemoryStream^ BitmapToMemoryStream(Bitmap^ bitmap, ImageFormat^ imageFormat)
		{
			MemoryStream^ ms = gcnew System::IO::MemoryStream();
			bitmap->Save(ms, imageFormat);
			return ms;
		}

		static std::vector<char> MemoryStreamToVector(MemoryStream^ ms)
		{
			unsigned char* byteArray = ToCharPtr(ms->ToArray());
			std::vector<char> result(byteArray, byteArray + ms->Length);
			return result;
		}

		static std::vector<AlprRegionOfInterest> ToVector(List<System::Drawing::Rectangle>^ src)
		{
			std::vector<AlprRegionOfInterest> result;

			for each(System::Drawing::Rectangle^ rect in src)
			{
				AlprRegionOfInterest roi(rect->X, rect->Y, rect->Width, rect->Height);
				result.push_back(roi);
			}

			return result;
		}

		static unsigned char* ToCharPtr(array<unsigned char>^ src)
		{
			//unsigned char* result = (unsigned char*) new unsigned char[src->Length];
			pin_ptr<unsigned char> pin(&src[0]);
			unsigned char* pc = pin;
			return pc;
		}

		static System::String^ ToManagedString(std::string s)
		{
			return gcnew String(s.c_str());
		}

		static std::string ToStlString(System::String^ s)
		{
			IntPtr ptr = Marshal::StringToHGlobalAnsi(s);
			if (ptr != IntPtr::Zero)
			{
				std::string tmp(reinterpret_cast<char*>(static_cast<void*>(ptr)));
				Marshal::FreeHGlobal(ptr);
				return tmp;
			}
			return std::string();
		}

		static System::Drawing::Rectangle ToRectangle(cv::Rect rect)
		{
			return System::Drawing::Rectangle(rect.x, rect.y, rect.width, rect.height);
		}

		static List<System::Drawing::Rectangle>^ ToRectangleList(std::vector<cv::Rect> srcRects)
		{
			List<System::Drawing::Rectangle>^ rects = gcnew List<System::Drawing::Rectangle>();
			for each(cv::Rect rect in srcRects)
			{
				rects->Add(ToRectangle(rect));
			}
			return rects;
		}

	};
};
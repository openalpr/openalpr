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
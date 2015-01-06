/*
* Copyright (c) 2015 Dr. Masroor Ehsan
*
* This file is part of OpenAlpr.Net.
*
* OpenAlpr.Net is free software: you can redistribute it and/or modify
* it under the terms of the GNU Affero General Public License
* version 3 as published by the Free Software Foundation
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Affero General Public License for more details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "openalpr-net.h"
#include "alpr.h"
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#using <mscorlib.dll>
//#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace msclr::interop;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace System::Drawing;
using namespace alpr;

namespace openalprnet {

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


//		static std::vector<AlprRegionOfInterest> ToVector(List<System::Drawing::Rectangle>^ src)
//		{
//			std::vector<AlprRegionOfInterest> result;
//
//			for each(System::Drawing::Rectangle^ rect in src)
//			{
//				AlprRegionOfInterest roi;
//				roi.x = rect->X;
//				roi.y = rect->Y;
//				roi.height = rect->Height;
//				roi.width = rect->Width;
//				result.push_back(roi);
//			}
//
//			return result;
//		}

		static System::String^ ToManagedString(std::string s)
		{
			return gcnew String(s.c_str());
		}

		static std::string ToStlString(System::String^ s)
		{
			IntPtr ptr = Marshal::StringToHGlobalAnsi(s);
			if(ptr != IntPtr::Zero)
			{
				std::string tmp(reinterpret_cast<char*>(static_cast<void*>(ptr)));
				Marshal::FreeHGlobal(ptr);
				return tmp;
			}
			return std::string();
		}
	};

	public ref class AlprPlateNet sealed
	{
	public:
		AlprPlateNet(AlprPlate plate){
			//_characters = marshal_as<System::String^>(plate.characters);
			m_characters = AlprHelper::ToManagedString(plate.characters);
			m_overall_confidence=plate.overall_confidence;
			m_matches_template=plate.matches_template;
		}

		property System::String^ characters {
			System::String^ get() {
				return m_characters;
			}
		}

		property float overall_confidence {
			float get() {
				return m_overall_confidence;
			}
		}

		property bool matches_template {
			bool get() {
				return m_matches_template;
			}
		}

	private:
		System::String^ m_characters;
		float m_overall_confidence;
		bool m_matches_template;
	};

	public ref class AlprPlateResultNet sealed
	{
	public:
		AlprPlateResultNet() : m_Impl( new AlprPlateResult ) {}

		AlprPlateResultNet(AlprPlateResult* result) : m_Impl( result ) {}

		property int requested_topn {
			int get() {
				return m_Impl->requested_topn;
			}
		}

		property int regionConfidence {
			int get() {
				return m_Impl->regionConfidence;
			}
		}

		property System::String^ region {
			System::String^ get() {
				return AlprHelper::ToManagedString(m_Impl->region);
			}
		}


		property AlprPlateNet^ bestPlate {
			AlprPlateNet^ get() {
				AlprPlateNet^ result = gcnew AlprPlateNet(m_Impl->bestPlate);
				return result;
			}
		}

		property List<System::Drawing::Point>^ plate_points {
			List<System::Drawing::Point>^ get() {
				List<System::Drawing::Point>^ list = gcnew List<System::Drawing::Point>(4);
				for (int i = 0; i < 4; i++)
				{
					list->Add(System::Drawing::Point(m_Impl->plate_points[i].x, m_Impl->plate_points[i].y));
				}
				return list;
			}
		} 

		property List<AlprPlateNet^>^ topNPlates {
			List<AlprPlateNet^>^ get() {
				List<AlprPlateNet^>^ list = gcnew List<AlprPlateNet^>(m_Impl->topNPlates.size());
				for (std::vector<AlprPlate>::iterator itr = m_Impl->topNPlates.begin(); itr != m_Impl->topNPlates.end(); itr++)
				{
					list->Add(gcnew AlprPlateNet(*itr));
				}
				return list;
			}
		}

		property float processing_time_ms {
			float get() {
				return m_Impl->processing_time_ms;
			}
		}

	private:
		AlprPlateResult * m_Impl;
	};


	public ref class AlprResultsNet sealed
	{
	public:
		AlprResultsNet() : m_Impl( new AlprResults ) {}

		AlprResultsNet(AlprResults* results) : m_Impl( results ) {}

		property int img_width {
			int get() {
				return m_Impl->img_width;
			}
		}

		property int img_height {
			int get() {
				return m_Impl->img_height;
			}
		}
		
		property float total_processing_time_ms {
			float get() {
				return m_Impl->total_processing_time_ms;
			}
		}

		property List<System::Drawing::Rectangle>^ regionsOfInterest {
			List<System::Drawing::Rectangle>^ get() {
				List<System::Drawing::Rectangle>^ list = gcnew List<System::Drawing::Rectangle>(m_Impl->regionsOfInterest.size());
				for (unsigned int i = 0; i < m_Impl->regionsOfInterest.size(); i++)
				{
					list->Add(System::Drawing::Rectangle(m_Impl->regionsOfInterest[i].x, m_Impl->regionsOfInterest[i].y, m_Impl->regionsOfInterest[i].width, m_Impl->regionsOfInterest[i].height));
				}
				return list;
			}
		} 

		property List<AlprPlateResultNet^>^ plates {
			List<AlprPlateResultNet^>^ get() {
				List<AlprPlateResultNet^>^ list = gcnew List<AlprPlateResultNet^>(m_Impl->plates.size());
				for (std::vector<AlprPlateResult>::iterator itr = m_Impl->plates.begin(); itr != m_Impl->plates.end(); itr++)
				{
					list->Add(gcnew AlprPlateResultNet(&*itr));
				}
				return list;
			}
		}



	private:
		AlprResults * m_Impl;
	};


	public ref class AlprNet sealed
	{
	public:
		// Allocate the native object on the C++ Heap via a constructor
		AlprNet(System::String^ country, System::String^ configFile) : m_Impl( new Alpr(marshal_as<std::string>(country), marshal_as<std::string>(configFile)) ) { }

		// Deallocate the native object on a destructor
		~AlprNet(){
			delete m_Impl;
		}

		property int TopN {
			int get() {
				return m_topN;
			}
			void set( int topn ){
				m_topN = topn;
				m_Impl->setTopN(topn);
			}
		}

		property bool DetectRegion {
			bool get() {
				return m_detectRegion;
			}
			void set( bool detectRegion ) {
				m_detectRegion = detectRegion;
				m_Impl->setDetectRegion(detectRegion);
			}
		}

		property System::String^ DefaultRegion {
			System::String^ get() {
				return m_defaultRegion;
			}
			void set( System::String^ region ){
				m_defaultRegion = region;
				m_Impl->setDefaultRegion(marshal_as<std::string>(region));
			}
		}

		AlprResultsNet^ recognize(System::String^ filepath) {
			AlprResults results = m_Impl->recognize(marshal_as<std::string>(filepath));
			return gcnew AlprResultsNet(&results);
		}

		AlprResultsNet^ recognize(cli::array<char>^ imageBuffer) {
			std::vector<char> p = AlprHelper::ToVector(imageBuffer);
			AlprResults results = m_Impl->recognize(p);
			return gcnew AlprResultsNet(&results);
		}


		bool isLoaded() {
			return m_Impl->isLoaded();
		}

		static System::String^ getVersion() {
			return AlprHelper::ToManagedString(Alpr::getVersion());
		}

//		System::String^ toJson(AlprResultsNet^ results) {
//			std::string json = Alpr::toJson(marshal_as<AlprResults>(results));
//			return AlprHelper::ToManagedString(json);
//		}

	protected:
		// Deallocate the native object on the finalizer just in case no destructor is called
		!AlprNet() {
			delete m_Impl;
		}

	private:
		Alpr * m_Impl;
		int m_topN;
		bool m_detectRegion;
		System::String^ m_defaultRegion;

	};
}
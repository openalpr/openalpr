/*
* Copyright (c) 2015 OpenALPR Technology, Inc.
*
* This file is part of OpenALPR.
*
* OpenALPR is free software: you can redistribute it and/or modify
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

using namespace System;
using namespace alpr;

namespace openalprnet {

	public ref class AlprPlateNet sealed
	{
	public:
		AlprPlateNet(AlprPlate plate){
			m_characters = AlprHelper::ToManagedString(plate.characters);
			m_overall_confidence=plate.overall_confidence;
			m_matches_template=plate.matches_template;
		}

		property System::String^ Characters {
			System::String^ get() {
				return m_characters;
			}
		}

		property float OverallConfidence {
			float get() {
				return m_overall_confidence;
			}
		}

		property bool MatchesTemplate {
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
		AlprPlateResultNet(AlprPlateResult result) {
			m_plate_index = result.plate_index;
			m_processing_time_ms = result.processing_time_ms;
			m_regionConfidence = result.regionConfidence;
			m_region = AlprHelper::ToManagedString(result.region);
			m_bestPlate = gcnew AlprPlateNet(result.bestPlate);

			m_plate_points = gcnew List<System::Drawing::Point>(4);
			for (int i = 0; i < 4; i++)
			{
				m_plate_points->Add(System::Drawing::Point(result.plate_points[i].x, result.plate_points[i].y));
			}

			int num = result.topNPlates.size();
			m_topNPlates = gcnew List<AlprPlateNet^>(num);
			for (int i = 0; i < num; i++)
			{
				m_topNPlates->Add(gcnew AlprPlateNet(result.topNPlates[i]));
			}
		}

		property int RequestedTopN {
			int get() {
				return m_requested_topn;
			}
		}

		property int RegionConfidence {
			int get() {
				return m_regionConfidence;
			}
		}

		property int PlateIndex {
			int get() {
				return m_plate_index;
			}
		}

		property System::String^ Region {
			System::String^ get() {
				return m_region;
			}
		}

		property AlprPlateNet^ BestPlate {
			AlprPlateNet^ get() {
				return m_bestPlate;
			}
		}

		property List<System::Drawing::Point>^ PlatePoints {
			List<System::Drawing::Point>^ get() {
				return m_plate_points;
			}
		} 

		property List<AlprPlateNet^>^ TopNPlates {
			List<AlprPlateNet^>^ get() {
				return m_topNPlates;
			}
		}

		property float ProcessingTimeMs {
			float get() {
				return m_processing_time_ms;
			}
		}

	private:
		int m_requested_topn;
		int m_regionConfidence;
		int m_plate_index;
		System::String^ m_region;
		float m_processing_time_ms;
		List<AlprPlateNet^>^ m_topNPlates;
		List<System::Drawing::Point>^ m_plate_points;
		AlprPlateNet^ m_bestPlate;
	};


	public ref class AlprResultsNet sealed
	{
	public:
		AlprResultsNet(AlprResults results) {
			m_epoch_time = results.epoch_time;
			m_img_width = results.img_width;
			m_img_height = results.img_height;
			m_total_processing_time_ms = results.total_processing_time_ms;

			int num_rois = results.regionsOfInterest.size();
			m_regionsOfInterest = gcnew List<System::Drawing::Rectangle>(num_rois);
			for (int i = 0; i < num_rois; i++)
			{
				m_regionsOfInterest->Add(System::Drawing::Rectangle(
					results.regionsOfInterest[i].x, 
					results.regionsOfInterest[i].y, 
					results.regionsOfInterest[i].width, 
					results.regionsOfInterest[i].height));
			}

			int num_plates = results.plates.size();
			m_plates = gcnew List<AlprPlateResultNet^>(num_plates);
			for (int i = 0; i < num_plates; i++)
			{
				m_plates->Add(gcnew AlprPlateResultNet(results.plates[i]));					
			}

			std::string json = Alpr::toJson(results);
			m_json = AlprHelper::ToManagedString(json);
		}

		property long EpochTime {
			long get() {
				return m_epoch_time;
			}
		}

		property int ImageWidth {
			int get() {
				return m_img_width;
			}
		}

		property int ImageHeight {
			int get() {
				return m_img_height;
			}
		}

		property float TotalProcessingTimeMs {
			float get() {
				return m_total_processing_time_ms;
			}
		}

		property List<System::Drawing::Rectangle>^ RegionsOfInterest {
			List<System::Drawing::Rectangle>^ get() {
				return m_regionsOfInterest;
			}
		}

		property List<AlprPlateResultNet^>^ Plates {
			List<AlprPlateResultNet^>^ get() {
				return m_plates;
			}
		}

		property System::String^ Json {
			System::String^ get() {
				return m_json;
			}
		}

	private:
		long m_epoch_time;
		int m_img_width;
		int m_img_height;
		float m_total_processing_time_ms;
		List<System::Drawing::Rectangle>^ m_regionsOfInterest;
		List<AlprPlateResultNet^>^ m_plates;
		System::String^ m_json;
	};


	public ref class AlprFrameEventArgs : public EventArgs
	{
	public:
		AlprFrameEventArgs(int frameNumber, System::Drawing::Image^ frame, AlprResultsNet^ results) {
			m_frameNumber = frameNumber;
			m_frame = frame;
			m_results = results;
			m_cancel = false;
		}

		property int FrameNumber {
			int get() {
				return m_frameNumber;
			}
		}

		property System::Drawing::Image^ Frame {
			System::Drawing::Image^ get() {
				return m_frame;
			}
		}

		property AlprResultsNet^ Results {
			AlprResultsNet^ get() {
				return m_results;
			}
		}

		property bool Cancel {
			bool get() {
				return m_cancel;
			}
			void set( bool cancel ) {
				m_cancel = cancel;
			}
		}
	private:
		int m_frameNumber;
		System::Drawing::Image^ m_frame;
		AlprResultsNet^ m_results;
		bool m_cancel;
	};

	public ref class AlprNet sealed : IDisposable
	{
	public:
		// Allocate the native object on the C++ Heap via a constructor
		AlprNet(System::String^ country, System::String^ configFile, System::String^ runtimeDir) : m_Impl( new Alpr(marshal_as<std::string>(country), marshal_as<std::string>(configFile), marshal_as<std::string>(runtimeDir)) )
		{
			this->m_config = gcnew AlprConfigNet(this->m_Impl->getConfig());
		}

		~AlprNet() {
			if(this->m_disposed)
			{
				return;
			}

			this->!AlprNet();
			this->m_disposed = true;
		}
	
		property AlprConfigNet^ Configuration {
			AlprConfigNet^ get()
			{
				return this->m_config;
			}
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

		event EventHandler<AlprFrameEventArgs^>^ FrameProcessed;

		void RecognizeFromVideo(System::String^ videoPath) {
			if (System::IO::File::Exists(videoPath)) {
				int framenum = 0;
				cv::VideoCapture cap = cv::VideoCapture();
				cap.open(marshal_as<std::string>(videoPath));

				cv::Mat frame;

				while (cap.read(frame))
				{
					std::vector<AlprRegionOfInterest> regionsOfInterest;
					regionsOfInterest.push_back(AlprRegionOfInterest(0, 0, frame.cols, frame.rows));

					AlprResults results = m_Impl->recognize(frame.data, frame.elemSize(), frame.cols, frame.rows, regionsOfInterest);
					int framecolsorig = frame.cols;
					if (framecolsorig % 4 != 0) copyMakeBorder(frame, frame, 0, 0, 0, 4 - (framecolsorig % 4), IPL_BORDER_REPLICATE);			//Stride has to be multiple of 4
					Image^ frameImage = gcnew Bitmap(framecolsorig, frame.rows, frame.step, Imaging::PixelFormat::Format24bppRgb, IntPtr(frame.data));
					AlprFrameEventArgs^ alprFrameEventArgs = gcnew AlprFrameEventArgs(framenum, frameImage, gcnew AlprResultsNet(results));
					FrameProcessed(this, alprFrameEventArgs);
					delete frameImage;
					if (alprFrameEventArgs->Cancel) {
						break;
					}
					framenum++;
				}
			}
			else {
				throw gcnew System::IO::FileNotFoundException("No video was not found at " + videoPath, videoPath);
			}
		}

		/// <summary>
		/// Recognize from an image on disk
		/// </summary>
		AlprResultsNet^ Recognize(System::String^ filepath) {
			return Recognize(filepath, gcnew List<System::Drawing::Rectangle>());
		}

		/// <summary>
		/// Recognize from an image on disk
		/// </summary>
		AlprResultsNet^ Recognize(System::String^ filepath, List<System::Drawing::Rectangle>^ regionsOfInterest) {
			array<Byte>^ byteArray = File::ReadAllBytes(filepath);
			return Recognize(byteArray, regionsOfInterest);
		}

		/// <summary>
		/// Recognize from a bitmap
		/// </summary>
		AlprResultsNet^ Recognize(Bitmap^ bitmap)
		{
			return Recognize(bitmap, gcnew List<System::Drawing::Rectangle>());
		}

		/// <summary>
		/// Recognize from MemoryStream representing an encoded image (e.g., BMP, PNG, JPG, GIF etc).
		/// </summary>
		AlprResultsNet^ Recognize(MemoryStream^ memoryStream)
		{
			return Recognize(memoryStream, gcnew List<System::Drawing::Rectangle>());
		}

		/// <summary>
		/// Recognize from MemoryStream representing an encoded image (e.g., BMP, PNG, JPG, GIF etc).
		/// </summary>
		AlprResultsNet^ Recognize(MemoryStream^ memoryStream, List<System::Drawing::Rectangle>^ regionsOfInterest)
		{
			return Recognize(memoryStream->ToArray(), regionsOfInterest);
		}

		/// <summary>
		/// Recognize from byte data representing an encoded image (e.g., BMP, PNG, JPG, GIF etc).
		/// </summary>
		AlprResultsNet^ Recognize(array<Byte>^ imageBuffer) {
			return Recognize(imageBuffer, gcnew List<System::Drawing::Rectangle>());
		}

		/// <summary>
		/// Recognize from a bitmap
		/// </summary>
		AlprResultsNet^ Recognize(Bitmap^ bitmap, List<System::Drawing::Rectangle>^ regionsOfInterest)
		{
			BitmapMat^ wrapper = gcnew BitmapMat(bitmap);
			AlprResultsNet^ results = Recognize(wrapper, regionsOfInterest);
			delete wrapper;
			return results;
		}

		/// <summary>
		/// Recognize from byte data representing an encoded image (e.g., BMP, PNG, JPG, GIF etc).
		/// </summary>
		AlprResultsNet^ Recognize(array<Byte>^ imageBuffer, List<System::Drawing::Rectangle>^ regionsOfInterest) {
			BitmapMat^ wrapper = gcnew BitmapMat(imageBuffer);
			AlprResultsNet^ results = Recognize(wrapper, regionsOfInterest);
			delete wrapper;
			return results;
		}

		/// <summary>
		/// Pre-warp from raw pixel data. 
		/// </summary>
		array<Byte>^ PreWarp(array<Byte>^ imageBuffer)
		{
			std::vector<char> buffer = AlprHelper::ToVector(imageBuffer);
			cv::Mat src = cv::imdecode(buffer, CV_LOAD_IMAGE_COLOR);

			alpr::PreWarp *preWarp = new alpr::PreWarp(m_Impl->getConfig());
			cv::Mat warpedImage = preWarp->warpImage(src);

			std::vector<uchar> warpedImageVector;
			cv::imencode(".jpg", warpedImage, warpedImageVector);

			const size_t warpedImageSize = warpedImageVector.size();

			array<Byte>^ warpedImageByteArray = gcnew array<Byte>(warpedImageSize);
			pin_ptr<Byte> pin(&warpedImageByteArray[0]);

			std::memcpy(pin, &warpedImageVector[0], warpedImageSize);

			delete preWarp;

			return warpedImageByteArray;
		}

		bool IsLoaded() {
			return m_Impl->isLoaded();
		}

		static System::String^ GetVersion() {
			return AlprHelper::ToManagedString(Alpr::getVersion());
		}

	protected:

		AlprResultsNet^ Recognize(BitmapMat^ bitmapMat, List<System::Drawing::Rectangle>^ regionsOfInterest) {
			cv::Mat frame = bitmapMat->Value;
			std::vector<AlprRegionOfInterest> rois = AlprHelper::ToVector(regionsOfInterest);
			AlprResults results = m_Impl->recognize(frame.data, frame.elemSize(), frame.cols, frame.rows, rois);			
			return gcnew AlprResultsNet(results);
		}

		// Deallocate the native object on the finalizer just in case no destructor is called
		!AlprNet() {
			delete m_Impl;
			delete m_config;
		}
	
	private:
		Alpr * m_Impl;
		AlprConfigNet^ m_config;
		int m_topN;
		bool m_detectRegion;
		System::String^ m_defaultRegion;
		bool m_disposed;
	};
}

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
#include "alpr.h"
#include "config.h" // alpr
#include "motiondetector.h" // alpr
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#using <mscorlib.dll>
//#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace System;
using namespace msclr::interop;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace System::Drawing;
using namespace System::Drawing::Imaging;
using namespace System::IO;
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

		static cv::Mat BitmapToMat(Bitmap^ bitmap)
		{
			int channels = 0;

			switch(bitmap->PixelFormat)
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

			cv::Mat dstMat(cv::Size(bitmap->Width, bitmap->Height), CV_8UC(channels), reinterpret_cast<char*>(bitmapData->Scan0.ToPointer()));

			bitmap->UnlockBits(bitmapData);

			return dstMat;
		}

		static Bitmap^ MatToBitmap(cv::Mat mat)
		{
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

			::memcpy(bitmapData->Scan0.ToPointer(), data, totalSize);

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
			if(ptr != IntPtr::Zero)
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

	};

	public enum class OpenCVMatType {
		Unchanged = CV_LOAD_IMAGE_UNCHANGED,
		Grayscale = CV_LOAD_IMAGE_GRAYSCALE,
		Color = CV_LOAD_IMAGE_COLOR,
		AnyDepth = CV_LOAD_IMAGE_ANYDEPTH,
		AnyColor = CV_LOAD_IMAGE_ANYCOLOR
	};

	public ref class AlprMotionDetectionNet : IDisposable {
	public:
		AlprMotionDetectionNet()
		{
			m_motionDetector = new MotionDetector();
		}

		void ResetMotionDetection(Bitmap^ bitmap)
		{
			ResetMotionDetection(Mat(bitmap));
		}

		void ResetMotionDetection(String^ filename, OpenCVMatType matType)
		{
			return ResetMotionDetection(Mat(filename, matType));
		}

		System::Drawing::Rectangle MotionDetect(Bitmap^ bitmap)
		{
			return MotionDetect(Mat(bitmap));
		}

		System::Drawing::Rectangle MotionDetect(String^ filename, OpenCVMatType matType)
		{
			return MotionDetect(Mat(filename, matType));
		}

	private:
		void ResetMotionDetection(cv::Mat mat)
		{
			this->m_motionDetector->ResetMotionDetection(&mat);
		}

		System::Drawing::Rectangle MotionDetect(cv::Mat mat)
		{
			cv::Rect rect = this->m_motionDetector->MotionDetect(&mat);
			return AlprHelper::ToRectangle(rect);
		}

		cv::Mat Mat(String^ filename, OpenCVMatType matType)
		{
			cv::Mat mat = cv::imread(AlprHelper::ToStlString(filename), static_cast<int>(matType));
			return mat;
		}

		cv::Mat Mat(Bitmap^ bitmap)
		{
			cv::Mat mat = AlprHelper::BitmapToMat(bitmap);
			return mat;
		}

	private:

		~AlprMotionDetectionNet()
		{
			if(this->m_Disposed)
			{
				return;
			}

			this->!AlprMotionDetectionNet();
			this->m_Disposed = true;
		}

		!AlprMotionDetectionNet()
		{
			delete m_motionDetector;
		}

	private:
		MotionDetector* m_motionDetector;
		bool m_Disposed;
	};


	public enum class AlprDetectorTypeNet : int {
		DetectorLbpCpu = alpr::DETECTOR_LBP_CPU,
		DetectorLbpGpu = alpr::DETECTOR_LBP_GPU,
		DetectorLbpMorphCpu = alpr::DETECTOR_MORPH_CPU
	};

	public ref class AlprConfigNet sealed
	{
	public:

		AlprConfigNet(Config* config) : m_config (config)
		{
			
		}

		property bool IsLoaded {
			bool get()
			{
				return this->m_config->loaded;
			}
		}

		property AlprDetectorTypeNet Detector {
			AlprDetectorTypeNet get() {
				return static_cast<AlprDetectorTypeNet>(this->m_config->detector);
			}
			void set(AlprDetectorTypeNet value)
			{
				this->m_config->detector = static_cast<int>(value);
			}
		}

		property float DetectionIterationIncrease {
			float get()
			{
				return this->m_config->detection_iteration_increase;
			}
			void set(float value)
			{
				this->m_config->detection_iteration_increase = value;
			}
		}

		property float DetectionStrictness {
			float get()
			{
				return this->m_config->detectionStrictness;
			}
			void set(float value)
			{
				this->m_config->detectionStrictness = value;
			}
		}

		property float MaxPlateWidthPercent {
			float get()
			{
				return this->m_config->maxPlateWidthPercent;
			}
			void set(float value)
			{
				this->m_config->maxPlateWidthPercent = value;
			}
		}

		property float MaxPlateHeightPercent {
			float get()
			{
				return this->m_config->maxPlateHeightPercent;
			}
			void set(float value)
			{
				this->m_config->maxPlateHeightPercent = value;
			}
		}

		property int MaxDetectionInputWidth {
			int get()
			{
				return this->m_config->maxDetectionInputWidth;
			}
			void set(int value)
			{
				this->m_config->maxDetectionInputWidth = value;
			}
		}

		property int MaxDetectionInputHeight {
			int get()
			{
				return this->m_config->maxDetectionInputHeight;
			}
			void set(int value)
			{
				this->m_config->maxDetectionInputHeight = value;
			}
		}

		property bool SkipDetection {
			bool get()
			{
				return this->m_config->skipDetection;
			}
			void set(bool value)
			{
				this->m_config->skipDetection = true;
			}
		}

		property String^ PreWarp {
			String^ get()
			{
				return AlprHelper::ToManagedString(this->m_config->prewarp);
			}
			void set(String^ value)
			{
				this->m_config->prewarp = marshal_as<std::string>(value);
			}
		}

		property int MaxPlateAngleDegrees {
			int get()
			{
				return this->m_config->maxPlateAngleDegrees;
			}
			void set(int value)
			{
				this->m_config->maxPlateAngleDegrees = value;
			}
		}

		property float MinPlateSizeWidthPx {
			float get()
			{
				return this->m_config->minPlateSizeWidthPx;
			}
			void set(float value)
			{
				this->m_config->minPlateSizeWidthPx = value;
			}
		}

		property float minPlateSizeHeightPx {
			float get()
			{
				return this->m_config->minPlateSizeHeightPx;
			}
			void set(float value)
			{
				this->m_config->minPlateSizeHeightPx = value;
			}
		}

		property bool Multiline {
			bool get()
			{
				return this->m_config->multiline;
			}
			void set(bool value)
			{
				this->m_config->multiline = value;
			}
		}

		property float PlateWidthMM {
			float get()
			{
				return this->m_config->plateWidthMM;
			}
			void set(float value)
			{
				this->m_config->plateWidthMM = value;
			}
		}

		property float PlateHeightMM {
			float get()
			{
				return this->m_config->plateHeightMM;
			}
			void set(float value)
			{
				this->m_config->plateHeightMM = value;
			}
		}

		property float CharHeightMM {
			float get()
			{
				return this->m_config->charHeightMM;
			}
			void set(float value)
			{
				this->m_config->charHeightMM = value;
			}
		}

		property float CharWidthMM {
			float get()
			{
				return this->m_config->charWidthMM;
			}
			void set(float value)
			{
				this->m_config->charWidthMM = value;
			}
		}

		property float CharWhitespaceTopMM {
			float get()
			{
				return this->m_config->charWhitespaceTopMM;
			}
			void set(float value)
			{
				this->m_config->charWhitespaceTopMM = value;
			}
		}

		property float CharWhitespaceBotMM {
			float get()
			{
				return this->m_config->charWhitespaceBotMM;
			}
			void set(float value)
			{
				this->m_config->charWhitespaceBotMM = value;
			}
		}

		property int TemplateWidthPx {
			int get()
			{
				return this->m_config->templateWidthPx;
			}
			void set(int value)
			{
				this->m_config->templateWidthPx = value;
			}
		}

		property int TemplateHeightPx {
			int get()
			{
				return this->m_config->templateHeightPx;
			}
			void set(int value)
			{
				this->m_config->templateHeightPx = value;
			}
		}

		property int OcrImageWidthPx {
			int get()
			{
				return this->m_config->ocrImageWidthPx;
			}
			void set(int value)
			{
				this->m_config->ocrImageWidthPx = value;
			}
		}

		property int OcrImageHeightPx {
			int get()
			{
				return this->m_config->ocrImageHeightPx;
			}
			void set(int value)
			{
				this->m_config->ocrImageHeightPx = value;
			}
		}

		property int StateIdImageWidthPx {
			int get()
			{
				return this->m_config->stateIdImageWidthPx;
			}
			void set(int value)
			{
				this->m_config->stateIdImageWidthPx = value;
			}
		}

		property int StateIdimageHeightPx {
			int get()
			{
				return this->m_config->stateIdimageHeightPx;
			}
			void set(int value)
			{
				this->m_config->stateIdimageHeightPx = value;
			}
		}

		property float CharAnalysisMinPercent {
			float get()
			{
				return this->m_config->charAnalysisMinPercent;
			}
			void set(float value)
			{
				this->m_config->charAnalysisMinPercent = value;
			}
		}

		property float CharAnalysisHeightRange {
			float get()
			{
				return this->m_config->charAnalysisHeightRange;
			}
			void set(float value)
			{
				this->m_config->charAnalysisHeightRange = value;
			}
		}

		property float CharAnalysisHeightStepSize {
			float get()
			{
				return this->m_config->charAnalysisHeightStepSize;
			}
			void set(float value)
			{
				this->m_config->charAnalysisHeightStepSize = value;
			}
		}

		property int CharAnalysisNumSteps {
			int get()
			{
				return this->m_config->charAnalysisNumSteps;
			}
			void set(int value)
			{
				this->m_config->charAnalysisNumSteps = value;
			}
		}

		property float PlateLinesSensitivityVertical {
			float get()
			{
				return this->m_config->plateLinesSensitivityVertical;
			}
			void set(float value)
			{
				this->m_config->plateLinesSensitivityVertical = value;
			}
		}

		property float PlateLinesSensitivityHorizontal {
			float get()
			{
				return this->m_config->plateLinesSensitivityHorizontal;
			}
			void set(float value)
			{
				this->m_config->plateLinesSensitivityHorizontal = value;
			}
		}

		property int SegmentationMinBoxWidthPx {
			int get()
			{
				return this->m_config->segmentationMinBoxWidthPx;
			}
			void set(int value)
			{
				this->m_config->segmentationMinBoxWidthPx = value;
			}
		}

		property float SegmentationMinCharHeightPercent {
			float get()
			{
				return this->m_config->segmentationMinCharHeightPercent;
			}
			void set(float value)
			{
				this->m_config->segmentationMinCharHeightPercent = value;
			}
		}

		property float SegmentationMaxCharWidthvsAverage {
			float get()
			{
				return this->m_config->segmentationMaxCharWidthvsAverage;
			}
			void set(float value)
			{
				this->m_config->segmentationMaxCharWidthvsAverage = value;
			}
		}

		property String^ OcrLanguage {
			String^ get()
			{
				return AlprHelper::ToManagedString(this->m_config->ocrLanguage);
			}
			void set(String^ value)
			{
				this->m_config->ocrLanguage = marshal_as<std::string>(value);
			}
		}

		property int OcrMinFontSize {
			int get()
			{
				return this->m_config->ocrMinFontSize;
			}
			void set(int value)
			{
				this->m_config->ocrMinFontSize = value;
			}
		}

		property float PostProcessMinConfidence {
			float get()
			{
				return this->m_config->postProcessMinConfidence;
			}
			void set(float value)
			{
				this->m_config->postProcessMinConfidence = value;
			}
		}

		property float PostProcessConfidenceSkipLevel {
			float get()
			{
				return this->m_config->postProcessConfidenceSkipLevel;
			}
			void set(float value)
			{
				this->m_config->postProcessConfidenceSkipLevel = value;
			}
		}

		property unsigned int PostProcessMinCharacters {
			unsigned int get()
			{
				return this->m_config->postProcessMinCharacters;
			}
			void set(unsigned int value)
			{
				this->m_config->postProcessMinCharacters = value;
			}
		}

		property unsigned int PostProcessMaxCharacters {
			unsigned int get()
			{
				return this->m_config->postProcessMaxCharacters;
			}
			void set(unsigned int value)
			{
				this->m_config->postProcessMaxCharacters = value;
			}
		}

		property bool DebugGeneral {
			bool get()
			{
				return this->m_config->debugGeneral;
			}
			void set(bool value)
			{
				this->m_config->debugGeneral = value;
			}
		}

		property bool DebugTiming {
			bool get()
			{
				return this->m_config->debugTiming;
			}
			void set(bool value)
			{
				this->m_config->debugTiming = value;
			}
		}

		property bool DebugPrewarp {
			bool get()
			{
				return this->m_config->debugPrewarp;
			}
			void set(bool value)
			{
				this->m_config->debugPrewarp = value;
			}
		}

		property bool DebugDetector {
			bool get()
			{
				return this->m_config->debugDetector;
			}
			void set(bool value)
			{
				this->m_config->debugDetector = value;
			}
		}

		property bool DebugStateId {
			bool get()
			{
				return this->m_config->debugStateId;
			}
			void set(bool value)
			{
				this->m_config->debugStateId = value;
			}
		}

		property bool DebugPlateLines {
			bool get()
			{
				return this->m_config->debugPlateLines;
			}
			void set(bool value)
			{
				this->m_config->debugPlateLines = value;
			}
		}

		property bool DebugPlateCorners {
			bool get()
			{
				return this->m_config->debugPlateCorners;
			}
			void set(bool value)
			{
				this->m_config->debugPlateCorners = value;
			}
		}

		property bool DebugCharSegmenter {
			bool get()
			{
				return this->m_config->debugCharSegmenter;
			}
			void set(bool value)
			{
				this->m_config->debugCharSegmenter = value;
			}
		}

		property bool DebugCharAnalysis {
			bool get()
			{
				return this->m_config->debugCharAnalysis;
			}
			void set(bool value)
			{
				this->m_config->debugCharAnalysis = value;
			}
		}

		property bool DebugColorFiler {
			bool get()
			{
				return this->m_config->debugColorFiler;
			}
			void set(bool value)
			{
				this->m_config->debugColorFiler = value;
			}
		}

		property bool DebugOcr {
			bool get()
			{
				return this->m_config->debugOcr;
			}
			void set(bool value)
			{
				this->m_config->debugOcr = value;
			}
		}

		property bool DebugPostProcess {
			bool get()
			{
				return this->m_config->debugPostProcess;
			}
			void set(bool value)
			{
				this->m_config->debugPostProcess = value;
			}
		}

		property bool DebugShowImages {
			bool get()
			{
				return this->m_config->debugShowImages;
			}
			void set(bool value)
			{
				this->m_config->debugShowImages = value;
			}
		}

		property bool DebugPauseOnFrame {
			bool get()
			{
				return this->m_config->debugPauseOnFrame;
			}
			void set(bool value)
			{
				this->m_config->debugPauseOnFrame = value;
			}
		}

		void DebugOff(bool value)
		{
			this->m_config->debugOff(value);
		}

		String^ GetKeypointsRuntimeDir()
		{
			return AlprHelper::ToManagedString(this->m_config->getKeypointsRuntimeDir());
		}

		String^ GetCascadeRuntimeDir()
		{
			return AlprHelper::ToManagedString(this->m_config->getCascadeRuntimeDir());
		}

		String^ GetPostProcessRuntimeDir()
		{
			return AlprHelper::ToManagedString(this->m_config->getPostProcessRuntimeDir());
		}

		String^ GetTessdataPrefix()
		{
			return AlprHelper::ToManagedString(this->m_config->getTessdataPrefix());
		}

		~AlprConfigNet()
		{
			// void
		}

	private:
		Config *m_config;
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

		property int requested_topn {
			int get() {
				return m_requested_topn;
			}
		}

		property int regionConfidence {
			int get() {
				return m_regionConfidence;
			}
		}

		property int plate_index {
			int get() {
				return m_plate_index;
			}
		}

		property System::String^ region {
			System::String^ get() {
				return m_region;
			}
		}

		property AlprPlateNet^ bestPlate {
			AlprPlateNet^ get() {
				return m_bestPlate;
			}
		}

		property List<System::Drawing::Point>^ plate_points {
			List<System::Drawing::Point>^ get() {
				return m_plate_points;
			}
		} 

		property List<AlprPlateNet^>^ topNPlates {
			List<AlprPlateNet^>^ get() {
				return m_topNPlates;
			}
		}

		property float processing_time_ms {
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

		property long epoch_time {
			long get() {
				return m_epoch_time;
			}
		}

		property int img_width {
			int get() {
				return m_img_width;
			}
		}

		property int img_height {
			int get() {
				return m_img_height;
			}
		}

		property float total_processing_time_ms {
			float get() {
				return m_total_processing_time_ms;
			}
		}

		property List<System::Drawing::Rectangle>^ regionsOfInterest {
			List<System::Drawing::Rectangle>^ get() {
				return m_regionsOfInterest;
			}
		}

		property List<AlprPlateResultNet^>^ plates {
			List<AlprPlateResultNet^>^ get() {
				return m_plates;
			}
		}

		property System::String^ json {
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
			if(this->m_Disposed)
			{
				return;
			}

			this->!AlprNet();
			this->m_Disposed = true;
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

		void recognizeFromVideo(System::String^ videoPath) {
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
		AlprResultsNet^ recognize(System::String^ filepath) {
			AlprResults results = m_Impl->recognize(marshal_as<std::string>(filepath));
			return gcnew AlprResultsNet(results);
		}

		/// <summary>
		/// Recognize from an image on disk
		/// </summary>
		AlprResultsNet^ recognize(System::String^ filepath, List<System::Drawing::Rectangle>^ regionsOfInterest) {
			cv::Mat frame = cv::imread( marshal_as<std::string>(filepath) );
			std::vector<AlprRegionOfInterest> rois = AlprHelper::ToVector(regionsOfInterest);
			AlprResults results = m_Impl->recognize(frame.data, frame.elemSize(), frame.cols, frame.rows, rois );
			return gcnew AlprResultsNet(results);
		}

		/// <summary>
		/// Recognize from a bitmap
		/// </summary>
		AlprResultsNet^ recognize(Bitmap^ bitmap, List<System::Drawing::Rectangle>^ regionsOfInterest)
		{
			cv::Mat frame = AlprHelper::BitmapToMat(bitmap);
			std::vector<AlprRegionOfInterest> rois = AlprHelper::ToVector(regionsOfInterest);
			AlprResults results = m_Impl->recognize(frame.data, frame.elemSize(), frame.cols, frame.rows, rois);
			return gcnew AlprResultsNet(results);
		}

		/// <summary>
		/// Recognize from a bitmap
		/// </summary>
		AlprResultsNet^ recognize(Bitmap^ bitmap)
		{
			cv::Mat frame = AlprHelper::BitmapToMat(bitmap);
			std::vector<AlprRegionOfInterest> rois;
			AlprResults results = m_Impl->recognize(frame.data, frame.elemSize(), frame.cols, frame.rows, rois);
			return gcnew AlprResultsNet(results);
		}

		/// <summary>
		/// Recognize from MemoryStream representing an encoded image (e.g., BMP, PNG, JPG, GIF etc).
		/// </summary>
		AlprResultsNet^ recognize(MemoryStream^ memoryStream)
		{
			std::vector<char> p = AlprHelper::MemoryStreamToVector(memoryStream);
			AlprResults results = m_Impl->recognize(p);
			return gcnew AlprResultsNet(results);
		}

		/// <summary>
		/// Recognize from byte data representing an encoded image (e.g., BMP, PNG, JPG, GIF etc).
		/// </summary>
		/// <param name="imageBuffer">Bytes representing image data</param>
		AlprResultsNet^ recognize(cli::array<char>^ imageBuffer) {
			std::vector<char> p = AlprHelper::ToVector(imageBuffer);
			AlprResults results = m_Impl->recognize(p);
			return gcnew AlprResultsNet(results);
		}

		/// <summary>
		/// Recognize from raw pixel data
		/// </summary>
		AlprResultsNet^ recognize(cli::array<unsigned char>^ pixelData, int bytesPerPixel, int imgWidth, int imgHeight, List<System::Drawing::Rectangle>^ regionsOfInterest) {
			unsigned char* p = AlprHelper::ToCharPtr(pixelData);
			std::vector<AlprRegionOfInterest> rois = AlprHelper::ToVector(regionsOfInterest);
			AlprResults results = m_Impl->recognize(p, bytesPerPixel, imgWidth, imgHeight, rois);
			free(p); // ?? memory leak?
			return gcnew AlprResultsNet(results);
		}

		bool isLoaded() {
			return m_Impl->isLoaded();
		}

		static System::String^ getVersion() {
			return AlprHelper::ToManagedString(Alpr::getVersion());
		}

	protected:
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
		bool m_Disposed;
	};
}

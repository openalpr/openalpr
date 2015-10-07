#pragma once

#include "config.h" // alpr
#include "types-net.h"
#include "helper-net.h"

using namespace System;
using namespace openalprnet::types;

namespace openalprnet 
{

	public ref class AlprConfigNet sealed
	{
	internal:
		AlprConfigNet(Config* config) : m_config(config)
		{

		}

	public:

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

		property int DetectionStrictness {
			int get()
			{
				return this->m_config->detectionStrictness;
			}
			void set(int value)
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
				if(String::IsNullOrWhiteSpace(value))
				{
					this->m_config->prewarp = "";
					return;
				}
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

		property float MinPlateSizeHeightPx {
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

		property bool MustMatchPattern {
			bool get()
			{
				return this->m_config->mustMatchPattern;
			}
			void set(bool value)
			{
				this->m_config->mustMatchPattern = value;
			}
		}

		property String^ OcrLanguage {
			String^ get()
			{
				return AlprHelper::ToManagedString(this->m_config->ocrLanguage);
			}
			void set(String^ value)
			{
				if (String::IsNullOrWhiteSpace(value))
				{
					return;
				}
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

		void SetDebug(bool value)
		{
			this->m_config->setDebug(value);
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
}
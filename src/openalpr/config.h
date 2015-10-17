/*
 * Copyright (c) 2015 OpenALPR Technology, Inc.
 * Open source Automated License Plate Recognition [http://www.openalpr.com]
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


#ifndef OPENALPR_CONFIG_H
#define OPENALPR_CONFIG_H


#include "constants.h"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <stdlib.h>     /* getenv */
#include <math.h>

namespace alpr
{

  class Config
  {

    public:
      Config(const std::string country, const std::string config_file = "", const std::string runtime_dir = "");
      virtual ~Config();

      bool loaded;
      
      std::string config_file_path;

      std::string country;

      int detector;

      float detection_iteration_increase;
      int detectionStrictness;
      float maxPlateWidthPercent;
      float maxPlateHeightPercent;
      int maxDetectionInputWidth;
      int maxDetectionInputHeight;
      
      bool skipDetection;

      bool auto_invert;
      bool always_invert;

      std::string prewarp;
      
      int maxPlateAngleDegrees;

      float minPlateSizeWidthPx;
      float minPlateSizeHeightPx;

      bool multiline;

      float plateWidthMM;
      float plateHeightMM;

      std::vector<float> charHeightMM;
      std::vector<float> charWidthMM;
      
      float avgCharHeightMM;
      float avgCharWidthMM;
      
      float charWhitespaceTopMM;
      float charWhitespaceBotMM;
      float charWhitespaceBetweenLinesMM;

      int templateWidthPx;
      int templateHeightPx;

      int ocrImageWidthPx;
      int ocrImageHeightPx;

      int stateIdImageWidthPx;
      int stateIdimageHeightPx;

      float charAnalysisMinPercent;
      float charAnalysisHeightRange;
      float charAnalysisHeightStepSize;
      int charAnalysisNumSteps;

      float plateLinesSensitivityVertical;
      float plateLinesSensitivityHorizontal;

      float segmentationMinSpeckleHeightPercent;
      int segmentationMinBoxWidthPx;
      float segmentationMinCharHeightPercent;
      float segmentationMaxCharWidthvsAverage;

      std::string detectorFile;
      
      std::string ocrLanguage;
      int ocrMinFontSize;

      bool mustMatchPattern;
      
      float postProcessMinConfidence;
      float postProcessConfidenceSkipLevel;
      unsigned int postProcessMinCharacters;
      unsigned int postProcessMaxCharacters;

      std::string postProcessRegexLetters;
      std::string postProcessRegexNumbers;

      bool debugGeneral;
      bool debugTiming;
      bool debugPrewarp;
      bool debugDetector;
      bool debugStateId;
      bool debugPlateLines;
      bool debugPlateCorners;
      bool debugCharSegmenter;
      bool debugCharAnalysis;
      bool debugColorFiler;
      bool debugOcr;
      bool debugPostProcess;
      bool debugShowImages;
      bool debugPauseOnFrame;

      void setDebug(bool value);

      std::string getKeypointsRuntimeDir();
      std::string getCascadeRuntimeDir();
      std::string getPostProcessRuntimeDir();
      std::string getTessdataPrefix();

      std::string runtimeBaseDir;

      std::vector<std::string> loaded_countries;

      bool setCountry(std::string country);

    private:
    
      float ocrImagePercent;
      float stateIdImagePercent;

      std::vector<std::string> parse_country_string(std::string countries);

      void loadCommonValues(std::string configFile);
      void loadCountryValues(std::string configFile, std::string country);

  };


  enum DETECTOR_TYPE
  {
    DETECTOR_LBP_CPU=0,
    DETECTOR_LBP_GPU=1,
    DETECTOR_MORPH_CPU=2,
    DETECTOR_LBP_OPENCL=3
  };

}
#endif // OPENALPR_CONFIG_H

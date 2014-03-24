/*
 * Copyright (c) 2013 New Designs Unlimited, LLC
 * Opensource Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenAlpr.
 *
 * OpenAlpr is free software: you can redistribute it and/or modify
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

#ifndef CONFIG_H
#define CONFIG_H

#include "simpleini/simpleini.h"
#include "support/filesystem.h"

#include "constants.h"

#include <stdio.h>
#include <iostream>
#include <stdlib.h>     /* getenv */
#include <math.h>

using namespace std;

class Config
{

  public:
    Config(const std::string country, const std::string runtimeDir = "");
    virtual ~Config();

    string country;

    bool opencl_enabled;

    float maxPlateWidthPercent;
    float maxPlateHeightPercent;

    float minPlateSizeWidthPx;
    float minPlateSizeHeightPx;

    float plateWidthMM;
    float plateHeightMM;

    float charHeightMM;
    float charWidthMM;
    float charWhitespaceTopMM;
    float charWhitespaceBotMM;

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

    int segmentationMinBoxWidthPx;
    float segmentationMinCharHeightPercent;
    float segmentationMaxCharWidthvsAverage;

    string ocrLanguage;
    int ocrMinFontSize;

    float postProcessMinConfidence;
    float postProcessConfidenceSkipLevel;
    int postProcessMaxSubstitutions;
    int postProcessMinCharacters;
    int postProcessMaxCharacters;

    bool debugGeneral;
    bool debugTiming;
    bool debugStateId;
    bool debugPlateLines;
    bool debugPlateCorners;
    bool debugCharRegions;
    bool debugCharSegmenter;
    bool debugCharAnalysis;
    bool debugColorFiler;
    bool debugOcr;
    bool debugPostProcess;
    bool debugShowImages;

    void debugOff();

    string getKeypointsRuntimeDir();
    string getCascadeRuntimeDir();
    string getPostProcessRuntimeDir();
    string getTessdataPrefix();

  private:
    CSimpleIniA* ini;

    string runtimeBaseDir;

    void loadValues(string country);

    int getInt(string section, string key, int defaultValue);
    float getFloat(string section, string key, float defaultValue);
    string getString(string section, string key, string defaultValue);
    bool getBoolean(string section, string key, bool defaultValue);
};

#endif // CONFIG_H

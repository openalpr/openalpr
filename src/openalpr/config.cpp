/*
 * Copyright (c) 2015 New Designs Unlimited, LLC
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

#include "config.h"

using namespace std;

namespace alpr
{

  Config::Config(const std::string country, const std::string config_file, const std::string runtime_dir)
  {

    string debug_message = "";

    this->loaded = false;

    ini = new CSimpleIniA();

    string configFile;

    char* envConfigFile;
    envConfigFile = getenv (ENV_VARIABLE_CONFIG_FILE);
    if (config_file.compare("") != 0)
    {
        // User has supplied a config file.  Use that.
      configFile = config_file;
      debug_message = "Config file location provided via API";
    }
    else if (envConfigFile != NULL)
    {
      // Environment variable is non-empty.  Use that.
      configFile = envConfigFile;
      debug_message = "Config file location provided via environment variable: " + string(ENV_VARIABLE_CONFIG_FILE);
    }
    else if (DirectoryExists(getExeDir().c_str()) && fileExists((getExeDir() + CONFIG_FILE).c_str()))
    {
          configFile = getExeDir() + CONFIG_FILE;
      debug_message = "Config file location provided via exe location";
    }
    else
    {
      // Use the default
      configFile = DEFAULT_CONFIG_FILE;
      debug_message = "Config file location provided via default location";
    }

    //string configFile = (this->runtimeBaseDir + CONFIG_FILE);

    if (fileExists(configFile.c_str()) == false)
    {
      std::cerr << "--(!) Config file '" << configFile << "' does not exist!" << endl;
      std::cerr << "--(!)             You can specify the configuration file location via the command line " << endl;
      std::cerr << "--(!)             or by setting the environment variable '" << ENV_VARIABLE_CONFIG_FILE << "'" << endl;
      return;
    }
    else if (DirectoryExists(configFile.c_str()))
    {
      std::cerr << "--(!) Config file '" << configFile << "' was specified as a directory, rather than a file!" << endl;
      std::cerr << "--(!)             Please specify the full path to the 'openalpr.conf file'" << endl;
      std::cerr << "--(!)             e.g., /etc/openalpr/openalpr.conf" << endl;
      return;
    }

    ini->LoadFile(configFile.c_str());

    this->country = country;


    loadValues(country);

    if (runtime_dir.compare("") != 0)
    {
      // User provided a runtime directory directly into the library.  Use this.
      this->runtimeBaseDir = runtime_dir;
    }

    if ((DirectoryExists(this->runtimeBaseDir.c_str()) == false) &&
            (DirectoryExists((getExeDir() + RUNTIME_DIR).c_str())))
    {
            // Runtime dir in the config is invalid and there is a runtime dir in the same dir as the exe.
      this->runtimeBaseDir = getExeDir() + RUNTIME_DIR;

    }

    if (DirectoryExists(this->runtimeBaseDir.c_str()) == false)
    {
      std::cerr << "--(!) Runtime directory '" << this->runtimeBaseDir << "' does not exist!" << endl;
      std::cerr << "--(!)                   Please update the OpenALPR config file: '" << configFile << "'" << endl;
      std::cerr << "--(!)                   to point to the correct location of your runtime_dir" << endl;
      return;
    }
    else if (fileExists((this->runtimeBaseDir + "/ocr/tessdata/" + this->ocrLanguage + ".traineddata").c_str()) == false)
    {
      std::cerr << "--(!) Runtime directory '" << this->runtimeBaseDir << "' is invalid.  Missing OCR data for the country: '" << country<< "'!" << endl;
      return;
    }


    if (this->debugGeneral)
    {
      std::cout << debug_message << endl;
    }

    this->loaded = true;
  }
  Config::~Config()
  {
    delete ini;
  }

  void Config::loadValues(string country)
  {

    runtimeBaseDir = getString("common", "runtime_dir", "/usr/share/openalpr/runtime_data");

    detection_iteration_increase = getFloat("common", "detection_iteration_increase", 1.1);
    detectionStrictness = getInt("common", "detection_strictness", 3);
    maxPlateWidthPercent = getFloat("common", "max_plate_width_percent", 100);
    maxPlateHeightPercent = getFloat("common", "max_plate_height_percent", 100);
    maxDetectionInputWidth = getInt("common", "max_detection_input_width", 1280);
    maxDetectionInputHeight = getInt("common", "max_detection_input_height", 768);

    skipDetection = getBoolean("common", "skip_detection", false);
    
    maxPlateAngleDegrees = getInt("common", "max_plate_angle_degrees", 15);

    minPlateSizeWidthPx = getInt(country, "min_plate_size_width_px", 100);
    minPlateSizeHeightPx = getInt(country, "min_plate_size_height_px", 100);

    multiline = 	getBoolean(country, "multiline",		false);

    plateWidthMM = getFloat(country, "plate_width_mm", 100);
    plateHeightMM = getFloat(country, "plate_height_mm", 100);

    charHeightMM = getFloat(country, "char_height_mm", 100);
    charWidthMM = getFloat(country, "char_width_mm", 100);
    charWhitespaceTopMM = getFloat(country, "char_whitespace_top_mm", 100);
    charWhitespaceBotMM = getFloat(country, "char_whitespace_bot_mm", 100);

    templateWidthPx = getInt(country, "template_max_width_px", 100);
    templateHeightPx = getInt(country, "template_max_height_px", 100);

    float ocrImagePercent = getFloat("common", "ocr_img_size_percent", 100);
    ocrImageWidthPx = round(((float) templateWidthPx) * ocrImagePercent);
    ocrImageHeightPx = round(((float)templateHeightPx) * ocrImagePercent);


    float stateIdImagePercent = getFloat("common", "state_id_img_size_percent", 100);
    stateIdImageWidthPx = round(((float)templateWidthPx) * stateIdImagePercent);
    stateIdimageHeightPx = round(((float)templateHeightPx) * stateIdImagePercent);


    charAnalysisMinPercent = getFloat(country, "char_analysis_min_pct", 0);
    charAnalysisHeightRange = getFloat(country, "char_analysis_height_range", 0);
    charAnalysisHeightStepSize = getFloat(country, "char_analysis_height_step_size", 0);
    charAnalysisNumSteps = getInt(country, "char_analysis_height_num_steps", 0);

    segmentationMinBoxWidthPx = getInt(country, "segmentation_min_box_width_px", 0);
    segmentationMinCharHeightPercent = getFloat(country, "segmentation_min_charheight_percent", 0);
    segmentationMaxCharWidthvsAverage = getFloat(country, "segmentation_max_segment_width_percent_vs_average", 0);

    plateLinesSensitivityVertical = getFloat(country, "plateline_sensitivity_vertical", 0);
    plateLinesSensitivityHorizontal = getFloat(country, "plateline_sensitivity_horizontal", 0);

    ocrLanguage = getString(country, "ocr_language", "none");
    ocrMinFontSize = getInt("common", "ocr_min_font_point", 100);

    postProcessMinConfidence = getFloat("common", "postprocess_min_confidence", 100);
    postProcessConfidenceSkipLevel = getFloat("common", "postprocess_confidence_skip_level", 100);
    postProcessMaxSubstitutions = getInt("common", "postprocess_max_substitutions", 100);
    postProcessMinCharacters = getInt("common", "postprocess_min_characters", 100);
    postProcessMaxCharacters = getInt("common", "postprocess_max_characters", 100);

    debugGeneral = 	getBoolean("debug", "general",		false);
    debugTiming = 	getBoolean("debug", "timing",		false);
    debugStateId = 	getBoolean("debug", "state_id",		false);
    debugPlateLines = 	getBoolean("debug", "plate_lines", 	false);
    debugPlateCorners = 	getBoolean("debug", "plate_corners", 	false);
    debugCharSegmenter = 	getBoolean("debug", "char_segment", 	false);
    debugCharAnalysis =	getBoolean("debug", "char_analysis",	false);
    debugColorFiler = 	getBoolean("debug", "color_filter", 	false);
    debugOcr = 		getBoolean("debug", "ocr", 		false);
    debugPostProcess = 	getBoolean("debug", "postprocess", 	false);
    debugShowImages = 	getBoolean("debug", "show_images",	false);
    debugPauseOnFrame = 	getBoolean("debug", "pause_on_frame",	false);

  }

  void Config::debugOff()
  {
    debugGeneral = 	false;
    debugTiming = 	false;
    debugStateId = 	false;
    debugPlateLines = 	false;
    debugPlateCorners = 	false;
    debugCharSegmenter = 	false;
    debugCharAnalysis =	false;
    debugColorFiler = 	false;
    debugOcr = 		false;
    debugPostProcess = 	false;
    debugPauseOnFrame = 	false;
  }


  string Config::getCascadeRuntimeDir()
  {
    return this->runtimeBaseDir + CASCADE_DIR;
  }
  string Config::getKeypointsRuntimeDir()
  {
    return this->runtimeBaseDir + KEYPOINTS_DIR;
  }
  string Config::getPostProcessRuntimeDir()
  {
    return this->runtimeBaseDir + POSTPROCESS_DIR;
  }
  string Config::getTessdataPrefix()
  {
    return this->runtimeBaseDir + "/ocr/";
  }




  float Config::getFloat(string section, string key, float defaultValue)
  {
    const char * pszValue = ini->GetValue(section.c_str(), key.c_str(), NULL /*default*/);
    if (pszValue == NULL)
    {
      std::cout << "Error: missing configuration entry for: " << section << "->" << key << endl;
      return defaultValue;
    }

    float val = atof(pszValue);
    return val;
  }
  int Config::getInt(string section, string key, int defaultValue)
  {
    const char * pszValue = ini->GetValue(section.c_str(), key.c_str(), NULL /*default*/);
    if (pszValue == NULL)
    {
      std::cout << "Error: missing configuration entry for: " << section << "->" << key << endl;
      return defaultValue;
    }

    int val = atoi(pszValue);
    return val;
  }
  bool Config::getBoolean(string section, string key, bool defaultValue)
  {
    const char * pszValue = ini->GetValue(section.c_str(), key.c_str(), NULL /*default*/);
    if (pszValue == NULL)
    {
      std::cout << "Error: missing configuration entry for: " << section << "->" << key << endl;
      return defaultValue;
    }

    int val = atoi(pszValue);
    return val != 0;
  }
  string Config::getString(string section, string key, string defaultValue)
  {
    const char * pszValue = ini->GetValue(section.c_str(), key.c_str(), NULL /*default*/);
    if (pszValue == NULL)
    {
      std::cout << "Error: missing configuration entry for: " << section << "->" << key << endl;
      return defaultValue;
    }

    string val = string(pszValue);
    return val;
  }
}
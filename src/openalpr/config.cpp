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

#include "config.h"
#include "support/filesystem.h"
#include "support/platform.h"
#include "simpleini/simpleini.h"
#include "utility.h"
#include "config_helper.h"

using namespace std;

namespace alpr
{


  Config::Config(const std::string country, const std::string config_file, const std::string runtime_dir)
  {

    string debug_message = "";

    this->loaded = false;



    char* envConfigFile;
    envConfigFile = getenv (ENV_VARIABLE_CONFIG_FILE);
    if (config_file.compare("") != 0)
    {
        // User has supplied a config file.  Use that.
      config_file_path = config_file;
      debug_message = "Config file location provided via API";
    }
    else if (envConfigFile != NULL)
    {
      // Environment variable is non-empty.  Use that.
      config_file_path = envConfigFile;
      debug_message = "Config file location provided via environment variable: " + string(ENV_VARIABLE_CONFIG_FILE);
    }
    else if (DirectoryExists(getExeDir().c_str()) && fileExists((getExeDir() + CONFIG_FILE).c_str()))
    {
          config_file_path = getExeDir() + CONFIG_FILE;
      debug_message = "Config file location provided via exe location";
    }
    else
    {
      // Use the default
      config_file_path = DEFAULT_CONFIG_FILE;
      debug_message = "Config file location provided via default location";
    }


    if (fileExists(config_file_path.c_str()) == false && fileExists(CONFIG_FILE_TEMPLATE_LOCATION) == false)
    {
      std::cerr << "--(!) Config file '" << config_file_path << "' does not exist!" << endl;
      std::cerr << "--(!)             You can specify the configuration file location via the command line " << endl;
      std::cerr << "--(!)             or by setting the environment variable '" << ENV_VARIABLE_CONFIG_FILE << "'" << endl;
      return;
    }
    else if (DirectoryExists(config_file_path.c_str()))
    {
      std::cerr << "--(!) Config file '" << config_file_path << "' was specified as a directory, rather than a file!" << endl;
      std::cerr << "--(!)             Please specify the full path to the 'openalpr.conf file'" << endl;
      std::cerr << "--(!)             e.g., /etc/openalpr/openalpr.conf" << endl;
      return;
    }
    
    
    loadCommonValues(config_file_path);

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
      std::cerr << "--(!)                   Please update the OpenALPR config file: '" << config_file_path << "'" << endl;
      std::cerr << "--(!)                   to point to the correct location of your runtime_dir" << endl;
      return;
    }

    bool countries_loaded = load_countries(country);

    if (this->debugGeneral)
    {
      std::cout << debug_message << endl;
    }

    this->loaded = countries_loaded;
  }
  
  Config::~Config()
  {
    
  }
  
  bool Config::load_countries(const std::string countries) {

    this->loaded_countries = this->parse_country_string(countries);

    if (this->loaded_countries.size() == 0)
    {
      std::cerr << "--(!) Country not specified." << endl;
      return false;
    }
    for (unsigned int i = 0; i < loaded_countries.size(); i++)
    {
      bool country_loaded = setCountry(this->loaded_countries[i]);
      if (!country_loaded)
      {
        return false;
      }
    }
    
    setCountry(this->loaded_countries[0]);
    
    return true;
  }

  
  void Config::loadCommonValues(string configFile)
  {

    CSimpleIniA* ini = NULL;
    CSimpleIniA iniObj;
    if (fileExists(configFile.c_str()))
    {
      iniObj.LoadFile(configFile.c_str());
      ini = &iniObj;
    }

    
    CSimpleIniA* defaultIni = NULL;
    CSimpleIniA defaultIniObj;
    if (fileExists(CONFIG_FILE_TEMPLATE_LOCATION))
    {
      defaultIniObj.LoadFile(CONFIG_FILE_TEMPLATE_LOCATION);
      defaultIni = &defaultIniObj;
    }
    
    runtimeBaseDir = getString(ini,defaultIni, "", "runtime_dir", DEFAULT_RUNTIME_DATA_DIR);
   
    // Special hack to allow config files to work if the package hasn't been installed
    // Cmake will do this replacement on deploy, but this is useful in development
    if (runtimeBaseDir.find("${CMAKE_INSTALL_PREFIX}") >= 0)
      runtimeBaseDir = replaceAll(runtimeBaseDir, "${CMAKE_INSTALL_PREFIX}", INSTALL_PREFIX);
    
    std::string detectorString = getString(ini, defaultIni, "", "detector", "lbpcpu");
    std::transform(detectorString.begin(), detectorString.end(), detectorString.begin(), ::tolower);

    if (detectorString.compare("lbpcpu") == 0)
      detector = DETECTOR_LBP_CPU;
    else if (detectorString.compare("lbpgpu") == 0)
      detector = DETECTOR_LBP_GPU;
    else if (detectorString.compare("lbpopencl") == 0)
      detector = DETECTOR_LBP_OPENCL;
    else if (detectorString.compare("morphcpu") == 0)
      detector = DETECTOR_MORPH_CPU;
    else
    {
      std::cerr << "Invalid detector specified: " << detectorString << ".  Using default" << std::endl;
      detector = DETECTOR_LBP_CPU;
    }
    
    detection_iteration_increase = getFloat(ini, defaultIni, "", "detection_iteration_increase", 1.1);
    detectionStrictness = getInt(ini, defaultIni, "", "detection_strictness", 3);
    maxPlateWidthPercent = getFloat(ini, defaultIni, "", "max_plate_width_percent", 100);
    maxPlateHeightPercent = getFloat(ini, defaultIni, "", "max_plate_height_percent", 100);
    maxDetectionInputWidth = getInt(ini, defaultIni, "", "max_detection_input_width", 1280);
    maxDetectionInputHeight = getInt(ini, defaultIni, "", "max_detection_input_height", 768);

    contrastDetectionThreshold = getFloat(ini, defaultIni, "", "contrast_detection_threshold", 0.3);
    
    mustMatchPattern = getBoolean(ini, defaultIni, "", "must_match_pattern", false);
    
    skipDetection = getBoolean(ini, defaultIni, "", "skip_detection", false);
    
    detection_mask_image = getString(ini, defaultIni, "", "detection_mask_image", "");
    
    analysis_count = getInt(ini, defaultIni, "", "analysis_count", 1);
    
    prewarp = getString(ini, defaultIni, "", "prewarp", "");
            
    maxPlateAngleDegrees = getInt(ini, defaultIni, "", "max_plate_angle_degrees", 15);


    ocrImagePercent = getFloat(ini, defaultIni, "", "ocr_img_size_percent", 100);
    stateIdImagePercent = getFloat(ini, defaultIni, "", "state_id_img_size_percent", 100);

    ocrMinFontSize = getInt(ini, defaultIni, "", "ocr_min_font_point", 100);

    postProcessMinConfidence = getFloat(ini, defaultIni, "", "postprocess_min_confidence", 100);
    postProcessConfidenceSkipLevel = getFloat(ini, defaultIni, "", "postprocess_confidence_skip_level", 100);

    debugGeneral = 	getBoolean(ini, defaultIni, "", "debug_general",		false);
    debugTiming = 	getBoolean(ini, defaultIni, "", "debug_timing",		false);
    debugPrewarp = 	getBoolean(ini, defaultIni, "", "debug_prewarp",		false);
    debugDetector = 	getBoolean(ini, defaultIni, "", "debug_detector",		false);
    debugStateId = 	getBoolean(ini, defaultIni, "", "debug_state_id",		false);
    debugPlateLines = 	getBoolean(ini, defaultIni, "", "debug_plate_lines", 	false);
    debugPlateCorners = 	getBoolean(ini, defaultIni, "", "debug_plate_corners", 	false);
    debugCharSegmenter = 	getBoolean(ini, defaultIni, "", "debug_char_segment", 	false);
    debugCharAnalysis =	getBoolean(ini, defaultIni, "", "debug_char_analysis",	false);
    debugColorFiler = 	getBoolean(ini, defaultIni, "", "debug_color_filter", 	false);
    debugOcr = 		getBoolean(ini, defaultIni, "", "debug_ocr", 		false);
    debugPostProcess = 	getBoolean(ini, defaultIni, "", "debug_postprocess", 	false);
    debugShowImages = 	getBoolean(ini, defaultIni, "", "debug_show_images",	false);
    debugPauseOnFrame = 	getBoolean(ini, defaultIni, "", "debug_pause_on_frame",	false);

  }
  
  
  void Config::loadCountryValues(string configFile, string country)
  {
    CSimpleIniA iniObj;
    iniObj.SetMultiKey(true);
    iniObj.LoadFile(configFile.c_str());
    CSimpleIniA* ini = &iniObj;
    
    minPlateSizeWidthPx = getInt(ini, "", "min_plate_size_width_px", 100);
    minPlateSizeHeightPx = getInt(ini, "", "min_plate_size_height_px", 100);

    multiline = 	getBoolean(ini, "", "multiline",		false);

    string invert_val = getString(ini, "", "invert", "auto");

    if (invert_val == "always")
    {
      auto_invert = false;
      always_invert = true;
    }
    else if (invert_val == "never")
    {
      auto_invert = false;
      always_invert = false;
    }
    else
    {
      auto_invert = true;
      always_invert = false;
    }

    plateWidthMM = getFloat(ini, "", "plate_width_mm", 100);
    plateHeightMM = getFloat(ini, "", "plate_height_mm", 100);

    charHeightMM = getAllFloats(ini, "", "char_height_mm");
    charWidthMM = getAllFloats(ini, "", "char_width_mm");
    
    // Compute the average char height/widths 
    avgCharHeightMM = 0;
    avgCharWidthMM = 0;
    for (unsigned int i = 0; i < charHeightMM.size(); i++)
    {
      avgCharHeightMM += charHeightMM[i];
      avgCharWidthMM += charWidthMM[i];
    }
    avgCharHeightMM /= charHeightMM.size();
    avgCharWidthMM /= charHeightMM.size();
    
    charWhitespaceTopMM = getFloat(ini, "", "char_whitespace_top_mm", 100);
    charWhitespaceBotMM = getFloat(ini, "", "char_whitespace_bot_mm", 100);
    charWhitespaceBetweenLinesMM = getFloat(ini, "", "char_whitespace_between_lines_mm", 5);

    templateWidthPx = getInt(ini, "", "template_max_width_px", 100);
    templateHeightPx = getInt(ini, "", "template_max_height_px", 100);

    charAnalysisMinPercent = getFloat(ini, "", "char_analysis_min_pct", 0);
    charAnalysisHeightRange = getFloat(ini, "", "char_analysis_height_range", 0);
    charAnalysisHeightStepSize = getFloat(ini, "", "char_analysis_height_step_size", 0);
    charAnalysisNumSteps = getInt(ini, "", "char_analysis_height_num_steps", 0);

    segmentationMinSpeckleHeightPercent = getFloat(ini, "", "segmentation_min_speckle_height_percent", 0);
    segmentationMinBoxWidthPx = getInt(ini, "", "segmentation_min_box_width_px", 0);
    segmentationMinCharHeightPercent = getFloat(ini, "", "segmentation_min_charheight_percent", 0);
    segmentationMaxCharWidthvsAverage = getFloat(ini, "", "segmentation_max_segment_width_percent_vs_average", 0);

    plateLinesSensitivityVertical = getFloat(ini, "", "plateline_sensitivity_vertical", 0);
    plateLinesSensitivityHorizontal = getFloat(ini, "", "plateline_sensitivity_horizontal", 0);

    detectorFile = getString(ini, "", "detector_file", "");
    
    ocrLanguage = getString(ini, "", "ocr_language", "none");

    postProcessRegexLetters = getString(ini, "", "postprocess_regex_letters", "\\pL");
    postProcessRegexNumbers = getString(ini, "", "postprocess_regex_numbers", "\\pN");

    ocrImageWidthPx = round(((float) templateWidthPx) * ocrImagePercent);
    ocrImageHeightPx = round(((float)templateHeightPx) * ocrImagePercent);
    stateIdImageWidthPx = round(((float)templateWidthPx) * stateIdImagePercent);
    stateIdimageHeightPx = round(((float)templateHeightPx) * stateIdImagePercent);

    postProcessMinCharacters = getInt(ini, "", "postprocess_min_characters", 4);
    postProcessMaxCharacters = getInt(ini, "", "postprocess_max_characters", 8);
  }

  void Config::setDebug(bool value)
  {
    debugGeneral = value;
    debugTiming = value;
    debugPrewarp = value;
    debugDetector = value;
    debugStateId = value;
    debugPlateLines = value;
    debugPlateCorners = value;
    debugCharSegmenter = value;
    debugCharAnalysis = value;
    debugColorFiler = value;
    debugOcr = value;
    debugPostProcess = value;
    debugPauseOnFrame = value;
    debugShowImages = value;
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


  std::vector<std::string> Config::parse_country_string(std::string countries)
  {
    std::istringstream ss(countries);
    std::string token;

    std::vector<std::string>  parsed_countries;
    while(std::getline(ss, token, ',')) {
      std::string trimmed_token = trim(token);
      if (trimmed_token.size() > 0)
        parsed_countries.push_back(trimmed_token);
    }

    return parsed_countries;
  }

  bool Config::country_is_loaded(std::string country) {
    for (uint32_t i = 0; i < loaded_countries.size(); i++)
    {
      if (loaded_countries[i] == country)
        return true;
    }
    
    return false;
  }

  bool Config::setCountry(std::string country)
  {
    this->country = country;
    
    

    std::string country_config_file = this->runtimeBaseDir + "/config/" + country + ".conf";
    if (fileExists(country_config_file.c_str()) == false)
    {
      std::cerr << "--(!) Country config file '" << country_config_file << "' does not exist.  Missing config for the country: '" << country<< "'!" << endl;
      return false;
    }

    loadCountryValues(country_config_file, country);

    if (fileExists((this->runtimeBaseDir + "/ocr/tessdata/" + this->ocrLanguage + ".traineddata").c_str()) == false)
    {
      std::cerr << "--(!) Runtime directory '" << this->runtimeBaseDir << "' is invalid.  Missing OCR data for the country: '" << country<< "'!" << endl;
      return false;
    }

    if (!country_is_loaded(country))
      this->loaded_countries.push_back(country);
    
    return true;
  }

}



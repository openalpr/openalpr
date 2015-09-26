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

#include "ocr.h"

using namespace std;
using namespace cv;
using namespace tesseract;

namespace alpr
{

  OCR::OCR(Config* config)
  : postProcessor(config)
  {
    const string MINIMUM_TESSERACT_VERSION = "3.03";

    this->config = config;
    
    if (cmpVersion(tesseract.Version(), MINIMUM_TESSERACT_VERSION.c_str()) < 0)
    {
      std::cerr << "Warning: You are running an unsupported version of Tesseract." << endl;
      std::cerr << "Expecting at least " << MINIMUM_TESSERACT_VERSION << ", your version is: " << tesseract.Version() << endl;
    }

    // Tesseract requires the prefix directory to be set as an env variable
    tesseract.Init(config->getTessdataPrefix().c_str(), config->ocrLanguage.c_str() 	);
    tesseract.SetVariable("save_blob_choices", "T");
    tesseract.SetVariable("debug_file", "/dev/null");
    tesseract.SetPageSegMode(PSM_SINGLE_CHAR);
  }

  OCR::~OCR()
  {
    tesseract.End();
  }

  void OCR::performOCR(PipelineData* pipeline_data)
  {
    const int SPACE_CHAR_CODE = 32;
    
    timespec startTime;
    getTimeMonotonic(&startTime);

    postProcessor.clear();

    // Don't waste time on OCR processing if it is impossible to get sufficient characters
    int total_char_spaces = 0;
    for (unsigned int i = 0; i < pipeline_data->charRegions.size(); i++)
      total_char_spaces += pipeline_data->charRegions[i].size();
    if (total_char_spaces < config->postProcessMinCharacters)
    {
      pipeline_data->disqualify_reason = "Insufficient character boxes detected.  No OCR performed.";
      pipeline_data->disqualified = true;
      return;
    }

    for (unsigned int i = 0; i < pipeline_data->thresholds.size(); i++)
    {
      // Make it black text on white background
      bitwise_not(pipeline_data->thresholds[i], pipeline_data->thresholds[i]);
      tesseract.SetImage((uchar*) pipeline_data->thresholds[i].data, 
                          pipeline_data->thresholds[i].size().width, pipeline_data->thresholds[i].size().height, 
                          pipeline_data->thresholds[i].channels(), pipeline_data->thresholds[i].step1());

      int absolute_charpos = 0;
      for (unsigned int line_idx = 0; line_idx < pipeline_data->charRegions.size(); line_idx++)
      {
        for (unsigned int j = 0; j < pipeline_data->charRegions[line_idx].size(); j++)
        {
          Rect expandedRegion = expandRect( pipeline_data->charRegions[line_idx][j], 2, 2, pipeline_data->thresholds[i].cols, pipeline_data->thresholds[i].rows) ;

          tesseract.SetRectangle(expandedRegion.x, expandedRegion.y, expandedRegion.width, expandedRegion.height);
          tesseract.Recognize(NULL);

          tesseract::ResultIterator* ri = tesseract.GetIterator();
          tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;
          do
          {
            const char* symbol = ri->GetUTF8Text(level);
            float conf = ri->Confidence(level);

            bool dontcare;
            int fontindex = 0;
            int pointsize = 0;
            const char* fontName = ri->WordFontAttributes(&dontcare, &dontcare, &dontcare, &dontcare, &dontcare, &dontcare, &pointsize, &fontindex);

            // Ignore NULL pointers, spaces, and characters that are way too small to be valid
            if(symbol != 0 && symbol[0] != SPACE_CHAR_CODE && pointsize >= config->ocrMinFontSize)
            {
              postProcessor.addLetter(string(symbol), line_idx, absolute_charpos, conf);

              if (this->config->debugOcr)
                printf("charpos%d line%d: threshold %d:  symbol %s, conf: %f font: %s (index %d) size %dpx", absolute_charpos, line_idx, i, symbol, conf, fontName, fontindex, pointsize);

              bool indent = false;
              tesseract::ChoiceIterator ci(*ri);
              do
              {
                const char* choice = ci.GetUTF8Text();

                postProcessor.addLetter(string(choice), line_idx, absolute_charpos, ci.Confidence());

                if (this->config->debugOcr)
                {
                  if (indent) printf("\t\t ");
                  printf("\t- ");
                  printf("%s conf: %f\n", choice, ci.Confidence());
                }

                indent = true;
              }
              while(ci.Next());
              
            }

            if (this->config->debugOcr)
              printf("---------------------------------------------\n");

            delete[] symbol;
          }
          while((ri->Next(level)));

          delete ri;
          
          absolute_charpos++;
        }
      }
    }

    if (config->debugTiming)
    {
      timespec endTime;
      getTimeMonotonic(&endTime);
      cout << "OCR Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }
  }

}
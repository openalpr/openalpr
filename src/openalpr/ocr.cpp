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

#include "ocr.h"

using namespace std;
using namespace cv;
using namespace tesseract;

OCR::OCR(Config* config)
{
  const string EXPECTED_TESSERACT_VERSION = "3.03";
  
  this->config = config;

  this->postProcessor = new PostProcess(config);

  tesseract=new TessBaseAPI();

  if (tesseract->Version() != EXPECTED_TESSERACT_VERSION)
  {
    std::cerr << "Warning: You are running an unsupported version of Tesseract." << endl;
    std::cerr << "Expecting version " << EXPECTED_TESSERACT_VERSION << ", your version is: " << tesseract->Version() << endl;
  }
  
  // Tesseract requires the prefix directory to be set as an env variable
  tesseract->Init(config->getTessdataPrefix().c_str(), config->ocrLanguage.c_str() 	);
  tesseract->SetVariable("save_blob_choices", "T");
  tesseract->SetPageSegMode(PSM_SINGLE_CHAR);
}

OCR::~OCR()
{
  tesseract->Clear();
  delete postProcessor;
  delete tesseract;
}

void OCR::performOCR(PipelineData* pipeline_data)
{
  timespec startTime;
  getTime(&startTime);

  postProcessor->clear();
  
  // Don't waste time on OCR processing if it is impossible to get sufficient characters
  if (pipeline_data->charRegions.size() < config->postProcessMinCharacters)
    return;

  for (int i = 0; i < pipeline_data->thresholds.size(); i++)
  {
    // Make it black text on white background
    bitwise_not(pipeline_data->thresholds[i], pipeline_data->thresholds[i]);
    tesseract->SetImage((uchar*) pipeline_data->thresholds[i].data, 
			pipeline_data->thresholds[i].size().width, pipeline_data->thresholds[i].size().height, 
			pipeline_data->thresholds[i].channels(), pipeline_data->thresholds[i].step1());

    for (int j = 0; j < pipeline_data->charRegions.size(); j++)
    {
      Rect expandedRegion = expandRect( pipeline_data->charRegions[j], 2, 2, pipeline_data->thresholds[i].cols, pipeline_data->thresholds[i].rows) ;

      tesseract->SetRectangle(expandedRegion.x, expandedRegion.y, expandedRegion.width, expandedRegion.height);
      tesseract->Recognize(NULL);

      tesseract::ResultIterator* ri = tesseract->GetIterator();
      tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;
      do
      {
        const char* symbol = ri->GetUTF8Text(level);
        float conf = ri->Confidence(level);

        bool dontcare;
        int fontindex = 0;
        int pointsize = 0;
        const char* fontName = ri->WordFontAttributes(&dontcare, &dontcare, &dontcare, &dontcare, &dontcare, &dontcare, &pointsize, &fontindex);

        if(symbol != 0  && pointsize >= config->ocrMinFontSize)
        {
          postProcessor->addLetter(*symbol, j, conf);

          if (this->config->debugOcr)
            printf("charpos%d: threshold %d:  symbol %s, conf: %f font: %s (index %d) size %dpx", j, i, symbol, conf, fontName, fontindex, pointsize);

          bool indent = false;
          tesseract::ChoiceIterator ci(*ri);
          do
          {
            const char* choice = ci.GetUTF8Text();

            postProcessor->addLetter(*choice, j, ci.Confidence());

            //letterScores.addScore(*choice, j, ci.Confidence() - MIN_CONFIDENCE);
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
    }
  }

  if (config->debugTiming)
  {
    timespec endTime;
    getTime(&endTime);
    cout << "OCR Time: " << diffclock(startTime, endTime) << "ms." << endl;
  }
}

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

OCR::OCR(Config* config)
{
  this->config = config;

  this->postProcessor = new PostProcess(config);

  tesseract=new TessBaseAPI();

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

void OCR::performOCR(vector<Mat> thresholds, vector<Rect> charRegions)
{
  timespec startTime;
  getTime(&startTime);

  postProcessor->clear();
  
  // Don't waste time on OCR processing if it is impossible to get sufficient characters
  if (charRegions.size() < config->postProcessMinCharacters)
    return;

  for (int i = 0; i < thresholds.size(); i++)
  {
    // Make it black text on white background
    bitwise_not(thresholds[i], thresholds[i]);
    tesseract->SetImage((uchar*) thresholds[i].data, thresholds[i].size().width, thresholds[i].size().height, thresholds[i].channels(), thresholds[i].step1());

    for (int j = 0; j < charRegions.size(); j++)
    {
      Rect expandedRegion = expandRect( charRegions[j], 2, 2, thresholds[i].cols, thresholds[i].rows) ;

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

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

#ifndef OPENALPR_TESSERACTOCR_H
#define OPENALPR_TESSERACTOCR_H

#include <vector>

#include "utility.h"
#include "config.h"
#include "pipeline_data.h"

#include "constants.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "support/filesystem.h"
#include "support/version.h"

#include "ocr.h"
#include "tesseract/baseapi.h"

namespace alpr
{

  class TesseractOcr : public OCR 
  {

    public:
      TesseractOcr(Config* config);
      virtual ~TesseractOcr();



    private:

      std::vector<OcrChar> recognize_line(int line_index, PipelineData* pipeline_data);
      void segment(PipelineData* pipeline_data);
    
      tesseract::TessBaseAPI tesseract;

  };

}

#endif // OPENALPR_TESSERACTOCR_H

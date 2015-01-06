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

#ifndef OPENALPR_OCR_H
#define OPENALPR_OCR_H

#include <iostream>
#include <stdio.h>

#include "utility.h"
#include "postprocess.h"
#include "config.h"
#include "pipeline_data.h"

#include "constants.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "support/filesystem.h"

#include "tesseract/baseapi.h"

namespace alpr
{

  class OCR
  {

    public:
      OCR(Config* config);
      virtual ~OCR();

      void performOCR(PipelineData* pipeline_data);

      PostProcess postProcessor;

    private:
      Config* config;

      tesseract::TessBaseAPI tesseract;

  };

}

#endif // OPENALPR_OCR_H

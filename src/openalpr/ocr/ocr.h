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

#ifndef OPENALPR_OCR_H
#define	OPENALPR_OCR_H

#include "postprocess/postprocess.h"
#include "pipeline_data.h"

namespace alpr
{
  class OCR {
  public:
    OCR(Config* config);
    virtual ~OCR();

    virtual void performOCR(PipelineData* pipeline_data)=0;

    PostProcess postProcessor;

  protected:
    Config* config;

  };
}

#endif	/* OPENALPR_OCR_H */


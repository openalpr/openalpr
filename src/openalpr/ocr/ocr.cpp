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

namespace alpr
{
  
  OCR::OCR(Config* config) : postProcessor(config) {
    this->config = config;
  }


  OCR::~OCR() {
  }

  
  void OCR::performOCR(PipelineData* pipeline_data)
  {
    
    timespec startTime;
    getTimeMonotonic(&startTime);

    segment(pipeline_data);
    
    postProcessor.clear();


    int absolute_charpos = 0;
    for (unsigned int line_idx = 0; line_idx < pipeline_data->textLines.size(); line_idx++)
    {
      std::vector<OcrChar> chars = recognize_line(line_idx, pipeline_data);
      
      for (uint32_t i = 0; i < chars.size(); i++)
      {
        // For multi-line plates, set the character indexes to sequential values based on the line number
        int line_ordered_index = (line_idx * config->postProcessMaxCharacters) + chars[i].char_index;
        postProcessor.addLetter(chars[i].letter, line_idx, line_ordered_index, chars[i].confidence);
        absolute_charpos++;
      }
    }
    

    if (config->debugTiming)
    {
      timespec endTime;
      getTimeMonotonic(&endTime);
      std::cout << "OCR Time: " << diffclock(startTime, endTime) << "ms." << std::endl;
    }
  }
}
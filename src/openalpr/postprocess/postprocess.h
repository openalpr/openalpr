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

#ifndef OPENALPR_POSTPROCESS_H
#define OPENALPR_POSTPROCESS_H

#include "regexrule.h"
#include "constants.h"
#include "utility.h"
#include <set>
#include <string>
#include <vector>
#include "config.h"


#define SKIP_CHAR "~"

namespace alpr
{

  struct Letter
  {
    std::string letter;
    int line_index;
    int charposition;
    float totalscore;
    int occurrences;
  };

  struct PPResult
  {
    std::string letters;
    float totalscore;
    bool matchesTemplate;
    std::vector<Letter> letter_details;
  };

  bool letterCompare( const Letter &left, const Letter &right );

  
  class PostProcess
  {
    public:
      PostProcess(Config* config);
      ~PostProcess();

      void addLetter(std::string letter, int line_index, int charposition, float score);

      void clear();
      void analyze(std::string templateregion, int topn);

      std::string bestChars;
      bool matchesTemplate;

      const std::vector<PPResult> getResults();

      bool regionIsValid(std::string templateregion);
      
      std::vector<std::string> getPatterns();
      
      void setConfidenceThreshold(float min_confidence, float skip_level);
      
    private:
      Config* config;

      void findAllPermutations(std::string templateregion, int topn);
      bool analyzePermutation(std::vector<int> letterIndices, std::string templateregion, int topn);

      void insertLetter(std::string letter, int line_index, int charPosition, float score);

      std::map<std::string, std::vector<RegexRule*> > rules;

      float calculateMaxConfidenceScore();

      std::vector<std::vector<Letter> > letters;
      std::vector<int> unknownCharPositions;

      std::vector<PPResult> allPossibilities;
      std::set<std::string> allPossibilitiesLetters;
      
      float min_confidence;
      float skip_level;
  };

}
#endif // OPENALPR_POSTPROCESS_H

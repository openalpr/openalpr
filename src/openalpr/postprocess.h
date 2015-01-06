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

#ifndef OPENALPR_POSTPROCESS_H
#define OPENALPR_POSTPROCESS_H

#include "TRexpp.h"
#include "constants.h"
#include "utility.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <vector>
#include "config.h"


#define SKIP_CHAR "~"

namespace alpr
{

  struct Letter
  {
    std::string letter;
    int charposition;
    float totalscore;
    int occurences;
  };

  struct PPResult
  {
    std::string letters;
    float totalscore;
    bool matchesTemplate;
  };

  bool wordCompare( const PPResult &left, const PPResult &right );
  bool letterCompare( const Letter &left, const Letter &right );

  class RegexRule
  {
    public:
      RegexRule(std::string region, std::string pattern);

      bool match(std::string text);
      std::string filterSkips(std::string text);

    private:
      int numchars;
      TRexpp trexp;
      std::string original;
      std::string regex;
      std::string region;
      std::vector<int> skipPositions;
  };

  class PostProcess
  {
    public:
      PostProcess(Config* config);
      ~PostProcess();

      void addLetter(std::string letter, int charposition, float score);

      void clear();
      void analyze(std::string templateregion, int topn);

      std::string bestChars;
      bool matchesTemplate;

      const std::vector<PPResult> getResults();

    private:
      Config* config;
      //void getTopN();
      void findAllPermutations(std::vector<Letter> prevletters, int charPos, int substitutionsLeft);

      void insertLetter(std::string letter, int charPosition, float score);

      std::map<std::string, std::vector<RegexRule*> > rules;

      float calculateMaxConfidenceScore();

      std::vector<std::vector<Letter> > letters;
      std::vector<int> unknownCharPositions;

      std::vector<PPResult> allPossibilities;

      // Functions used to prune the list of letters (based on topn) to improve performance
      std::vector<int> getMaxDepth(int topn);
      int getPermutationCount(std::vector<int> depth);
      int getNextLeastDrop(std::vector<int> depth);
  };

}
#endif // OPENALPR_POSTPROCESS_H

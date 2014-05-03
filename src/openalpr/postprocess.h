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

using namespace std;

#define SKIP_CHAR '~'

struct Letter
{
  char letter;
  int charposition;
  float totalscore;
  int occurences;
};

struct PPResult
{
  string letters;
  float totalscore;
  bool matchesTemplate;
};

bool wordCompare( const PPResult &left, const PPResult &right );
bool letterCompare( const Letter &left, const Letter &right );

class RegexRule
{
  public:
    RegexRule(string region, string pattern);

    bool match(string text);
    string filterSkips(string text);

  private:
    int numchars;
    TRexpp trexp;
    string original;
    string regex;
    string region;
    vector<int> skipPositions;
};

class PostProcess
{
  public:
    PostProcess(Config* config);
    ~PostProcess();

    void addLetter(char letter, int charposition, float score);

    void clear();
    void analyze(string templateregion, int topn);

    string bestChars;
    bool matchesTemplate;

    const vector<PPResult> getResults();

  private:
    Config* config;
    //void getTopN();
    void findAllPermutations(vector<Letter> prevletters, int charPos, int substitutionsLeft);

    void insertLetter(char letter, int charPosition, float score);

    map<string, vector<RegexRule*> > rules;

    float calculateMaxConfidenceScore();

    vector<vector<Letter> > letters;
    vector<int> unknownCharPositions;

    vector<PPResult> allPossibilities;

    // Functions used to prune the list of letters (based on topn) to improve performance
    vector<int> getMaxDepth(int topn);
    int getPermutationCount(vector<int> depth);
    int getNextLeastDrop(vector<int> depth);
};

/*
class LetterScores
{
  public:
    LetterScores(int numCharPositions);

    void addScore(char letter, int charposition, float score);

    vector<char> getBestScore();
    float getConfidence();

  private:
    int numCharPositions;

    vector<char> letters;
    vector<int> charpositions;
    vector<float> scores;
};
*/
#endif // OPENALPR_POSTPROCESS_H

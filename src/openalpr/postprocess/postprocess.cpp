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

#include "postprocess.h"

#include <fstream>
#include <queue>
#include <utility>

using namespace std;

namespace alpr
{

  PostProcess::PostProcess(Config* config)
  {
    this->config = config;

    this->min_confidence = 0;
    this->skip_level = 0;
    
    stringstream filename;
    filename << config->getPostProcessRuntimeDir() << "/" << config->country << ".patterns";

    std::ifstream infile(filename.str().c_str());

    string region, pattern;
    while (infile >> region >> pattern)
    {
      RegexRule* rule = new RegexRule(region, pattern, config->postProcessRegexLetters, config->postProcessRegexNumbers);
      //cout << "REGION: " << region << " PATTERN: " << pattern << endl;

      if (rules.find(region) == rules.end())
      {
        vector<RegexRule*> newRule;
        newRule.push_back(rule);
        rules[region] = newRule;
      }
      else
      {
        vector<RegexRule*> oldRule = rules[region];
        oldRule.push_back(rule);
        rules[region] = oldRule;
      }
    }

  }

  PostProcess::~PostProcess()
  {
    // TODO: Delete all entries in rules vector
    map<string, vector<RegexRule*> >::iterator iter;

    for (iter = rules.begin(); iter != rules.end(); ++iter)
    {
      for (int i = 0; i < iter->second.size(); i++)
      {
        delete iter->second[i];
      }
    }
  }
  
  void PostProcess::setConfidenceThreshold(float min_confidence, float skip_level) {
    this->min_confidence = min_confidence;
    this->skip_level = skip_level;
  }


  void PostProcess::addLetter(string letter, int line_index, int charposition, float score)
  {
    if (score < min_confidence)
      return;

    insertLetter(letter, line_index, charposition, score);

    if (score < skip_level)
    {
      float adjustedScore = abs(skip_level - score) + min_confidence;
      insertLetter(SKIP_CHAR, line_index, charposition, adjustedScore );
    }

    //if (letter == '0')
    //{
    //  insertLetter('O', charposition, score - 0.5);
    //}
  }

  void PostProcess::insertLetter(string letter, int line_index, int charposition, float score)
  {
    score = score - min_confidence;

    int existingIndex = -1;
    if (letters.size() < charposition + 1)
    {
      for (int i = letters.size(); i < charposition + 1; i++)
      {
        vector<Letter> tmp;
        letters.push_back(tmp);
      }
    }

    for (int i = 0; i < letters[charposition].size(); i++)
    {
      if (letters[charposition][i].letter == letter &&
          letters[charposition][i].line_index == line_index &&
          letters[charposition][i].charposition == charposition)
      {
        existingIndex = i;
        break;
      }
    }

    if (existingIndex == -1)
    {
      Letter newLetter;
      newLetter.line_index = line_index;
      newLetter.charposition = charposition;
      newLetter.letter = letter;
      newLetter.occurrences = 1;
      newLetter.totalscore = score;
      letters[charposition].push_back(newLetter);
    }
    else
    {
      letters[charposition][existingIndex].occurrences = letters[charposition][existingIndex].occurrences + 1;
      letters[charposition][existingIndex].totalscore = letters[charposition][existingIndex].totalscore + score;
    }
  }

  void PostProcess::clear()
  {
    for (int i = 0; i < letters.size(); i++)
    {
      letters[i].clear();
    }
    letters.resize(0);

    unknownCharPositions.clear();
    unknownCharPositions.resize(0);
    allPossibilities.clear();
    allPossibilitiesLetters.clear();
    //allPossibilities.resize(0);

    bestChars = "";
    matchesTemplate = false;
  }

  void PostProcess::analyze(string templateregion, int topn)
  {
    timespec startTime;
    getTimeMonotonic(&startTime);

    // Get a list of missing positions
    for (int i = letters.size() -1; i >= 0; i--)
    {
      if (letters[i].size() == 0)
      {
        unknownCharPositions.push_back(i);
      }
    }

    if (letters.size() == 0)
      return;

    // Sort the letters as they are
    for (int i = 0; i < letters.size(); i++)
    {
      if (letters[i].size() > 0)
        std::stable_sort(letters[i].begin(), letters[i].end(), letterCompare);
    }

    if (this->config->debugPostProcess)
    {
      // Print all letters
      for (int i = 0; i < letters.size(); i++)
      {
        for (int j = 0; j < letters[i].size(); j++)
          cout << "PostProcess Line " << letters[i][j].line_index << " Letter: " << letters[i][j].charposition << " " << letters[i][j].letter << " -- score: " << letters[i][j].totalscore << " -- occurrences: " << letters[i][j].occurrences << endl;
      }
    }

    timespec permutationStartTime;
    getTimeMonotonic(&permutationStartTime);

    findAllPermutations(templateregion, topn);

    if (config->debugTiming)
    {
      timespec permutationEndTime;
      getTimeMonotonic(&permutationEndTime);
      cout << " -- PostProcess Permutation Time: " << diffclock(permutationStartTime, permutationEndTime) << "ms." << endl;
    }

    if (allPossibilities.size() > 0)
    {

      bestChars = allPossibilities[0].letters;
      for (int z = 0; z < allPossibilities.size(); z++)
      {
        if (allPossibilities[z].matchesTemplate)
        {
          bestChars = allPossibilities[z].letters;
          break;
        }
      }

      // Now adjust the confidence scores to a percentage value
      float maxPercentScore = calculateMaxConfidenceScore();
      float highestRelativeScore = (float) allPossibilities[0].totalscore;

      for (int i = 0; i < allPossibilities.size(); i++)
      {
        allPossibilities[i].totalscore = maxPercentScore * (allPossibilities[i].totalscore / highestRelativeScore);
      }
    }

    if (this->config->debugPostProcess)
    {
      // Print top words
      for (int i = 0; i < allPossibilities.size(); i++)
      {
        cout << "Top " << topn << " Possibilities: " << allPossibilities[i].letters << " :\t" << allPossibilities[i].totalscore;
        if (allPossibilities[i].letters == bestChars)
          cout << " <--- ";
        cout << endl;
      }
      cout << allPossibilities.size() << " total permutations" << endl;
    }

    if (config->debugTiming)
    {
      timespec endTime;
      getTimeMonotonic(&endTime);
      cout << "PostProcess Time: " << diffclock(startTime, endTime) << "ms." << endl;
    }

    if (this->config->debugPostProcess)
      cout << "PostProcess Analysis Complete: " << bestChars << " -- MATCH: " << matchesTemplate << endl;
  }

  bool PostProcess::regionIsValid(std::string templateregion)
  {
    return rules.find(templateregion) != rules.end();
  }
  
  float PostProcess::calculateMaxConfidenceScore()
  {
    // Take the best score for each char position and average it.

    float totalScore = 0;
    int numScores = 0;
    // Get a list of missing positions
    for (int i = 0; i < letters.size(); i++)
    {
      if (letters[i].size() > 0)
      {
        totalScore += (letters[i][0].totalscore / letters[i][0].occurrences) + min_confidence;
        numScores++;
      }
    }

    if (numScores == 0)
      return 0;

    return totalScore / ((float) numScores);
  }

  const vector<PPResult> PostProcess::getResults()
  {
    return this->allPossibilities;
  }

  struct PermutationCompare {
    bool operator() (pair<float,vector<int> > &a, pair<float,vector<int> > &b)
    {
      return (a.first < b.first);
    }
  };

  void PostProcess::findAllPermutations(string templateregion, int topn) {

    // use a priority queue to process permutations in highest scoring order
    priority_queue<pair<float,vector<int> >, vector<pair<float,vector<int> > >, PermutationCompare> permutations;
    set<float> permutationHashes;

    // push the first word onto the queue
    float totalscore = 0;
    for (int i=0; i<letters.size(); i++)
    {
      if (letters[i].size() > 0)
        totalscore += letters[i][0].totalscore;
    }
    vector<int> v(letters.size());
    permutations.push(make_pair(totalscore, v));

    int consecutiveNonMatches = 0;
    while (permutations.size() > 0)
    {
      // get the top permutation and analyze
      pair<float, vector<int> > topPermutation = permutations.top();
      if (analyzePermutation(topPermutation.second, templateregion, topn) == true)
        consecutiveNonMatches = 0;
      else
        consecutiveNonMatches += 1;
      permutations.pop();

      if (allPossibilities.size() >= topn || consecutiveNonMatches >= (topn*2))
        break;

      // add child permutations to queue
      for (int i=0; i<letters.size(); i++)
      {
        // no more permutations with this letter
        if (topPermutation.second[i]+1 >= letters[i].size())
          continue;

        pair<float, vector<int> > childPermutation = topPermutation;
        childPermutation.first -= letters[i][topPermutation.second[i]].totalscore - letters[i][topPermutation.second[i] + 1].totalscore;
        childPermutation.second[i] += 1;

        // ignore permutations that have already been visited (assume that score is a good hash for permutation)
        if (permutationHashes.end() != permutationHashes.find(childPermutation.first))
          continue;

        permutations.push(childPermutation);
        permutationHashes.insert(childPermutation.first);
      }
    }
  }

  bool PostProcess::analyzePermutation(vector<int> letterIndices, string templateregion, int topn)
  {
    PPResult possibility;
    possibility.letters = "";
    possibility.totalscore = 0;
    possibility.matchesTemplate = false;
    int plate_char_length = 0;

    int last_line = 0;
    for (int i = 0; i < letters.size(); i++)
    {
      if (letters[i].size() == 0)
        continue;

      Letter letter = letters[i][letterIndices[i]];

      // Add a "\n" on new lines
      if (letter.line_index != last_line)
      {
        possibility.letters = possibility.letters + "\n";
      }
      last_line = letter.line_index;
      
      if (letter.letter != SKIP_CHAR)
      {
        possibility.letters = possibility.letters + letter.letter;
        possibility.letter_details.push_back(letter);
        plate_char_length += 1;
      }
      possibility.totalscore = possibility.totalscore + letter.totalscore;
    }

    // ignore plates that don't fit the length requirements
    if (plate_char_length < config->postProcessMinCharacters ||
      plate_char_length > config->postProcessMaxCharacters)
      return false;

    // Apply templates
    if (templateregion != "")
    {
      vector<RegexRule*> regionRules = rules[templateregion];

      for (int i = 0; i < regionRules.size(); i++)
      {
        possibility.matchesTemplate = regionRules[i]->match(possibility.letters);
        if (possibility.matchesTemplate)
        {
          break;
        }
      }
    }

    // ignore duplicate words
    if (allPossibilitiesLetters.end() != allPossibilitiesLetters.find(possibility.letters))
      return false;

    // If mustMatchPattern is toggled in the config and a template is provided, 
    // only include this result if there is a pattern match
    if (!config->mustMatchPattern || templateregion.size() == 0 || 
        (config->mustMatchPattern && possibility.matchesTemplate))
    {
      allPossibilities.push_back(possibility);
      allPossibilitiesLetters.insert(possibility.letters);
      return true;
    }
    
    return false;
  }

  std::vector<string> PostProcess::getPatterns() {
    vector<string> v;
    for(map<string,std::vector<RegexRule*> >::iterator it = rules.begin(); it != rules.end(); ++it) {
      v.push_back(it->first);
    }
    
    return v;
  }

  bool letterCompare( const Letter &left, const Letter &right )
  {
    if (left.totalscore < right.totalscore)
      return false;
    return true;
  }

}
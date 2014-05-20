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

#include "postprocess.h"

using namespace std;

PostProcess::PostProcess(Config* config)
{
  this->config = config;

  stringstream filename;
  filename << config->getPostProcessRuntimeDir() << "/" << config->country << ".patterns";

  std::ifstream infile(filename.str().c_str());

  string region, pattern;
  while (infile >> region >> pattern)
  {
    RegexRule* rule = new RegexRule(region, pattern);
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

  //vector<RegexRule> test = rules["base"];
  //for (int i = 0; i < test.size(); i++)
  //  cout << "Rule: " << test[i].regex << endl;
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

void PostProcess::addLetter(char letter, int charposition, float score)
{
  if (score < config->postProcessMinConfidence)
    return;

  insertLetter(letter, charposition, score);

  if (score < config->postProcessConfidenceSkipLevel)
  {
    float adjustedScore = abs(config->postProcessConfidenceSkipLevel - score) + config->postProcessMinConfidence;
    insertLetter(SKIP_CHAR, charposition, adjustedScore );
  }

  //if (letter == '0')
  //{
  //  insertLetter('O', charposition, score - 0.5);
  //}
}

void PostProcess::insertLetter(char letter, int charposition, float score)
{
  score = score - config->postProcessMinConfidence;

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
        letters[charposition][i].charposition == charposition)
    {
      existingIndex = i;
      break;
    }
  }

  if (existingIndex == -1)
  {
    Letter newLetter;
    newLetter.charposition = charposition;
    newLetter.letter = letter;
    newLetter.occurences = 1;
    newLetter.totalscore = score;
    letters[charposition].push_back(newLetter);
  }
  else
  {
    letters[charposition][existingIndex].occurences = letters[charposition][existingIndex].occurences + 1;
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
  //allPossibilities.resize(0);

  bestChars = "";
  matchesTemplate = false;
}

void PostProcess::analyze(string templateregion, int topn)
{
  timespec startTime;
  getTime(&startTime);

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
      sort(letters[i].begin(), letters[i].end(), letterCompare);
  }

  if (this->config->debugPostProcess)
  {
    // Print all letters
    for (int i = 0; i < letters.size(); i++)
    {
      for (int j = 0; j < letters[i].size(); j++)
        cout << "PostProcess Letter: " << letters[i][j].charposition << " " << letters[i][j].letter << " -- score: " << letters[i][j].totalscore << " -- occurences: " << letters[i][j].occurences << endl;
    }
  }

  // Prune the letters based on the topN value.
  // If our topN value is 3, for example, we can get rid of a lot of low scoring letters
  // because it would be impossible for them to be a part of our topN results.
  vector<int> maxDepth = getMaxDepth(topn);

  for (int i = 0; i < letters.size(); i++)
  {
    for (int k = letters[i].size() - 1; k > maxDepth[i]; k--)
    {
      letters[i].erase(letters[i].begin() + k);
    }
  }

  //getTopN();
  vector<Letter> tmp;
  findAllPermutations(tmp, 0, config->postProcessMaxSubstitutions);

  timespec sortStartTime;
  getTime(&sortStartTime);

  int numelements = topn;
  if (allPossibilities.size() < topn)
    numelements = allPossibilities.size() - 1;

  partial_sort( allPossibilities.begin(), allPossibilities.begin() + numelements, allPossibilities.end(), wordCompare );

  if (config->debugTiming)
  {
    timespec sortEndTime;
    getTime(&sortEndTime);
    cout << " -- PostProcess Sort Time: " << diffclock(sortStartTime, sortEndTime) << "ms." << endl;
  }

  matchesTemplate = false;

  if (templateregion != "")
  {
    vector<RegexRule*> regionRules = rules[templateregion];

    for (int i = 0; i < allPossibilities.size(); i++)
    {
      for (int j = 0; j < regionRules.size(); j++)
      {
        allPossibilities[i].matchesTemplate = regionRules[j]->match(allPossibilities[i].letters);
        if (allPossibilities[i].matchesTemplate)
        {
          allPossibilities[i].letters = regionRules[j]->filterSkips(allPossibilities[i].letters);
          //bestChars = regionRules[j]->filterSkips(allPossibilities[i].letters);
          matchesTemplate = true;
          break;
        }
      }

      if (i >= topn - 1)
        break;
      //if (matchesTemplate || i >= TOP_N - 1)
      //break;
    }
  }

  if (matchesTemplate)
  {
    for (int z = 0; z < allPossibilities.size(); z++)
    {
      if (allPossibilities[z].matchesTemplate)
      {
        bestChars = allPossibilities[z].letters;
        break;
      }
    }
  }
  else
  {
    bestChars = allPossibilities[0].letters;
  }

  // Now adjust the confidence scores to a percentage value
  if (allPossibilities.size() > 0)
  {
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

      if (i >= topn - 1)
        break;
    }
    cout << allPossibilities.size() << " total permutations" << endl;
  }

  if (config->debugTiming)
  {
    timespec endTime;
    getTime(&endTime);
    cout << "PostProcess Time: " << diffclock(startTime, endTime) << "ms." << endl;
  }

  if (this->config->debugPostProcess)
    cout << "PostProcess Analysis Complete: " << bestChars << " -- MATCH: " << matchesTemplate << endl;
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
      totalScore += (letters[i][0].totalscore / letters[i][0].occurences) + config->postProcessMinConfidence;
      numScores++;
    }
  }

  if (numScores == 0)
    return 0;

  return totalScore / ((float) numScores);
}

// Finds the minimum number of letters to include in the recursive sorting algorithm.
// For example, if I have letters
//	A-200 B-100 C-100
//	X-99 Y-95   Z-90
//	Q-55        R-80
// And my topN value was 3, this would return:
// 0, 1, 1
// Which represents:
// 	A-200 B-100 C-100
//	      Y-95  Z-90
vector<int> PostProcess::getMaxDepth(int topn)
{
  vector<int> depth;
  for (int i = 0; i < letters.size(); i++)
    depth.push_back(0);

  int nextLeastDropCharPos = getNextLeastDrop(depth);
  while (nextLeastDropCharPos != -1)
  {
    if (getPermutationCount(depth) >= topn)
      break;

    depth[nextLeastDropCharPos] = depth[nextLeastDropCharPos] + 1;

    nextLeastDropCharPos = getNextLeastDrop(depth);
  }

  return depth;
}

int PostProcess::getPermutationCount(vector<int> depth)
{
  int permutationCount = 1;
  for (int i = 0; i < depth.size(); i++)
  {
    permutationCount *= (depth[i] + 1);
  }

  return permutationCount;
}

int PostProcess::getNextLeastDrop(vector<int> depth)
{
  int nextLeastDropCharPos = -1;
  float leastNextDrop = 99999999999;

  for (int i = 0; i < letters.size(); i++)
  {
    if (depth[i] + 1 >= letters[i].size())
      continue;

    float drop = letters[i][depth[i]].totalscore - letters[i][depth[i]+1].totalscore;

    if (drop < leastNextDrop)
    {
      nextLeastDropCharPos = i;
      leastNextDrop = drop;
    }
  }

  return nextLeastDropCharPos;
}

const vector<PPResult> PostProcess::getResults()
{
  return this->allPossibilities;
}

void PostProcess::findAllPermutations(vector<Letter> prevletters, int charPos, int substitutionsLeft)
{
  if (substitutionsLeft < 0)
    return;

  // Add my letter to the chain and recurse
  for (int i = 0; i < letters[charPos].size(); i++)
  {
    if (charPos == letters.size() - 1)
    {
      // Last letter, add the word
      PPResult possibility;
      possibility.letters = "";
      possibility.totalscore = 0;
      possibility.matchesTemplate = false;
      for (int z = 0; z < prevletters.size(); z++)
      {
        if (prevletters[z].letter != SKIP_CHAR)
          possibility.letters = possibility.letters + prevletters[z].letter;
        possibility.totalscore = possibility.totalscore + prevletters[z].totalscore;
      }

      if (letters[charPos][i].letter != SKIP_CHAR)
        possibility.letters = possibility.letters + letters[charPos][i].letter;
      possibility.totalscore = possibility.totalscore +letters[charPos][i].totalscore;

      allPossibilities.push_back(possibility);
    }
    else
    {
      prevletters.push_back(letters[charPos][i]);

      float scorePercentDiff = abs( letters[charPos][0].totalscore - letters[charPos][i].totalscore ) / letters[charPos][0].totalscore;
      if (i != 0 && letters[charPos][i].letter != SKIP_CHAR && scorePercentDiff > 0.10f )
        findAllPermutations(prevletters, charPos + 1, substitutionsLeft - 1);
      else
        findAllPermutations(prevletters, charPos + 1, substitutionsLeft);

      prevletters.pop_back();
    }
  }

  if (letters[charPos].size() == 0)
  {
    // No letters for this char position...
    // Just pass it along
    findAllPermutations(prevletters, charPos + 1, substitutionsLeft);
  }
}

bool wordCompare( const PPResult &left, const PPResult &right )
{
  if (left.totalscore < right.totalscore)
    return false;
  return true;
}

bool letterCompare( const Letter &left, const Letter &right )
{
  if (left.totalscore < right.totalscore)
    return false;
  return true;
}

RegexRule::RegexRule(string region, string pattern)
{
  this->original = pattern;
  this->region = region;

  numchars = 0;
  for (int i = 0; i < pattern.size(); i++)
  {
    if (pattern.at(i) == '[')
    {
      while (pattern.at(i) != ']' )
      {
        this->regex = this->regex + pattern.at(i);
        i++;
      }
      this->regex = this->regex + ']';
    }
    else if (pattern.at(i) == '?')
    {
      this->regex = this->regex + '.';
      this->skipPositions.push_back(numchars);
    }
    else if (pattern.at(i) == '@')
    {
      this->regex = this->regex + "\\a";
    }
    else if (pattern.at(i) == '#')
    {
      this->regex = this->regex + "\\d";
    }

    numchars++;
  }

  trexp.Compile(this->regex.c_str());

  //cout << "AA " << this->region << ": " << original << " regex: " << regex << endl;
  //for (int z = 0; z < this->skipPositions.size(); z++)
  //  cout << "AA Skip position: " << skipPositions[z] << endl;
}

bool RegexRule::match(string text)
{
  if (text.length() != numchars)
    return false;

  return trexp.Match(text.c_str());
}

string RegexRule::filterSkips(string text)
{
  string response = "";
  for (int i = 0; i < text.size(); i++)
  {
    bool skip = false;
    for (int j = 0; j < skipPositions.size(); j++)
    {
      if (skipPositions[j] == i)
      {
        skip = true;
        break;
      }
    }

    if (skip == false)
      response = response + text[i];
  }

  return response;
}

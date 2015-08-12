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

#include <sstream>

#include "regexrule.h"

using namespace std;

tthread::mutex regexrule_mutex_m;

namespace alpr
{
   
  RegexRule::RegexRule(string region, string pattern, std::string letters_regex, std::string numbers_regex)
  //: re2_regex("")
  {   
    this->original = pattern;
    this->region = region;
    this->regex = "";

    this->valid = false;
    string::iterator end_it = utf8::find_invalid(pattern.begin(), pattern.end());
    if (end_it != pattern.end()) {
      cerr << "Invalid UTF-8 encoding detected " << endl;
      return;
    }
    
    std::stringstream regexval;
    string::iterator utf_iterator = pattern.begin();
    numchars = 0;
    while (utf_iterator < pattern.end())
    {
      int cp = utf8::next(utf_iterator, pattern.end());
      
      string utf_character = utf8chr(cp);
      
      
      if (utf_character == "[")
      {
        regexval << "[";
        
        while (utf_character != "]" )
        {
          if (utf_iterator >= pattern.end())
            break; // Invalid regex, don't bother processing
          int cp = utf8::next(utf_iterator, pattern.end());

          utf_character = utf8chr(cp);
          regexval << utf_character;
        }
        
      }
      else if (utf_character == "\\")
      {
        // Don't add "\" characters to our character count
        regexval << utf_character;
        continue;
      }
      else if (utf_character == "?")
      {
        regexval << ".";
        this->skipPositions.push_back(numchars);
      }
      else if (utf_character == "@")
      {
        regexval << letters_regex;
      }
      else if (utf_character == "#")
      {
        regexval << numbers_regex;
      }
      else if ((utf_character == "*") || (utf_character == "+"))
      {
        cerr << "Regex with wildcards (* or +) not supported" << endl;
      }
      else
      {
        regexval << utf_character;
      }

      numchars++;
    }

    this->regex = regexval.str();

    re2_regex = new re2::RE2(this->regex);
    

    
    if (!re2_regex->ok()) {
      cerr << "Unable to load regex: " << pattern << endl;
    }
    else
    {
      this->valid = true;
    }
  }
  
  
  RegexRule::~RegexRule()
  {
    delete re2_regex;
  }

  bool RegexRule::match(string text)
  {
    if (!this->valid)
      return false;
    
    string::iterator end_it = utf8::find_invalid(text.begin(), text.end());
    if (end_it != text.end()) {
      cerr << "Invalid UTF-8 encoding detected " << endl;
      return false;
    }
   
    int text_char_length = utf8::distance(text.begin(), text.end());

    if (text_char_length != numchars)
      return false;

    bool match = re2::RE2::FullMatch(text, *re2_regex);

    return match;
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

}


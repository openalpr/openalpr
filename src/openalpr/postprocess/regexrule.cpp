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

#include "regexrule.h"

using namespace std;

tthread::mutex regexrule_mutex_m;

namespace alpr
{
   
  RegexRule::RegexRule(string region, string pattern)
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
    
    string::iterator utf_iterator = pattern.begin();
    numchars = 0;
    while (utf_iterator < pattern.end())
    {
      int cp = utf8::next(utf_iterator, pattern.end());
      
      string utf_character = utf8chr(cp);
      
      if (utf_character == "[")
      {
        this->regex = this->regex + "[";
        
        while (utf_character != "]" )
        {
          if (utf_iterator >= pattern.end())
            break; // Invalid regex, don't bother processing
          int cp = utf8::next(utf_iterator, pattern.end());

          utf_character = utf8chr(cp);
          this->regex = this->regex + utf_character;
        }
        
      }
      else if (utf_character == "\\")
      {
        // Don't add "\" characters to our character count
        this->regex = this->regex + utf_character;
        continue;
      }
      else if (utf_character == "?")
      {
        this->regex = this->regex + '.';
        this->skipPositions.push_back(numchars);
      }
      else if (utf_character == "@")
      {
        this->regex = this->regex + "\\p{Alpha}";
      }
      else if (utf_character == "#")
      {
        this->regex = this->regex + "\\p{Digit}";
      }
      else if ((utf_character == "*") || (utf_character == "+"))
      {
        cerr << "Regex with wildcards (* or +) not supported" << endl;
      }
      else
      {
        this->regex = this->regex + utf_character;
      }

      numchars++;
    }

    // Onigurama is not thread safe when compiling regex.  Using a mutex to ensure that
    // we don't crash
    regexrule_mutex_m.lock();
    UChar* cstr_pattern = (UChar* )this->regex.c_str();
    OnigErrorInfo einfo;

    int r = onig_new(&onig_regex, cstr_pattern, cstr_pattern + strlen((char* )cstr_pattern),
      ONIG_OPTION_DEFAULT, ONIG_ENCODING_UTF8, ONIG_SYNTAX_DEFAULT, &einfo);

    regexrule_mutex_m.unlock(); 
    
    if (r != ONIG_NORMAL) {
      //char s[ONIG_MAX_ERROR_MESSAGE_LEN];
      //onig_error_code_to_str(s, r, &einfo);
      cerr << "Unable to load regex: " << pattern << endl;
    }
    else
    {
      this->valid = true;
    }
  }
  
  
  RegexRule::~RegexRule()
  {
    onig_free(onig_regex);
    onig_end();
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

    OnigRegion *region = onig_region_new();
    unsigned char *start, *end;
    UChar* cstr_text = (UChar* )text.c_str();
    end   = cstr_text + strlen((char* )cstr_text);
    start = cstr_text;
    
    int match = onig_match(onig_regex, cstr_text, end, start, region, ONIG_OPTION_NONE);
    
    onig_region_free(region, 1);
    
    return match == text.length();
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


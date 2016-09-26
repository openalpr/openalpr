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

#include "config_helper.h"

#include <clocale>
#include <iostream>

using namespace std;

namespace alpr
{
  
  float getFloat(CSimpleIniA* ini, string section, string key, float defaultValue)
  {
    if (ini == NULL)
      return defaultValue;
    
    const char * pszValue = ini->GetValue(section.c_str(), key.c_str(), NULL /*default*/);
    if (pszValue == NULL)
    {
      return defaultValue;
    }

    char * locale = std::setlocale(LC_ALL, NULL);
    setlocale(LC_NUMERIC, "C");

    float val = atof(pszValue);

    std::setlocale(LC_NUMERIC, locale);

    return val;
  }
  
  std::vector<float> getAllFloats(CSimpleIniA* ini, string section, string key)
  {
    CSimpleIniA::TNamesDepend values;
    
    ini->GetAllValues(section.c_str(), key.c_str(), values);
 
    // sort the values into the original load order
    values.sort(CSimpleIniA::Entry::LoadOrder());

    std::vector<float> response;

    char * locale = std::setlocale(LC_ALL, NULL);
    std::setlocale(LC_NUMERIC, "C");

      // output all of the items
    CSimpleIniA::TNamesDepend::const_iterator i;
    for (i = values.begin(); i != values.end(); ++i) {
      response.push_back(atof(i->pItem));
    }

    std::setlocale(LC_NUMERIC, locale);

    return response;
  }
  
  int getInt(CSimpleIniA* ini, string section, string key, int defaultValue)
  {
    if (ini == NULL)
      return defaultValue;
    
    const char * pszValue = ini->GetValue(section.c_str(), key.c_str(), NULL /*default*/);
    if (pszValue == NULL)
    {
      return defaultValue;
    }

    int val = atoi(pszValue);
    return val;
  }
  bool getBoolean(CSimpleIniA* ini, string section, string key, bool defaultValue)
  {
    if (ini == NULL)
      return defaultValue;
    
    const char * pszValue = ini->GetValue(section.c_str(), key.c_str(), NULL /*default*/);
    if (pszValue == NULL)
    {
      return defaultValue;
    }

    int val = atoi(pszValue);
    return val != 0;
  }
  string getString(CSimpleIniA* ini, string section, string key, string defaultValue)
  {
    if (ini == NULL)
      return defaultValue;
    
    const char * pszValue = ini->GetValue(section.c_str(), key.c_str(), NULL /*default*/);
    if (pszValue == NULL)
    {
      return defaultValue;
    }

    string val = string(pszValue);
    return val;
  }
  
  bool hasValue(CSimpleIniA* ini, std::string section, std::string key)
  {
    const char * pszValue = ini->GetValue(section.c_str(), key.c_str(), NULL /*default*/);
    
    return pszValue != NULL;
  }

  int getInt(CSimpleIniA* ini, CSimpleIniA* defaultIni, std::string section, std::string key, int defaultValue)
  {
    if (ini != NULL && hasValue(ini, section, key))
      return getInt(ini, section, key, defaultValue);
    
    if (defaultIni != NULL && hasValue(defaultIni, section, key))
      return getInt(defaultIni, section, key, defaultValue);
    
    std::cerr << "Missing config value for " << key << std::endl;
    
    return defaultValue;
  }
  float getFloat(CSimpleIniA* ini, CSimpleIniA* defaultIni, std::string section, std::string key, float defaultValue)
  {
    if (ini != NULL && hasValue(ini, section, key))
      return getFloat(ini, section, key, defaultValue);
    
    if (defaultIni != NULL && hasValue(defaultIni, section, key))
      return getFloat(defaultIni, section, key, defaultValue);
    
    std::cerr << "Missing config value for " << key << std::endl;
    
    return defaultValue;
  }
  std::string getString(CSimpleIniA* ini, CSimpleIniA* defaultIni, std::string section, std::string key, std::string defaultValue)
  {
     if (ini != NULL && hasValue(ini, section, key))
      return getString(ini, section, key, defaultValue);
    
    if (defaultIni != NULL && hasValue(defaultIni, section, key))
      return getString(defaultIni, section, key, defaultValue);
    
    std::cerr << "Missing config value for " << key << std::endl;
    return defaultValue;
  }
  bool getBoolean(CSimpleIniA* ini, CSimpleIniA* defaultIni, std::string section, std::string key, bool defaultValue)
  {
     if (ini != NULL && hasValue(ini, section, key))
      return getBoolean(ini, section, key, defaultValue);
    
    if (defaultIni != NULL && hasValue(defaultIni, section, key))
      return getBoolean(defaultIni, section, key, defaultValue);
    
    std::cerr << "Missing config value for " << key << std::endl;
    return defaultValue;
  }
}
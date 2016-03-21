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

#ifndef OPENALPR_CONFIG_HELPER_H
#define	OPENALPR_CONFIG_HELPER_H

#include "simpleini/simpleini.h"
#include <string>
#include <vector>

namespace alpr
{
  
  bool hasValue(CSimpleIniA* ini, std::string section, std::string key);

  int getInt(CSimpleIniA* ini, std::string section, std::string key, int defaultValue);
  float getFloat(CSimpleIniA* ini, std::string section, std::string key, float defaultValue);
  std::string getString(CSimpleIniA* ini, std::string section, std::string key, std::string defaultValue);
  bool getBoolean(CSimpleIniA* ini, std::string section, std::string key, bool defaultValue);
  std::vector<float> getAllFloats(CSimpleIniA* ini, std::string section, std::string key);

  // Checks the ini objects in the order they are placed in the vector
  // e.g., second ini object overrides the first if they both have the value
  int getInt(CSimpleIniA* ini, CSimpleIniA* defaultIni, std::string section, std::string key, int defaultValue);
  float getFloat(CSimpleIniA* ini, CSimpleIniA* defaultIni, std::string section, std::string key, float defaultValue);
  std::string getString(CSimpleIniA* ini, CSimpleIniA* defaultIni, std::string section, std::string key, std::string defaultValue);
  bool getBoolean(CSimpleIniA* ini, CSimpleIniA* defaultIni, std::string section, std::string key, bool defaultValue);
  
}

#endif	/* CONFIG_HELPER_H */


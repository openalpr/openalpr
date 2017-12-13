/*
 * Copyright (c) 2016 OpenALPR Technology, Inc.
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

#include "daemonconfig.h"
#include "config_helper.h"

using namespace alpr;

DaemonConfig::DaemonConfig(std::string config_file, std::string config_defaults_file) {
  CSimpleIniA ini;
  ini.SetMultiKey();
  ini.LoadFile(config_file.c_str());
  
  CSimpleIniA defaultIni;
  defaultIni.SetMultiKey();
  defaultIni.LoadFile(config_defaults_file.c_str());
  
  // Stream will only be in the user override config, never in the defaults
  CSimpleIniA::TNamesDepend values;
  ini.GetAllValues("daemon", "stream", values);

  // sort the values into the original load order
  values.sort(CSimpleIniA::Entry::LoadOrder());

  // output all of the items
  CSimpleIniA::TNamesDepend::const_iterator i;
  for (i = values.begin(); i != values.end(); ++i) { 
      stream_urls.push_back(i->pItem);
  }

  country = getString(&ini, &defaultIni, "daemon", "country", "us");
  topn = getInt(&ini, &defaultIni, "daemon", "topn", 20);
  analysis_threads = getInt(&ini, &defaultIni, "daemon", "analysis_threads", 1);
  
  storePlates = getBoolean(&ini, &defaultIni, "daemon", "store_plates", false);
  imageFolder = getString(&ini, &defaultIni, "daemon", "store_plates_location", "/tmp/");
  uploadData = getBoolean(&ini, &defaultIni, "daemon", "upload_data", false);
  upload_url = getString(&ini, &defaultIni, "daemon", "upload_address", "");
  company_id = getString(&ini, &defaultIni, "daemon", "company_id", "");
  site_id = getString(&ini, &defaultIni, "daemon", "site_id", "");
  pattern = getString(&ini, &defaultIni, "daemon", "pattern", "");
}

DaemonConfig::~DaemonConfig() {
}


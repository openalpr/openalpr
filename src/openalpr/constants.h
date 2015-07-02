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
#ifndef OPENALPR_CONSTANTS_H
#define OPENALPR_CONSTANTS_H



#define RUNTIME_DIR 		"/runtime_data"
#define CONFIG_FILE 		"/openalpr.conf"
#define KEYPOINTS_DIR		"/keypoints"
#define CASCADE_DIR		"/region/"
#define POSTPROCESS_DIR		"/postprocess"

#ifndef DEFAULT_CONFIG_FILE
  #define DEFAULT_CONFIG_FILE 	"/etc/openalpr/openalpr.conf"
#endif

#define ENV_VARIABLE_CONFIG_FILE "OPENALPR_CONFIG_FILE"

#endif // OPENALPR_CONSTANTS_H
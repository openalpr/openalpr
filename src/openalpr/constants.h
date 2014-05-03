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
#ifndef OPENALPR_CONSTANTS_H
#define OPENALPR_CONSTANTS_H


#define OPENALPR_MAJOR_VERSION 1
#define OPENALPR_MINOR_VERSION 1
#define OPENALPR_PATCH_VERSION 0

#define CONFIG_FILE 		"/openalpr.conf"
#define KEYPOINTS_DIR		"/keypoints"
#define CASCADE_DIR		"/region/"
#define POSTPROCESS_DIR		"/postprocess"

#ifndef DEFAULT_RUNTIME_DIR
#define DEFAULT_RUNTIME_DIR 	"/default_runtime_data_dir/"
#endif

#define ENV_VARIABLE_RUNTIME_DIR "OPENALPR_RUNTIME_DIR"

#endif // OPENALPR_CONSTANTS_H
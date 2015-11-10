/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include "ge-file-util.h"
#include <string.h>

char* ge_file_dir_get(const char path[])
{
	char *p = NULL;
	char buf[256] = {0,};
	struct stat info = {0,};

	if (stat(path, &info) == 0) {
		strncpy(buf, path, 256);
		buf[256 - 1] = 0;
		p = dirname(buf);
		return strdup(p);
	} else {
		return "";
	}
}

int ge_file_exists(const char *path)
{
	struct stat info = {0,};

	if (stat(path, &info) == 0) {
		return 1;
	} else {
		return 0;
	}
}

int ge_file_size(const char *filename)
{
	struct stat info = {0,};
	if (stat(filename, &info) == 0) {
		if (!S_ISDIR(info.st_mode)) {
			return info.st_size;
		}
	}

	return 0;
}

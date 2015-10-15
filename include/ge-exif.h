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

#ifdef _USE_ROTATE_BG_GE

#ifndef _GE_EXIF_H_
#define _GE_EXIF_H_

#include <stdbool.h>
#include <stdio.h>

#ifdef _cplusplus
extern "C"
{
#endif

/*
Orientation - angle
1 - 0
3 - 180
6 - 90
8 - 270
*/
typedef enum _ge_orientation_rot_t ge_orientation_rot_e;

enum _ge_orientation_rot_t {
	GE_ORIENTATION_ROT_ERR = 0,
	GE_ORIENTATION_ROT_0 = 1,
	GE_ORIENTATION_ROT_180 = 3,
	GE_ORIENTATION_ROT_90 = 6,
	GE_ORIENTATION_ROT_270 = 8,
};

int _ge_exif_get_orientation(char *file_path, unsigned int *orientation);

#ifdef _cplusplus
}
#endif

#endif //_GE_EXIF_H_

#endif //_USE_ROTATE_BG_GE


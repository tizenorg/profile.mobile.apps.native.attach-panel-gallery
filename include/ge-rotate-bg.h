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

#ifndef _GE_ROTATE_BG_H_
#define _GE_ROTATE_BG_H_

#include "gallery-efl.h"

Evas_Object *_ge_rotate_bg_add(Evas_Object *parent, bool b_preload);
int _ge_rotate_bg_set_file(Evas_Object *bg, const char *file, int w, int h);
int _ge_rotate_bg_rotate_image(Evas_Object *bg, unsigned int orient);
int _ge_rotate_bg_add_image(Evas_Object *bg, int w, int h);
int _ge_rotate_bg_set_image_file(Evas_Object *bg, const char *file);

#endif //_GE_ROTATE_BG_H_

#endif //_USE_ROTATE_BG_GE

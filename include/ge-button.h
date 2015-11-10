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

#ifndef _GE_BUTTON_H_
#define _GE_BUTTON_H_

#include "gallery-efl.h"

/**< Evas smart objects' "smart callback" function signature */
typedef void (*But_Smart_Cb)(void *data, Evas_Object *obj, void *event_info);

Evas_Object *_ge_but_create_but(Evas_Object *parent, Elm_Theme *theme,
                                const char *icon, const char *text,
                                const char *style, But_Smart_Cb cb_func,
                                const void *data);
Evas_Object *_ge_but_create_image_but(Evas_Object *parent, const char *icon,
                                      const char *text, const char *style,
                                      Evas_Smart_Cb cb_func, void *data,
                                      Eina_Bool flag_propagate);

#endif /* _GE_BUTTON_H_ */


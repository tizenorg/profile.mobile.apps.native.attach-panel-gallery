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


#ifndef __GE_MAIN_VIEW_H__
#define __GE_MAIN_VIEW_H__

#include "ge-ugdata.h"

typedef enum _ge_ctrl_tab_mode {
	GE_CTRL_TAB_ALBUMS,
	GE_CTRL_TAB_CNT,
} ge_ctrl_tab_mode;

int _ge_main_create_view(ge_ugdata *ugd);

#endif //__GE_MAIN_VIEW_H__


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

#include "ge-lang.h"
#include "ge-ui-util.h"
#include "ge-util.h"
#include "ge-data.h"
#include "ge-gridview.h"
#include "ge-main-view.h"

int _ge_lang_update(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	int view_m = _ge_get_view_mode(ugd);
	ge_dbg("view: %d", view_m);

	if (view_m == GE_VIEW_THUMBS || view_m == GE_VIEW_THUMBS_EDIT)
		_ge_grid_update_lang(ugd);
	return 0;
}


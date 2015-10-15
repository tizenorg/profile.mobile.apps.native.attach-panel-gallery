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

#include "ge-ext-exec.h"
#include "ge-debug.h"
#include "ge-data.h"
#include "ge-util.h"
#include "ge-gridview.h"

#define GE_EXT_FILETRANSFER "com.samsung.rcs-im"
#define GE_EXT_CMD_ARGS_LEN 20
#define GE_EXT_LAUNCH_TYPE "launch-type"
#define GE_EXT_LAUNCH_FT_WITH_CONTACT "ft-with-contact"
#define GE_EXT_FT_FILE "file"
#define GE_EXT_FT_CONTACT "contact"

static int __ge_ext_compose_cmd(ge_ugdata *ugd, GeExtAppType type,
				app_control_h service, char **pkg_name, char *paths,
				int count)
{
	ge_dbg("type:%d", type);
	GE_CHECK_VAL(pkg_name, LAUNCH_FAIL);
	GE_CHECK_VAL(service, LAUNCH_FAIL);
	GE_CHECK_VAL(ugd, LAUNCH_FAIL);

	switch (type) {
	case FILETRANSFER: {
		app_control_add_extra_data(service, GE_EXT_LAUNCH_TYPE,
				       GE_EXT_LAUNCH_FT_WITH_CONTACT);
		app_control_add_extra_data(service, GE_EXT_FT_FILE, paths);

		char index_arg[GE_EXT_CMD_ARGS_LEN] = { 0, };
		snprintf(index_arg, sizeof(index_arg), "%d",
			 ugd->file_select_contact_id);
		app_control_add_extra_data(service, GE_EXT_FT_CONTACT, index_arg);
		*pkg_name = GE_EXT_FILETRANSFER;
		break;
	}
	default:
		*pkg_name = NULL;
		return LAUNCH_FAIL;
	}

	return LAUNCH_SUCCESS;
}

int _ge_ext_load_app(ge_ugdata *ugd, GeExtAppType type, char *paths, int count)
{
	GE_CHECK_VAL(ugd, LAUNCH_FAIL);
	app_control_h service = NULL;
	char *pkg_name = NULL;

	app_control_create(&service);
	GE_CHECK_VAL(service, LAUNCH_FAIL);

	__ge_ext_compose_cmd(ugd, type, service, &pkg_name, paths, count);
	if (pkg_name == NULL) {
		ge_dbgE("pkg_name is NULL!");
		app_control_destroy(service);
		return LAUNCH_FAIL;
	}

 	int ret = 0;
	app_control_set_operation(service, APP_CONTROL_OPERATION_DEFAULT);
	if (APP_CONTROL_ERROR_NONE != app_control_set_app_id(service, pkg_name)) {
		ge_dbgE("app_control_set_app_id failed");
	}
	ret = app_control_send_launch_request(service, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		app_control_destroy(service);
		ge_dbgE("app_control_send_launch_request failed[%d]!", ret);
		return LAUNCH_FAIL;
	}

	ret = app_control_destroy(service);
	if (ret != APP_CONTROL_ERROR_NONE)
		return LAUNCH_FAIL;
	else
		return LAUNCH_SUCCESS;
}


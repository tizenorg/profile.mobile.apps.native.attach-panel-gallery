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

#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <storage.h>
#include "ge-albums.h"
#include "ge-gridview.h"
#include "ge-data.h"
#include "ge-util.h"
#include "ge-ui-util.h"
#include "ge-icon.h"
#include "ge-ext-exec.h"

#define GE_AUL_PREFIX "gallery:"
/* For local album */
#define GE_AUL_ALBUM "album"
/* For IMAGE */
#define GE_AUL_IMAGEVIEWER "imageviewer"
#define GE_TIME_MSEC_PER_SEC 1000
#define GE_TIME_SEC_PER_MIN 60
#define GE_TIME_MIN_PER_HOUR 60
#define GE_TIME_SEC_PER_HOUR (GE_TIME_MIN_PER_HOUR * GE_TIME_SEC_PER_MIN)

#define GE_PERIOD_LIVEBOX 30.0

static int __externalStorageId = 0;

bool getSupportedStorages_cb(int storageId, storage_type_e type, storage_state_e state, const char *path, void *userData)
{
	ge_dbg("");

	if (type == STORAGE_TYPE_EXTERNAL) {
		__externalStorageId = storageId;
		return false;
	}

	return true;
}

static void _ge_db_update_sdcard_info(int storage_id, storage_state_e state, void *user_data)
{
	ge_dbgW("Storage state changed!");
	GE_CHECK(user_data);

	int error = storage_foreach_device_supported(getSupportedStorages_cb, NULL);
	if (error == STORAGE_ERROR_NONE) {
		storage_state_e mmc_state;
		int ret = storage_get_state(__externalStorageId, &mmc_state);
		if (ret != STORAGE_ERROR_NONE) {
			ge_dbgE("storage_get_state failed!");
			return;
		}

		/* Update view */
		if (mmc_state == STORAGE_STATE_MOUNTED) {
			ge_dbg("mmc_state[%d]: STORAGE_STATE_MOUNTED", mmc_state);
			ge_update_view((ge_ugdata*)user_data, GE_UPDATE_MMC_ADDED);
		} else if (mmc_state == STORAGE_STATE_REMOVED) {
			ge_dbg("mmc_state[%d]: STORAGE_STATE_REMOVED", mmc_state);
			ge_update_view((ge_ugdata*)user_data, GE_UPDATE_MMC_REMOVED);
		} else if (mmc_state == STORAGE_STATE_UNMOUNTABLE) {
			ge_dbg("mmc_state[%d]: STORAGE_STATE_UNMOUNTABLE", mmc_state);
			ge_update_view((ge_ugdata*)user_data, GE_UPDATE_MMC_REMOVED);
		}
	}
}

Eina_Bool ge_update_view(ge_ugdata* ugd, int mode)
{
	ge_dbg("mode: %d", mode);
	GE_CHECK_FALSE(ugd);
	int view_mode = _ge_get_view_mode(ugd);
	ge_dbg("view_mode: %d", view_mode);
	int ret = -1;

	ret = ge_update_gengrid(ugd);

	if (ret != 0) {
		return EINA_FALSE;
	}
	return EINA_TRUE;
}

int ge_reg_db_update_noti(ge_ugdata* ugd)
{
	ge_dbg("");
	GE_CHECK_VAL(ugd, -1);
	int error_code = 0;

	error_code = storage_set_state_changed_cb(__externalStorageId, _ge_db_update_sdcard_info, ugd);
	if (error_code != STORAGE_ERROR_NONE) {
		ge_dbgE("storage_set_state_changed_cb failed!");
	}
	return 0;
}

int ge_dereg_db_update_noti(void)
{
	ge_dbg("");
	int error_code = -1;

	error_code = storage_unset_state_changed_cb(__externalStorageId, _ge_db_update_sdcard_info);
	if (error_code != STORAGE_ERROR_NONE) {
		ge_dbgE("storage_set_state_changed_cb failed!");
	}

	return 0;
}

bool _ge_is_image_valid(void *data, char *filepath)
{
	GE_CHECK_FALSE(data);
	ge_ugdata *ugd = (ge_ugdata *)data;
	GE_CHECK_FALSE(filepath);

	Evas_Object *image = NULL;
	int width = 0;
	int height = 0;
	Evas *evas = NULL;

	evas = evas_object_evas_get(ugd->win);
	GE_CHECK_FALSE(evas);

	image = evas_object_image_add(evas);
	GE_CHECK_FALSE(image);

	evas_object_image_filled_set(image, 0);
	evas_object_image_load_scale_down_set(image, 0);
	evas_object_image_file_set(image, filepath, NULL);
	evas_object_image_size_get(image, &width, &height);
	if (image) {
		evas_object_del(image);
		image = NULL;
	}

	if (width <= 0 || height <= 0) {
		ge_sdbg("Cannot load file : %s", filepath);
		return false;
	}

	return true;
}

/* Change int to char * of video duration, caller need to free the allocated memory */
char *_ge_get_duration_string(unsigned int v_dur)
{
	char *dur_str = calloc(1, GE_FILE_PATH_LEN_MAX);
	GE_CHECK_NULL(dur_str);
	if (v_dur > 0) {
		int duration = floor(v_dur / GE_TIME_MSEC_PER_SEC);
		int dur_hr = 0;
		int dur_min = 0;
		int dur_sec = 0;
		int tmp = 0;

		if (duration >= GE_TIME_SEC_PER_HOUR) {
			dur_sec = duration % GE_TIME_SEC_PER_MIN;
			tmp = floor(duration / GE_TIME_SEC_PER_MIN);
			dur_min = tmp % GE_TIME_MIN_PER_HOUR;
			dur_hr = floor(tmp / GE_TIME_MIN_PER_HOUR);
		} else if (duration >= GE_TIME_SEC_PER_MIN) {
			dur_hr = 0;
			dur_min = floor(duration / GE_TIME_SEC_PER_MIN);
			dur_sec = duration % GE_TIME_SEC_PER_MIN;
		} else {
			dur_hr = 0;
			dur_min = 0;
			dur_sec = duration % GE_TIME_SEC_PER_MIN;
		}

		snprintf(dur_str, GE_FILE_PATH_LEN_MAX, "%02d:%02d:%02d",
		         dur_hr, dur_min, dur_sec);
	} else {
		snprintf(dur_str, GE_FILE_PATH_LEN_MAX, "00:00:00");
	}
	dur_str[strlen(dur_str)] = '\0';
	return dur_str;
}

ge_view_mode _ge_get_view_mode(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	return ugd->view_mode;
}

int _ge_set_view_mode(ge_ugdata *ugd, ge_view_mode mode)
{
	GE_CHECK_VAL(ugd, -1);
	ugd->view_mode = mode;
	return 0;
}

double _ge_get_win_factor(Evas_Object *win, bool b_hide_indicator, int *width,
                          int *height)
{
	if (win == NULL) {
		ge_dbgE("Invalid window!");
		return 1.0f;
	}

	double factor = 1.0f;
	int win_x = 0;
	int win_y = 0;
	int win_w = 0;
	int win_h = 0;

	elm_win_screen_size_get(win, &win_x, &win_y, &win_w, &win_h);
	ge_dbg("Window size: %dx%d, %dx%d", win_x, win_y, win_w, win_h);
	int win_p_h = win_w > win_h ? win_w : win_h;
	double scale = elm_config_scale_get();
	ge_dbg("scale: %f", scale);
	int fixed_h = GE_FIXED_HEIGHT;
	if (b_hide_indicator) {
		fixed_h = GE_FIXED_HEIGHT_HIDE;
	}
	if (win_p_h < GE_BASE_WIN_HEIGHT) {
		factor = (double)(win_p_h - (int)(fixed_h * scale)) / (double)(GE_BASE_WIN_HEIGHT - fixed_h);
	} else if (scale > factor) {
		factor = (GE_BASE_WIN_HEIGHT - fixed_h * scale) / (GE_BASE_WIN_HEIGHT - fixed_h);
	} else if (scale < factor) {
		factor = (GE_BASE_WIN_HEIGHT - fixed_h) / (GE_BASE_WIN_HEIGHT - fixed_h * scale);
	} else {
		factor = scale;
	}
	ge_dbg("factor: %f", factor);
	if (width) {
		*width = win_w;
	}
	if (height) {
		*height = win_h;
	}
	return factor;
}

int _ge_send_result(ge_ugdata *ugd)
{
	GE_CHECK_VAL(ugd, -1);
	ge_dbg("");

	if (!ugd->b_multifile) {
		ge_dbgW("Not in multiple files selection mode!");
		return -1;
	}
	/* file selection mode */
	char *paths = NULL;
	char **path_array = NULL; /* the array size is sel_cnt */
	bool send_success = true;
	int sel_cnt = 0;
	int i = 0;

	_ge_data_get_sel_paths(ugd, &paths, &path_array, &sel_cnt);
	if (sel_cnt <= 0) {
		ge_dbgE("Invalid selected path count!");
		send_success = false;
		goto GE_SEND_RESULT_FINISHED;
	}
	if (sel_cnt <= 0 || !path_array) {
		ge_dbgE("Invalid selection or paths");
		app_control_add_extra_data(ugd->service,
		                           GE_FILE_SELECT_RETURN_COUNT, "0");
		app_control_add_extra_data(ugd->service, GE_FILE_SELECT_RETURN_PATH,
		                           NULL);
		app_control_add_extra_data(ugd->service, APP_CONTROL_DATA_SELECTED,
		                           NULL);
		app_control_add_extra_data(ugd->service, APP_CONTROL_DATA_PATH,
		                           NULL);
		ug_send_result_full(ugd->ug, ugd->service,
		                    APP_CONTROL_RESULT_FAILED);
	} else {
		if (ugd->file_select_mode == GE_FILE_SELECT_T_IMFT) {
			_ge_ext_load_app(ugd, FILETRANSFER, paths, sel_cnt);
		} else if (ugd->file_select_mode == GE_FILE_SELECT_T_MULTIPLE) {
			char t_str[32] = { 0, };
			eina_convert_itoa(sel_cnt, t_str);

			app_control_add_extra_data(ugd->service,
			                           GE_FILE_SELECT_RETURN_COUNT, t_str);
			app_control_add_extra_data(ugd->service, GE_FILE_SELECT_RETURN_PATH,
			                           paths);
			app_control_add_extra_data_array(ugd->service, APP_CONTROL_DATA_SELECTED,
			                                 (const char **)path_array, sel_cnt);
			app_control_add_extra_data_array(ugd->service, APP_CONTROL_DATA_PATH,
			                                 (const char **)path_array, sel_cnt);
			ug_send_result_full(ugd->ug, ugd->service, APP_CONTROL_RESULT_SUCCEEDED);
		}
		send_success = true;
	}

GE_SEND_RESULT_FINISHED:

	if (paths) {
		g_free(paths);
		paths = NULL;
	}
	if (path_array) {
		for (i = 0; i < sel_cnt; i++) {
			GE_FREEIF(path_array[i]);
		}
		GE_FREE(path_array);
	}
	if (send_success == true) {
		return 0;
	} else {
		return -1;
	}
}

int _ge_atoi(const char *number)
{
	char *endptr = NULL;
	long val = 0;

	errno = 0;

	val = strtol(number, &endptr, 10);

	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (errno != 0 && val == 0)) {
		ge_dbgE("strtol, val = %d", val);
		return -1;
	}

	if (endptr == number) {
		ge_dbgE("No digits were found, number = %s", number);
		return -1;
	}

	return (int)val;
}

char *_ge_str(char *str_id)
{
	GE_CHECK_NULL(str_id);
	if (strstr(str_id, "IDS_")) {
		return dgettext(GE_STR_DOMAIN_LOCAL, str_id);
	} else {
		return str_id;
	}
}


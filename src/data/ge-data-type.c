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

#include <media_content.h>
#include "ge-data-type.h"
#include "ge-debug.h"

int _ge_data_type_new_media(ge_media_s **item)
{
	GE_CHECK_VAL(item, -1);
	ge_media_s *tmp_item = (ge_media_s *)calloc(1, sizeof(ge_media_s));
	GE_CHECK_VAL(tmp_item, -1);
	tmp_item->gtype = GE_TYPE_MEDIA;
	*item =  tmp_item;
	return 0;
}

int _ge_data_type_new_album(ge_album_s **album)
{
	GE_CHECK_VAL(album, -1);
	ge_album_s *tmp_item = (ge_album_s *)calloc(1, sizeof(ge_album_s));
	GE_CHECK_VAL(tmp_item, -1);
	tmp_item->gtype = GE_TYPE_ALBUM;
	*album =  tmp_item;
	return 0;
}

static int __ge_data_type_free_media(ge_media_s **item)
{
	GE_CHECK_VAL(item, -1);
	GE_CHECK_VAL(*item, -1);
	ge_media_s *tmp_item = *item;

	/* For local medias */
	if (tmp_item->media_h) {
		if (tmp_item->b_create_thumb) {
			media_info_cancel_thumbnail(tmp_item->media_h);
			tmp_item->b_create_thumb = false;
		}
		media_info_destroy(tmp_item->media_h);
	}

	GE_FREEIF(tmp_item->uuid);
	GE_FREEIF(tmp_item->thumb_url);
	GE_FREEIF(tmp_item->file_url);
	GE_FREEIF(tmp_item->display_name);
	GE_FREE(tmp_item);
	*item = NULL;
	return 0;
}

int _ge_data_type_free_media_list(Eina_List **list)
{
	GE_CHECK_VAL(list, -1);
	GE_CHECK_VAL(*list, -1);
	ge_media_s *item = NULL;
	Eina_List *tmp_list = *list;
	EINA_LIST_FREE(tmp_list, item) {
		if (item) {
			__ge_data_type_free_media(&item);
		}
	}
	*list = NULL;
	return 0;
}

static int __ge_data_type_free_album(ge_album_s **album)
{
	GE_CHECK_VAL(album, -1);
	GE_CHECK_VAL(*album, -1);
	ge_album_s *tmp_album = *album;

	GE_FREEIF(tmp_album->uuid);
	GE_FREEIF(tmp_album->display_name);
	GE_FREEIF(tmp_album->path);
	GE_FREE(tmp_album);
	*album = NULL;
	return 0;
}

int _ge_data_type_free_album_list(Eina_List **list)
{
	GE_CHECK_VAL(list, -1);
	GE_CHECK_VAL(*list, -1);
	ge_album_s *item = NULL;
	Eina_List *tmp_list = *list;
	EINA_LIST_FREE(tmp_list, item) {
		if (item) {
			__ge_data_type_free_album(&item);
		}
	}
	*list = NULL;
	return 0;
}

int _ge_data_type_free_geitem(void **item)
{
	GE_CHECK_VAL(item, -1);
	GE_CHECK_VAL(*item, -1);
	int ret = -1;

	if (((ge_album_s *)*item)->gtype == GE_TYPE_ALBUM) {
		ret = __ge_data_type_free_album((ge_album_s **)item);
	} else if (((ge_media_s *)*item)->gtype == GE_TYPE_MEDIA ||
	           ((ge_media_s *)*item)->gtype == GE_TYPE_WEB_MEDIA) {
		ret = __ge_data_type_free_media((ge_media_s **)item);
	}

	if (ret < 0) {
		return -1;
	} else {
		return 0;
	}
}


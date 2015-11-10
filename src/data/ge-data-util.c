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

#include "ge-data-util.h"

/*
*  create a gitem
*/
ge_item *_ge_data_util_calloc_geitem(void)
{
	ge_item *gitem = (ge_item*)calloc(1, sizeof(ge_item));
	GE_CHECK_NULL(gitem);
	return gitem;
}

/*
*   destroy a ge_item
*/
int _ge_data_util_free_geitem(ge_item* gitem)
{
	if (gitem) {
		if (gitem->item) {
			_ge_data_type_free_geitem((void **)&(gitem->item));
			gitem->item = NULL;
		}
		GE_FREE(gitem);
	}
	return 0;
}

int _ge_data_util_free_mtype_items(Eina_List **elist)
{
	void *current = NULL;

	if (elist && *elist) {
		ge_dbg("Clear Mitems list.");
		EINA_LIST_FREE(*elist, current) {
			if (current) {
				_ge_data_type_free_geitem((void **)&current);
				current = NULL;
			}
		}

		*elist = NULL;
	}

	return 0;
}

int _ge_data_util_free_cluster(ge_cluster* gcluster)
{
	GE_CHECK_VAL(gcluster, -1);
	if (gcluster->cover) {
		_ge_data_util_free_item(gcluster->cover);
		gcluster->cover = NULL;
	}
	if (gcluster->cluster) {
		_ge_data_type_free_geitem((void **)&(gcluster->cluster));
		gcluster->cluster = NULL;
	}
	GE_FREE(gcluster);
	return 0;
}

ge_cluster_list *_ge_data_util_calloc_clusters_list(void)
{
	ge_cluster_list* clus_list = (ge_cluster_list*)calloc(1,
	                             sizeof(ge_cluster_list));
	GE_CHECK_NULL(clus_list);
	return clus_list;
}

ge_cluster *_ge_data_util_calloc_cluster(void)
{
	ge_cluster* gcluster = (ge_cluster*)calloc(1, sizeof(ge_cluster));
	GE_CHECK_NULL(gcluster);
	return gcluster;
}

/* Create 'All' album */
ge_cluster *_ge_data_util_new_cluster_all(ge_ugdata *ugd, int count)
{
	GE_CHECK_NULL(ugd);
	ge_cluster* gcluster = _ge_data_util_calloc_cluster();
	GE_CHECK_NULL(gcluster);

	/* Pass -1 to get a mcluster from libmedia-info, not a real record in DB */
	ge_album_s *cluster = NULL;
	_ge_data_type_new_album(&cluster);
	if (cluster == NULL) {
		GE_FREE(gcluster);
		return NULL;
	}

	cluster->uuid = strdup(GE_ALBUM_ALL_ID);
	cluster->display_name = strdup(GE_ALBUM_ALL_NAME);
	cluster->count = count;
	cluster->type = GE_ALL;
	gcluster->cluster = cluster;
	gcluster->ugd = ugd;
	gcluster->index = 0;

	return gcluster;
}

ge_item *_ge_data_util_new_item_mitem(ge_media_s *mitem)
{
	ge_item *gitem = _ge_data_util_calloc_geitem();
	GE_CHECK_NULL(gitem);
	gitem->item = mitem;

	return gitem;
}

int _ge_data_util_free_item(ge_item *gitem)
{
	GE_CHECK_VAL(gitem, -1);

	_ge_data_util_free_geitem(gitem);
	return 0;
}

/*
*  Create a gitem for selected item
*/
ge_sel_item_s *_ge_data_util_new_sel_item(ge_item *gitem)
{
	GE_CHECK_NULL(gitem);
	GE_CHECK_NULL(gitem->item);
	ge_sel_item_s *item = (ge_sel_item_s *)calloc(1, sizeof(ge_sel_item_s));
	GE_CHECK_NULL(item);
	item->uuid = strdup(gitem->item->uuid);
	item->file_url = strdup(gitem->item->file_url);
	item->store_type = gitem->store_type;
	item->sequence = gitem->sequence;
	return item;
}

/*
*  Free a ge_sel_item_s item
*/
int _ge_data_util_free_sel_item(ge_sel_item_s *item)
{
	GE_CHECK_VAL(item, -1);
	GE_FREEIF(item->uuid);
	GE_FREEIF(item->file_url);
	GE_FREE(item);
	return 0;
}

/*
*  Create a album for selected item's album
*/
ge_sel_album_s *_ge_data_util_new_sel_album(char *album_uuid)
{
	GE_CHECK_NULL(album_uuid);
	ge_sel_album_s *album = NULL;

	album = (ge_sel_album_s *)calloc(1, sizeof(ge_sel_album_s));
	GE_CHECK_NULL(album);
	album->uuid = strdup(album_uuid);
	return album;
}

int _ge_data_util_create_filter2(char *cond, char *keyword, int offset, int count, filter_h *filter)
{
	GE_CHECK_VAL(filter, -1);
	int ret = -1;
	filter_h _filter = NULL;

	ret = media_filter_create(&_filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Fail to create filter!");
		return -1;
	}

	if (cond && strlen(cond) > 0) {
		ret = media_filter_set_condition(_filter, cond,
		                                 MEDIA_CONTENT_COLLATE_NOCASE);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			ge_dbgE("Fail to set condition!");
			goto GE_DATA_UTIL_FAILED;
		}
	}

	if (keyword && strlen(keyword) > 0) {
		media_content_order_e order_type = MEDIA_CONTENT_ORDER_DESC;
		if (!g_strcmp0(MEDIA_DISPLAY_NAME, keyword)) {
			order_type = MEDIA_CONTENT_ORDER_ASC;
		}
		ret = media_filter_set_order(_filter, order_type, keyword,
		                             MEDIA_CONTENT_COLLATE_NOCASE);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			ge_dbgE("Fail to set order!");
			goto GE_DATA_UTIL_FAILED;
		}
	}

	if (offset != -1) {
		ge_dbg("offset: %d, count: %d", offset, count);
		ret = media_filter_set_offset(_filter, offset, count);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			ge_dbgE("Fail to set offset!");
			goto GE_DATA_UTIL_FAILED;
		}
	}

	*filter = _filter;
	return 0;

GE_DATA_UTIL_FAILED:

	if (_filter) {
		media_filter_destroy(_filter);
		_filter = NULL;
		*filter = NULL;
	}
	return -1;
}

int _ge_data_util_create_filter(ge_filter_s *condition, filter_h *filter)
{
	GE_CHECK_VAL(filter, -1);
	GE_CHECK_VAL(condition, -1);
	int ret = -1;
	filter_h tmp_filter = NULL;

	ret = media_filter_create(&tmp_filter);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Fail to create filter!");
		return -1;
	}

	if (strlen(condition->cond) > 0) {
		ret = media_filter_set_condition(tmp_filter, condition->cond,
		                                 condition->collate_type);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			ge_dbgE("Fail to set condition!");
			goto GE_DATA_UTIL_FAILED;
		}
	}

	if (strlen(condition->sort_keyword) > 0) {
		ret = media_filter_set_order(tmp_filter, condition->sort_type,
		                             condition->sort_keyword,
		                             condition->collate_type);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			ge_dbgE("Fail to set order!");
			goto GE_DATA_UTIL_FAILED;
		}
	}

	ge_dbg("offset: %d, count: %d", condition->offset, condition->count);
	if (condition->offset != -1) {
		ret = media_filter_set_offset(tmp_filter, condition->offset,
		                              condition->count);
		if (ret != MEDIA_CONTENT_ERROR_NONE) {
			ge_dbgE("Fail to set offset!");
			goto GE_DATA_UTIL_FAILED;
		}
	}

	*filter = tmp_filter;
	return 0;

GE_DATA_UTIL_FAILED:

	if (tmp_filter) {
		media_filter_destroy(tmp_filter);
		tmp_filter = NULL;
		*filter = NULL;
	}
	return -1;
}

int _ge_data_util_destroy_filter(filter_h filter)
{
	GE_CHECK_VAL(filter, -1);

	if (media_filter_destroy(filter) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Failed to destroy filter!");
		return -1;
	}

	return 0;
}

bool _ge_data_util_clone_media(media_info_h media, ge_media_s **pitem,
                               bool b_meta)
{
	GE_CHECK_FALSE(pitem);
	ge_media_s *item = NULL;

	*pitem = NULL;

	item = (ge_media_s *)calloc(1, sizeof(ge_media_s));
	GE_CHECK_FALSE(item);
	item->gtype = GE_TYPE_MEDIA;

	if (media_info_clone(&(item->media_h), media) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Clone media handle error");
		goto GE_DATA_UTIL_FAILED;
	}
	if (media_info_get_media_id(media, &(item->uuid)) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get media id error");
		goto GE_DATA_UTIL_FAILED;
	}
	if (media_info_get_display_name(media, &(item->display_name)) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get media display name error");
		goto GE_DATA_UTIL_FAILED;
	}
	if (media_info_get_file_path(media, &(item->file_url)) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get media file path error");
		goto GE_DATA_UTIL_FAILED;
	}
	if (media_info_get_media_type(media, (media_content_type_e *)&(item->type)) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get media type error");
		goto GE_DATA_UTIL_FAILED;
	}
	if (media_info_get_thumbnail_path(media, &(item->thumb_url)) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get media thumbnail path error");
		goto GE_DATA_UTIL_FAILED;
	}
	ge_sdbg("thumb_url: %s", item->thumb_url);

	if (media_info_get_timeline(media, &(item->mtime)) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get media modified time error");
		goto GE_DATA_UTIL_FAILED;
	}
//	if (media_info_get_mode(media, (media_content_mode_e *)&(item->mode)) != MEDIA_CONTENT_ERROR_NONE) {
//		ge_dbgE("Get media mode failed!");
//		goto GE_DATA_UTIL_FAILED;
//	}
	media_content_storage_e storage_type = 0;
	if (media_info_get_storage_type(media, &storage_type) != MEDIA_CONTENT_ERROR_NONE) {
		ge_dbgE("Get storage type failed!");
		goto GE_DATA_UTIL_FAILED;
	}
	if (storage_type == MEDIA_CONTENT_STORAGE_INTERNAL) { /* The device's internal storage */
		item->storage_type = GE_PHONE;
	} else if (storage_type == MEDIA_CONTENT_STORAGE_EXTERNAL) { /* The device's external storage */
		item->storage_type = GE_MMC;
	} else {
		ge_dbgE("Undefined mode[%d]!", storage_type);
	}

	*pitem = item;
	return true;

GE_DATA_UTIL_FAILED:

	_ge_data_type_free_geitem((void **)(&item));
	return false;
}

/** @cond is allocated by caller, maybe it's an array */
int _ge_data_util_get_geo_cond(char *cond, double longmin, double longmax,
                               double latmin, double latmax)
{
	GE_CHECK_VAL(cond, -1);

	char *long_max = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
	char *long_min = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
	char *lat_max = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
	char *lat_min = g_new0(char, G_ASCII_DTOSTR_BUF_SIZE);
	int ret = -1;

	if (long_max == NULL || long_min == NULL || lat_max == NULL ||
	        lat_min == NULL) {
		ge_dbgE("Failed to malloc memory!");
		goto GE_DATA_UTIL_FAILED;
	}

	ge_sdbg("Double, Region: LongMax %f, LongMin %f, LatMax %f, LatMin %f",
	        longmax, longmin, latmax, latmin);
	/* 2013.04.23
	  * The format %lf in snprintf has a bug:
	  * The longitude should be 127.05998, but the result after snprintf is 127,05998.
	  * This cause media db operation failure(dot->comma).
	  * To avoid this bug, float/double number is converted to string first, then put %s to snprintf.
	  * This extra conversion can be deleted if the format bug in snprintf is fixed.
	  */
	g_ascii_dtostr(long_min, G_ASCII_DTOSTR_BUF_SIZE, longmin);
	g_ascii_dtostr(long_max, G_ASCII_DTOSTR_BUF_SIZE, longmax);
	g_ascii_dtostr(lat_min, G_ASCII_DTOSTR_BUF_SIZE, latmin);
	g_ascii_dtostr(lat_max, G_ASCII_DTOSTR_BUF_SIZE, latmax);
	ge_sdbg("String, Region: LongMax %s, LongMin %s, LatMax %s, LatMin %s",
	        long_max, long_min, lat_max, lat_min);
	g_snprintf(cond, CONDITION_LENGTH,
	           "(%s>=%s AND %s<=%s) AND (%s>=%s AND %s<=%s)",
	           MEDIA_LONGITUDE, long_min, MEDIA_LONGITUDE, long_max,
	           MEDIA_LATITUDE, lat_min, MEDIA_LATITUDE, lat_max);
	ge_sdbg("g_snprintf, %s", cond);
	ret = 0;

GE_DATA_UTIL_FAILED:

	GE_GFREEIF(long_max);
	GE_GFREEIF(long_min);
	GE_GFREEIF(lat_max);
	GE_GFREEIF(lat_min);
	return ret;
}

int _ge_data_util_get_type_cond(char *cond, int file_t)
{
	if (file_t == GE_FILE_T_IMAGE) {
		g_strlcpy(cond, GE_CONDITION_IMAGE, CONDITION_LENGTH);
	} else if (file_t == GE_FILE_T_VIDEO) {
		g_strlcpy(cond, GE_CONDITION_VIDEO, CONDITION_LENGTH);
	} else {
		g_strlcpy(cond, GE_CONDITION_IMAGE_VIDEO, CONDITION_LENGTH);
	}
	return 0;
}


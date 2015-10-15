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

#ifndef _GE_DATA_TYPE_H_
#define _GE_DATA_TYPE_H_

#include <stdlib.h>
#include <eina_list.h>
#include <media_content.h>

#define GE_FREEIF(ptr) \
	do { \
		if (ptr != NULL) \
		{	\
			free(ptr); \
			ptr = NULL; \
		} \
	} while (0)

#define GE_FREE(ptr) \
	do { \
		free(ptr); \
		ptr = NULL; \
	} while (0)

#define GE_GFREEIF(ptr) \
	do { \
		if (ptr != NULL) { \
			g_free(ptr); \
			ptr = NULL; \
		} \
	} while (0)

#define GE_GFREE(ptr) \
	do { \
		g_free(ptr); \
		ptr = NULL; \
	} while (0)

#define GE_TYPE_ALBUM (0x55551)
#define GE_TYPE_MEDIA (0x55553)
#define GE_TYPE_WEB_MEDIA (0x55554)

typedef enum
{
	GE_PHONE,/**< Stored only in phone */
	GE_MMC,	/**< Stored only in MMC */
	GE_ALL,	/**< Stored in ALL albums */
} _ge_store_type_t;


typedef struct _ge_album_t ge_album_s;
typedef struct _ge_media_t ge_media_s;

struct _ge_album_t {
	int gtype;                         /*self-defination type, when free this struct space, use it*/
	char *uuid;                        /*folder  UUID*/
	char *display_name;                /*album name*/
	char *path;                        /*the full path of this folder*/
	int type;                          /*storage type*/
	time_t mtime;                      /*modified time*/
	int count;                         /*the media count in this folder*/
};

struct _ge_media_t {
	int gtype;                         /*self-defination type, when free this struct space, use it*/
	media_info_h media_h;              /*the handle of operating this media*/
	char *uuid;                        /*meida id*/
	int type;                          /*meida type, image or video*/
	char *thumb_url;                   /*the thumbnail full path of this meida file*/
	char *file_url;                    /*the full path of this meida file*/
	time_t mtime;                      /*modified time*/
	char *display_name;                /*item name*/
	int mode; /* Show icon indicates different camera shot mode */
	media_content_storage_e storage_type;      /* Storage type got from DB */
	bool b_create_thumb;		/* Request DB to create thumbnail */
};

int _ge_data_type_new_media(ge_media_s **item);
int _ge_data_type_new_album(ge_album_s **album);

int _ge_data_type_free_media_list(Eina_List **list);
int _ge_data_type_free_album_list(Eina_List **list);
int _ge_data_type_free_geitem(void **item);

#endif


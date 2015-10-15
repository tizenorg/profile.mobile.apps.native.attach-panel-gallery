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

/**
 * The Exif specification defines an Orientation Tag to indicate the orientation of the
 * camera relative to the captured scene. This can be used by the camera either to
 * indicate the orientation automatically by an orientation sensor,
 * or to allow the user to indicate the orientation manually by a menu switch,
 * without actually transforming the image data itself.
 * Here is an explanation given by TsuruZoh Tachibanaya in
 * his description of the Exif file format:
 *
 * The orientation of the camera relative to the scene, when the image was captured.
 * The relation of the '0th row' and '0th column' to visual position is shown as below.
 *
 * Value	 0th Row	        0th Column
 *   1	  top	          left side
 *   2	  top	          right side
 *   3	  bottom	          right side
 *   4	  bottom	          left side
 *   5	  left side          top
 *   6	  right side        top
 *   7	  right side        bottom
 *   8	  left side          bottom
 *
 * Read this table as follows (thanks to Peter Nielsen for clarifying this - see also below):
 * Entry #6 in the table says that the 0th row in the stored image is the right side of
 * the captured scene, and the 0th column in the stored image is the top side of
 * the captured scene.
 *
 * Here is another description given by Adam M. Costello:
 *
 * For convenience, here is what the letter F would look like if it were tagged correctly
 * and displayed by a program that ignores the orientation tag
 * (thus showing the stored image):
 *
 *       1             2         3      4            5                 6                   7                  8
 *
 *  888888  888888       88    88      8888888888  88                             88  8888888888
 *  88               88        88    88      88  88          88  88                  88  88          88  88
 *  8888        8888     8888   8888   88                8888888888  8888888888               88
 *  88               88        88    88
 *  88               88  888888   888888
*/

#ifdef _USE_ROTATE_BG_GE

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <Elementary.h>
#include "ge-exif.h"
#include "ge-debug.h"
#include "ge-data-type.h"

#define GE_EXIF_BUF_LEN_MAX 65536L
#define GE_EXIF_BUF_TIME_LEN_MAX 20
#define GE_EXIF_DEFAULT_YEAR 1900

#define GE_EXIF_SOI 0xD8
#define GE_EXIF_TAG 0xFF
#define GE_EXIF_APP0 0xE0
#define GE_EXIF_APP1 0xE1
#define GE_EXIF_JFIF_00 0x00
#define GE_EXIF_JFIF_01 0x01
#define GE_EXIF_JFIF_02 0x02

#define GE_EXIF_IFD_DATA_FORMAT_UNSIGNED_BYTE 1
#define GE_EXIF_IFD_DATA_FORMAT_ASCII_STRINGS 1
#define GE_EXIF_IFD_DATA_FORMAT_UNSIGNED_SHORT 2
#define GE_EXIF_IFD_DATA_FORMAT_UNSIGNED_LONG 4
#define GE_EXIF_IFD_DATA_FORMAT_UNSIGNED_RATIONAL 8
#define GE_EXIF_IFD_DATA_FORMAT_SIGNED_BYTE 1
#define GE_EXIF_IFD_DATA_FORMAT_UNDEFINED 1
#define GE_EXIF_IFD_DATA_FORMAT_SIGNED_SHORT 2
#define GE_EXIF_IFD_DATA_FORMAT_SIGNED_LONG 4
#define GE_EXIF_IFD_DATA_FORMAT_SIGNED_RATIONAL 8
#define GE_EXIF_IFD_DATA_FORMAT_SIGNED_FLOAT 4
#define GE_EXIF_IFD_DATA_FORMAT_DOUBLE_FLOAT 8

#define GE_EXI_TMP_JPEG_FILE "/opt/usr/apps/org.tizen.gallery/data/.gallery_tmp_write_exif.jpg"

/* Read one byte, testing for EOF */
static int __ge_exif_read_1_byte(FILE *fd)
{
	int c = 0;

	/* Return next input byte, or EOF if no more */
	c = getc(fd);
	if (c == EOF) {
		ge_dbgE("Premature EOF in JPEG file!");
		return -1;
	}

	return c;
}

/* Read 2 bytes, convert to unsigned int */
/* All 2-byte quantities in JPEG markers are MSB first */
static int __ge_exif_read_2_bytes(FILE *fd, unsigned int *len)
{
	int c1 = 0;
	int c2 = 0;

	/* Return next input byte, or EOF if no more */
	c1 = getc(fd);
	if (c1 == EOF) {
		ge_dbgE("Premature EOF in JPEG file!");
		return -1;
	}

	/* Return next input byte, or EOF if no more */
	c2 = getc(fd);
	if (c2 == EOF) {
		ge_dbgE("Premature EOF in JPEG file!");
		return -1;
	}

	if (len)
		*len = (((unsigned int)c1) << 8) + ((unsigned int)c2);

	return 0;
}

static int __ge_exif_rw_jfif(FILE *fd, char *file_path,
			     unsigned int *orientation)
{
	GE_CHECK_VAL(fd, -1);
	GE_CHECK_VAL(file_path, -1);
	GE_CHECK_VAL(orientation, -1);
	unsigned char tmp[GE_EXIF_BUF_LEN_MAX] = { 0, };
	int i = 0;
	unsigned int length = 0;
	int tmp_exif = -1;
	bool is_motorola = false; /* Flag for byte order */
	unsigned int offset = 0;
	int ret = -1;
	/*unsigned char version = 0x00; */

	if (__ge_exif_read_2_bytes(fd, &length) < 0)
		goto GE_EXIF_FAILED;
	ge_dbg("length: %d", length);

	for (i = 0; i < 5; i++) {
		tmp_exif = __ge_exif_read_1_byte(fd);
		if (tmp_exif < 0)
			goto GE_EXIF_FAILED;
		tmp[i] = (unsigned char)tmp_exif;
	}

	/* JFIF0 */
	if (tmp[0] != 0x4A || tmp[1] != 0x46 || tmp[2] != 0x49 ||
	    tmp[3] != 0x46 || tmp[4] != 0x00) {
		ge_dbgE("Not met Jfif!");
		goto GE_EXIF_FAILED;
	}

	for (i = 0; i < 2; i++) {
		tmp_exif = __ge_exif_read_1_byte(fd);
		if (tmp_exif < 0)
			goto GE_EXIF_FAILED;
		tmp[i] = (unsigned char)tmp_exif;
	}

	/* Check JFIF version */
	if (tmp[0] == 0x01 && tmp[1] == GE_EXIF_JFIF_00) {
		ge_dbg("Jfif 1.00");
	} else if (tmp[0] == 0x01 && tmp[1] == GE_EXIF_JFIF_01) {
		ge_dbg("Jfif 1.01");
	} else if (tmp[0] == 0x01 && tmp[1] == GE_EXIF_JFIF_02) {
		ge_dbg("Jfif 1.02");
	} else {
		ge_dbgE("Unknow Jfif version[%d.%d]!", tmp[0], tmp[1]);
		goto GE_EXIF_FAILED;
	}

	/* Save version */
	/*version = tmp[1]; */

	/* Find APP1 */
	bool b_tag_ff = false;
	while(1) {
		tmp_exif = __ge_exif_read_1_byte(fd);
		if (tmp_exif < 0)
			goto GE_EXIF_FAILED;

		tmp[0] = (unsigned char)tmp_exif;

		ge_dbg("- %02X", tmp[0]);
		if (!b_tag_ff) {
			/* Get first tag */
			if (tmp[0] == GE_EXIF_TAG) {
				ge_dbgW("0xFF!");
				b_tag_ff = true;
			}
			continue;
		}

		/* Get APP1 */
		if (tmp[0] == GE_EXIF_APP1) {
			ge_dbgW("Exif in APP1!");
			break;
		}

		ge_dbgW("No Exif in APP1!");

		/* Close file */
		fclose(fd);
		/* Normal orientation = 0degree = 1 */
		*orientation = 1;
		return 0;

	}

	/* Find Exif */
	while(1) {
		tmp_exif = __ge_exif_read_1_byte(fd);
		if (tmp_exif < 0)
			goto GE_EXIF_FAILED;

		tmp[0] = (unsigned char)tmp_exif;
		if (tmp[0] != 0x45)
			continue;

		for (i = 0; i < 5; i++) {
			tmp_exif = __ge_exif_read_1_byte(fd);
			if (tmp_exif < 0)
				goto GE_EXIF_FAILED;

			tmp[i] = (unsigned char)tmp_exif;
			ge_dbg("- %02X", tmp[i]);
		}
		if (tmp[0] == 0x78 && tmp[1] == 0x69 && tmp[2] == 0x66 &&
		    tmp[3] == 0x00 && tmp[4] == 0x00) {
			ge_dbgW("Met Exif!");
			break;
		} else {
			ge_dbgW("Not met Exif!");
			goto GE_EXIF_FAILED;
		}
	}

	/* Read Exif body */
	for (i = 0; i < 4; i++) {
		tmp_exif = __ge_exif_read_1_byte(fd);
		if (tmp_exif < 0)
			goto GE_EXIF_FAILED;
		tmp[i] = (unsigned char)tmp_exif;
	}

	/* Check byte order and Tag Mark , "II(0x4949)" or "MM(0x4d4d)" */
	if (tmp[0] == 0x49 && tmp[1] == 0x49 && tmp[2] == 0x2A &&
	    tmp[3] == 0x00) {
		ge_dbg("Intel");
		is_motorola = false;
	} else if (tmp[0] == 0x4D && tmp[1] == 0x4D && tmp[2] == 0x00 &&
		   tmp[3] == 0x2A) {
		ge_dbg("Motorola");
		is_motorola = true;
	} else {
		goto GE_EXIF_FAILED;
	}

	for (i = 0; i < 4; i++) {
		tmp_exif = __ge_exif_read_1_byte(fd);
		if (tmp_exif < 0)
			goto GE_EXIF_FAILED;

		tmp[i] = (unsigned char)tmp_exif;
		ge_dbg("- %02X", tmp[i]);
	}

	/* Get first IFD offset (offset to IFD0) , MM-08000000, II-00000008 */
	if (is_motorola) {
		if (tmp[0] != 0 && tmp[1] != 0)
			goto GE_EXIF_FAILED;
		offset = tmp[2];
		offset <<= 8;
		offset += tmp[3];
	} else {
		if (tmp[3] != 0 && tmp[2] != 0)
			goto GE_EXIF_FAILED;
		offset = tmp[1];
		offset <<= 8;
		offset += tmp[0];
	}
	ge_dbg("offset: %d", offset);

	/* IFD: Image File Directory */
	/* Get the number of directory entries contained in this IFD, - 2 bytes, EE */
	unsigned int tags_cnt = 0;
	for (i = 0; i < 2; i++) {
		tmp_exif = __ge_exif_read_1_byte(fd);
		if (tmp_exif < 0)
			goto GE_EXIF_FAILED;

		tmp[i] = (unsigned char)tmp_exif;
	}
	if (is_motorola) {
		tags_cnt = tmp[0];
		tags_cnt <<= 8;
		tags_cnt += tmp[1];
	} else {
		tags_cnt = tmp[1];
		tags_cnt <<= 8;
		tags_cnt += tmp[0];
	}
	ge_dbg("tags_cnt: %d", tags_cnt);
	if (tags_cnt == 0) {
		ge_dbgE("tags_cnt == 0 - 2");
		goto GE_EXIF_FAILED;
	}

	/* Search for Orientation Tag in IFD0 */
	unsigned int tag_num = 0;
	while (1) {
		/* Every directory entry size is 12 */
		for (i = 0; i < 12; i++) {
			tmp_exif = __ge_exif_read_1_byte(fd);
			if (tmp_exif < 0)
				goto GE_EXIF_FAILED;

			tmp[i] = (unsigned char)tmp_exif;
		}
		/* Get Tag number */
		if (is_motorola) {
			tag_num = tmp[0];
			tag_num <<= 8;
			tag_num += tmp[1];
		} else {
			tag_num = tmp[1];
			tag_num <<= 8;
			tag_num += tmp[0];
		}
		/* found Orientation Tag */
		if (tag_num == 0x0112) {
			ge_dbgW("Found orientation tag!");
			break;
		}
		if (--tags_cnt == 0) {
			ge_dbgW("tags_cnt == 0, no found orientation tag!");
			/* Normal orientation = 0degree = 1 */
			*orientation = 1;
			ret = 0;
			goto GE_EXIF_FAILED;
		}
	}

	/* |TT|ff|NNNN|DDDD|  ---  TT - 2 bytes, tag NO. ;ff - 2 bytes, data format
	     NNNN - 4 bytes, entry count;  DDDD - 4 bytes Data value */
	/* Get the Orientation value */
	if (is_motorola) {
		if (tmp[8] != 0) {
			ge_dbgE("tmp[8] != 0");
			goto GE_EXIF_FAILED;
		}
		*orientation = (unsigned int)tmp[9];
	} else {
		if (tmp[9] != 0) {
			ge_dbgE("tmp[9] != 0");
			goto GE_EXIF_FAILED;
		}
		*orientation = (unsigned int)tmp[8];
	}
	if (*orientation > 8) {
		ge_dbgE("*orient > 8");
		goto GE_EXIF_FAILED;
	}
	ge_dbg("Read: %d", *orientation);
	ret = 0;

 GE_EXIF_FAILED:

	fclose(fd);
	ge_dbg("All done");
	return ret;
}

static int __ge_exif_rw_orient(char *file_path, unsigned int *orient)
{
	GE_CHECK_VAL(file_path, -1);
	unsigned int length = 0;
	unsigned int i = 0;
	bool is_motorola = false; /* Flag for byte order */
	unsigned int offset = 0;
	unsigned int tags_cnt = 0;
	unsigned int tag_num = 0;
	int tmp_exif = -1;
	unsigned char exif_data[GE_EXIF_BUF_LEN_MAX] = { 0, };
	FILE *fd = NULL;
	int ret = -1;

	if ((fd = fopen(file_path, "rb")) == NULL) {
		ge_sdbgE("Can't open %s!", file_path);
		return -1;
	}


	/* Read File head, check for JPEG SOI + Exif APP1 */
	for (i = 0; i < 4; i++) {
		tmp_exif = __ge_exif_read_1_byte(fd);
		if (tmp_exif < 0)
			goto GE_EXIF_FAILED;

		exif_data[i] = (unsigned char)tmp_exif;
	}

	if (exif_data[0] == GE_EXIF_TAG && exif_data[1] == GE_EXIF_SOI) {
		ge_dbg("JPEG file");
	} else {
		ge_dbgE("Not a JPEG file!");
		goto GE_EXIF_FAILED;
	}

	if (exif_data[2] == GE_EXIF_TAG && exif_data[3] == GE_EXIF_APP1) {
		ge_dbgW("Exif in APP1!");
	} else if (exif_data[2] == GE_EXIF_TAG &&
		   exif_data[3] == GE_EXIF_APP0) {
		ge_dbgW("Jfif in APP0!");
		int ret = __ge_exif_rw_jfif(fd, file_path, orient);
		return ret;
	} else {
		ge_dbgE("Not a Exif in APP1 or Jiff in APP2[%d]!", exif_data[3]);
		goto GE_EXIF_FAILED;
	}

	/* Get the marker parameter length count */
	if (__ge_exif_read_2_bytes(fd, &length) < 0)
		goto GE_EXIF_FAILED;
	ge_dbg("length: %d", length);
	/* Length includes itself, so must be at least 2
	    Following Exif data length must be at least 6 */
	if (length < 8) {
		ge_dbgE("length < 8");
		goto GE_EXIF_FAILED;
	}
	length -= 8;

	 /* Length of an IFD entry */
	if (length < 12) {
		ge_dbgE("length < 12");
		goto GE_EXIF_FAILED;
	}

	/* Read Exif head, check for "Exif" */
	for (i = 0; i < 6; i++) {
		tmp_exif = __ge_exif_read_1_byte(fd);
		if (tmp_exif < 0)
			goto GE_EXIF_FAILED;

		exif_data[i] = (unsigned char)tmp_exif;
		ge_dbg("- %02X", exif_data[i]);
	}

	if (exif_data[0] != 0x45 || exif_data[1] != 0x78 ||
	    exif_data[2] != 0x69 || exif_data[3] != 0x66 ||
	    exif_data[4] != 0x00 || exif_data[5] != 0x00) {
		ge_dbgE("Not met Exif!");
		goto GE_EXIF_FAILED;
	}

	/* Read Exif body */
	for (i = 0; i < length; i++) {
		tmp_exif = __ge_exif_read_1_byte(fd);
		if (tmp_exif < 0)
			goto GE_EXIF_FAILED;
		exif_data[i] = (unsigned char)tmp_exif;
	}

	/* Check byte order and Tag Mark , "II(0x4949)" or "MM(0x4d4d)" */
	if (exif_data[0] == 0x49 && exif_data[1] == 0x49 &&
	    exif_data[2] == 0x2A && exif_data[3] == 0x00) {
		ge_dbg("Intel");
		is_motorola = false;
	} else if (exif_data[0] == 0x4D && exif_data[1] == 0x4D &&
		 exif_data[2] == 0x00 && exif_data[3] == 0x2A) {
		ge_dbg("Motorola");
		is_motorola = true;
	} else {
		goto GE_EXIF_FAILED;
	}

	/* Get first IFD offset (offset to IFD0) , MM-00000008, II-08000000 */
	if (is_motorola) {
		if (exif_data[4] != 0 && exif_data[5] != 0)
			goto GE_EXIF_FAILED;
		offset = exif_data[6];
		offset <<= 8;
		offset += exif_data[7];
	} else {
		if (exif_data[7] != 0 && exif_data[6] != 0)
			goto GE_EXIF_FAILED;
		offset = exif_data[5];
		offset <<= 8;
		offset += exif_data[4];
	}
	/* check end of data segment */
	if (offset > length - 2) {
		ge_dbgE("offset > length - 2");
		goto GE_EXIF_FAILED;
	}

	/* IFD: Image File Directory */
	/* Get the number of directory entries contained in this IFD, - EEEE */
	if (is_motorola) {
		tags_cnt = exif_data[offset];
		tags_cnt <<= 8;
		tags_cnt += exif_data[offset+1];
	} else {
		tags_cnt = exif_data[offset+1];
		tags_cnt <<= 8;
		tags_cnt += exif_data[offset];
	}
	if (tags_cnt == 0) {
		ge_dbgE("tags_cnt == 0 - 2");
		goto GE_EXIF_FAILED;
	}
	offset += 2;

	/* check end of data segment */
	if (offset > length - 12) {
		ge_dbgE("offset > length - 12");
		goto GE_EXIF_FAILED;
	}

	/* Search for Orientation Tag in IFD0 */
	while (1) {
		/* Get Tag number */
		if (is_motorola) {
			tag_num = exif_data[offset];
			tag_num <<= 8;
			tag_num += exif_data[offset+1];
		} else {
			tag_num = exif_data[offset+1];
			tag_num <<= 8;
			tag_num += exif_data[offset];
		}
		/* found Orientation Tag */
		if (tag_num == 0x0112) {
			ge_dbgW("Found orientation tag!");
			break;
		}
		if (--tags_cnt == 0) {
			ge_dbgW("tags_cnt == 0, no found orientation tag!");
			/* Normal orientation = 0degree = 1 */
			*orient = 1;
			ret = 0;
			goto GE_EXIF_FAILED;
		}

		/* Every directory entry size is 12 */
		offset += 12;
	}

	/* Get the Orientation value */
	if (is_motorola) {
		if (exif_data[offset+8] != 0) {
			ge_dbgE("exif_data[offset+8] != 0");
			goto GE_EXIF_FAILED;
		}
		*orient = (unsigned int)exif_data[offset+9];
	} else {
		if (exif_data[offset+9] != 0) {
			ge_dbgE("exif_data[offset+9] != 0");
			goto GE_EXIF_FAILED;
		}
		*orient = (unsigned int)exif_data[offset+8];
	}
	if (*orient > 8) {
		ge_dbgE("*orient > 8");
		goto GE_EXIF_FAILED;
	}
	ge_dbg("Read: %d", *orient);

	ret = 0;

 GE_EXIF_FAILED:

	fclose(fd);
	ge_dbg("All done");
	return ret;
}

int _ge_exif_get_orientation(char *file_path, unsigned int *orientation)
{
	GE_CHECK_VAL(orientation, -1);
	GE_CHECK_VAL(file_path, -1);
	ge_sdbg("file_path: %s", file_path);

	return __ge_exif_rw_orient(file_path, orientation);
}

#endif


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

#ifndef _GL_DEBUG_H_
#define _GL_DEBUG_H_

#include <dlog.h>

#ifdef _cplusplus
extern "C" {
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "ATTACH_PANEL_GALLERY"

typedef enum {
	DEBUG_CRITICAL,
	DEBUG_CHECK,
	MB_DEBUG_LOG,
	DEBUG_MAX
} GlDebugType;

/* anci c color type */
#define FONT_COLOR_RESET    "\033[0m"
#define FONT_COLOR_RED      "\033[31m"
#define FONT_COLOR_GREEN    "\033[32m"
#define FONT_COLOR_YELLOW   "\033[33m"
#define FONT_COLOR_BLUE     "\033[34m"
#define FONT_COLOR_PURPLE   "\033[35m"
#define FONT_COLOR_CYAN     "\033[36m"
#define FONT_COLOR_GRAY     "\033[37m"

#ifdef _USE_DLOG_

#define ge_dbg(fmt, arg...)		LOGD(FONT_COLOR_BLUE fmt FONT_COLOR_RESET, ##arg)
#define ge_dbgW(fmt, arg...)		LOGW(FONT_COLOR_GREEN fmt FONT_COLOR_RESET, ##arg)
#define ge_dbgE(fmt, arg...)		LOGE(FONT_COLOR_RED fmt FONT_COLOR_RESET, ##arg)

#else

#define ge_dbg(fmt,arg...)
#define ge_dbgW(fmt, arg...)
#define ge_dbgE(fmt, arg...)
#endif

#ifdef _USE_SECURE_LOG_
#define ge_sdbg(fmt, arg...)  SECURE_LOGD(FONT_COLOR_BLUE fmt FONT_COLOR_RESET, ##arg)
#define ge_sdbgW(fmt, arg...) SECURE_LOGI(FONT_COLOR_GREEN fmt FONT_COLOR_RESET, ##arg)
#define ge_sdbgE(fmt, arg...) SECURE_LOGE(FONT_COLOR_RED fmt FONT_COLOR_RESET, ##arg)
#endif

#define ge_warn_if(expr, fmt, arg...) do { \
		if (expr) { \
			ge_dbg("(%s) -> "fmt, #expr, ##arg); \
		} \
	} while (0)
#define ge_ret_if(expr) do { \
		if (expr) { \
			ge_dbgE("(%s) -> %s() return", #expr, __FUNCTION__); \
			return; \
		} \
	} while (0)
#define ge_retv_if(expr, val) do { \
		if (expr) { \
			ge_dbgE("(%s) -> %s() return", #expr, __FUNCTION__); \
			return (val); \
		} \
	} while (0)
#define ge_retm_if(expr, fmt, arg...) do { \
		if (expr) { \
			ge_dbgE(fmt, ##arg); \
			ge_dbgE("(%s) -> %s() return", #expr, __FUNCTION__); \
			return; \
		} \
	} while (0)
#define ge_retvm_if(expr, val, fmt, arg...) do { \
		if (expr) { \
			ge_dbgE(fmt, ##arg); \
			ge_dbgE("(%s) -> %s() return", #expr, __FUNCTION__); \
			return (val); \
		} \
	} while (0)


#define GE_CHECK_VAL(expr, val) 		ge_retvm_if(!(expr), val, "Invalid parameter, return ERROR code!")
#define GE_CHECK_NULL(expr) 			ge_retvm_if(!(expr), NULL, "Invalid parameter, return NULL!")
#define GE_CHECK_FALSE(expr) 			ge_retvm_if(!(expr), false, "Invalid parameter, return FALSE!")
#define GE_CHECK_CANCEL(expr) 			ge_retvm_if(!(expr), ECORE_CALLBACK_CANCEL, "Invalid parameter, return ECORE_CALLBACK_CANCEL!")
#define GE_CHECK(expr) 					ge_retm_if(!(expr), "Invalid parameter, return!")


#include <time.h>
#include <sys/time.h>

void _ge_print_debug_time_ex(long start, long end, const char* func_name, char* time_string);

#ifdef _cplusplus
}
#endif

#endif //_GL_DEBUG_H_


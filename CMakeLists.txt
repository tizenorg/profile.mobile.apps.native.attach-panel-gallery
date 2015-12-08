CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(ug-attach-panel-gallery C)
SET(GALLERY_UG_PROJECT_NAME ug-attach-panel-gallery)
SET(GE_EDC_DIR black_theme/edc)
SET(GE_IMG_DIR black_theme/edc/images)
SET(GE_SOUND_DIR black_theme/edc/sounds)
SET(GE_IMG_EDC_DIR black_theme/images_edc)
SET(GE_IMG_C_DIR black_theme/images)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(RESDIR "${PREFIX}/res")
SET(LOCALEDIR "${RESDIR}/locale")
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")
SET(VERSION 1.0)

ADD_DEFINITIONS("-D_USE_DLOG_")
ADD_DEFINITIONS("-D_USE_SECURE_LOG_")
ADD_DEFINITIONS("-D_USE_ROTATE_BG_GE")
ADD_DEFINITIONS("-DLOCALEDIR=\"${LOCALEDIR}\"")
#ADD_DEFINITIONS("-D_USE_CUSTOMIZED_GENGRID_STYLE")
#ADD_DEFINITIONS("-D_USE_SHOW_SELECTED_COUNT_IN_TITLE")
#ADD_DEFINITIONS("-D_USE_ADD_TO_HOME_FOR_IMAGES")

AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src MAIN_UG_SRCS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/data DATA_UG_SRCS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/util UTIL_UG_SRCS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/view VIEW_UG_SRCS)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/src/widget WIDGET_UG_SRCS)

SET(UG_SRCS
	${MAIN_UG_SRCS}
	${DATA_UG_SRCS}
	${UTIL_UG_SRCS}
	${VIEW_UG_SRCS}
	${WIDGET_UG_SRCS}
	)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED
	elementary
	evas
	dlog
	glib-2.0
	capi-appfw-application
	capi-content-media-content
	ui-gadget-1
	efl-extension
	notification
	appsvc
	storage
	libtzplatform-config
)

FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS} -fvisibility=hidden")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fpie")
SET(CMAKE_C_FLAGS_RELEASE "-O2 -fpie")
SET(CMAKE_LDFLAGS "-Wl,-zdefs" )

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

#SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--as-needed")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--as-needed -pie")

ADD_LIBRARY(${PROJECT_NAME} SHARED ${UG_SRCS})
SET_TARGET_PROPERTIES(${PROJECT_NAME} PROPERTIES SOVERSION 0.0.1)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} "-ldl")

# EDJ
ADD_CUSTOM_COMMAND(OUTPUT attach-panel-gallery.edj
                COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_IMG_DIR}
				-sd  ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_SOUND_DIR}
                ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_EDC_DIR}/attach-panel-gallery.edc
				${CMAKE_BINARY_DIR}/attach-panel-gallery.edj
                DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_EDC_DIR}/attach-panel-gallery.edc
)

ADD_CUSTOM_COMMAND(OUTPUT attach-panel-gallery-images.edj
		COMMAND edje_cc -id ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_IMG_EDC_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_IMG_EDC_DIR}/attach-panel-gallery-images.edc ${CMAKE_BINARY_DIR}/attach-panel-gallery-images.edj
		DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_IMG_EDC_DIR}/attach-panel-gallery-images.edc
)

ADD_CUSTOM_TARGET(gallery_efl_edj_build DEPENDS attach-panel-gallery.edj attach-panel-gallery-images.edj)
ADD_DEPENDENCIES(${PROJECT_NAME} gallery_efl_edj_build)

INSTALL(FILES ${CMAKE_BINARY_DIR}/attach-panel-gallery.edj DESTINATION ${RESDIR}/edje/attach-panel-gallery)
INSTALL(FILES ${CMAKE_BINARY_DIR}/attach-panel-gallery-images.edj DESTINATION ${RESDIR}/edje/attach-panel-gallery)

INSTALL(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/res/${GE_IMG_C_DIR} DESTINATION ${RESDIR}/images/attach-panel-gallery)
INSTALL(TARGETS ${PROJECT_NAME} DESTINATION ${LIBDIR})

INSTALL(FILES attach-panel-gallery.xml DESTINATION ${TZ_SYS_RO_PACKAGES})

# i18n
ADD_SUBDIRECTORY(res/po)

# Install appcontrol icon
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/icon/attach-panel-gallery.png DESTINATION ${RESDIR}/images/attach-panel-gallery)


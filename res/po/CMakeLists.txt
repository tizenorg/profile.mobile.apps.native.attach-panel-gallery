# for i18n




#SET(POFILES bn.po en.po en_US.po gu.po hi.po kn.po ko_KR.po ml.po mr.po or.po pa.po si.po ta.po te.po)

SET(POFILES ar.po az.po bg.po bn.po ca.po cs.po da.po de.po el_GR.po en_PH.po en.po en_US.po es_ES.po es_US.po et.po eu.po fi.po fr_CA.po fr.po ga.po gl.po gu.po hi.po hr.po hu.po hy.po is.po it_IT.po ja_JP.po ka.po kk.po kn.po ko_KR.po lt.po lv.po mk.po ml.po mn_MN.po mr.po nb.po nl.po or.po pa.po pl.po pt_BR.po pt_PT.po ro.po ru_RU.po si.po sk.po sl.po sr.po sv.po ta.po te.po tr_TR.po uk.po uz.po zh_CN.po zh_HK.po zh_TW.po)

SET(MSGFMT "/usr/bin/msgfmt")
SET(LOCALEDIR "${CMAKE_INSTALL_PREFIX}/res/locale")

FOREACH(pofile ${POFILES})
	SET(pofile ${CMAKE_CURRENT_SOURCE_DIR}/${pofile})
	MESSAGE("PO: ${pofile}")
	GET_FILENAME_COMPONENT(absPofile ${pofile} ABSOLUTE)
	GET_FILENAME_COMPONENT(lang ${absPofile} NAME_WE)
	SET(moFile ${CMAKE_CURRENT_BINARY_DIR}/${lang}.mo)
	ADD_CUSTOM_COMMAND(
			OUTPUT ${moFile}
			COMMAND ${MSGFMT} -o ${moFile} ${absPofile}
			DEPENDS ${absPofile}
	)
	#INSTALL(FILES ${moFile} DESTINATION res/locale/${lang}/LC_MESSAGES RENAME ${PROJECT_NAME}.mo)
#Share mo files with Gallery UG.
	#INSTALL(FILES ${moFile} DESTINATION ${LOCALEDIR}/${lang}/LC_MESSAGES RENAME ${PROJECT_NAME}.mo)
	INSTALL(FILES ${moFile} DESTINATION ${LOCALEDIR}/${lang}/LC_MESSAGES RENAME attach-panel-gallery.mo)
	SET(moFiles ${moFiles} ${moFile})
ENDFOREACH(pofile)

MESSAGE(".mo files: ${moFiles}")
ADD_CUSTOM_TARGET(po ALL DEPENDS ${moFiles})

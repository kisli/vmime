
# Installing headers and preserving the directory structure
# Found here: http://www.semipol.de/archives/251
MACRO(INSTALL_HEADERS_WITH_DIRECTORY HEADER_LIST COMPONENT_NAME REMOVE_PREFIX)

	FOREACH(HEADER ${${HEADER_LIST}})
		# Without leading or trailing slash (eg. "vmime/net"), so that the
		# destination is already normalized (see policy CMP0177)
		GET_FILENAME_COMPONENT(DIR ${HEADER} DIRECTORY)
		FILE(RELATIVE_PATH DIR "${REMOVE_PREFIX}" "${DIR}")
		INSTALL(FILES ${HEADER} DESTINATION include/${DIR} COMPONENT ${COMPONENT_NAME})
	ENDFOREACH(HEADER)

ENDMACRO(INSTALL_HEADERS_WITH_DIRECTORY)


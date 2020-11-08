set("CPACK_DEBIAN_LANGUAGES_PACKAGE_NAME" "rhvoice-languages" PARENT_SCOPE)

macro(languageInDir languageName langDir)
	set(lang_var_name "ISO639_1_NAME2CODE_${languageName}")

	if(${lang_var_name})
		set(langCode "${${lang_var_name}}")
	else()
		message(FATAL_ERROR "Please set ${lang_var_name} to the code of the lang from https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes")
	endif()

	set(languagePackageName "RHVoice-language-${langCode}")
	string(REPLACE "-" "_" languagePackageFilteredName "${languagePackageName}")
	file(GLOB_RECURSE SRCFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${langDir}/*")
	add_custom_target(
		"${languagePackageFilteredName}"
		SOURCES "${SRCFILES}"
	)

	string(TOUPPER "${languagePackageFilteredName}" languagePackageFilteredNameUpper)
	cpack_add_component("${languagePackageFilteredName}" GROUP "languages" DISPLAY_NAME "${languageName}"
		DESCRIPTION "${languageName} language (${langCode}) for RHVoice"
	)
	list(APPEND CPACK_COMPONENTS_ALL "${languagePackageFilteredName}")

	set("CPACK_DEBIAN_${languagePackageFilteredNameUpper}_PACKAGE_NAME" "${languagePackageName}")
	set("CPACK_RPM_${languagePackageFilteredNameUpper}_PACKAGE_NAME" "${languagePackageName}")
	set("CPACK_DEBIAN_${languagePackageFilteredNameUpper}_PACKAGE_DEPENDS" "${CPACK_DEBIAN_CORE_PACKAGE_NAME}, RHVoice-language-${langCode}" PARENT_SCOPE)
	set("CPACK_DEBIAN_${languagePackageFilteredNameUpper}_PACKAGE_SECTION" "contrib" PARENT_SCOPE)

	install(DIRECTORY "${langDir}"
		DESTINATION "${languages_dir}"
		COMPONENT "${languagePackageFilteredName}"
		PATTERN "SConscript" EXCLUDE
		PATTERN "CMakeLists.txt" EXCLUDE
	)
	pass_through_cpack_vars()
endmacro()


#set("CPACK_DEBIAN_LANGUAGES_PACKAGE_NAME" "rhvoice-languages" PARENT_SCOPE)

cpack_add_component_group("languages"
	DISPLAY_NAME "Languages"
	DESCRIPTION "Different languages"
	PARENT_GROUP "data"
	EXPANDED
)

file(GLOB languagesDirzInThisDir
	LIST_DIRECTORIES true
	"${CMAKE_CURRENT_SOURCE_DIR}/*"
)
foreach(langDir IN LISTS languagesDirzInThisDir)
	if(IS_DIRECTORY "${langDir}")
		get_filename_component(languageName "${langDir}" NAME)
		if(EXISTS "${langDir}/CMakeLists.txt")
			add_subdirectory("${langDir}")
		else()
			languageInDir("${languageName}" "${langDir}" )
		endif()
	endif()
endforeach()

pass_through_cpack_vars()

set("CPACK_DEBIAN_VOICES_PACKAGE_NAME" "rhvoice-voices" PARENT_SCOPE)

macro(voiceInDir voiceName voiceDir)
	set(lang_var_name "VOICE_${voiceName}_LANG")
	set(voice_license_name "VOICE_${voiceName}_LICENSE")

	if(${lang_var_name})
		set(langCode "${${lang_var_name}}")
	else()
		message(FATAL_ERROR "Please set ${lang_var_name} to the code of the lang of the voice ${voiceName} from https://en.wikipedia.org/wiki/List_of_ISO_639-1_codes")
	endif()

	set(langNameVarName "ISO639_1_CODE2NAME_${langCode}")
	set(langName "${${langNameVarName}}")

	if(${voice_license_name})
		set(license "${${voice_license_name}}")
		set(licenseFileCandidate "${licenses_dir}/${license}.txt")
	else()
		set(license "lgpl-2.1")
		set(licenseFileCandidate "${CMAKE_SOURCE_DIR}/LICENSE.md")
	endif()

	set(languagePackageName "RHVoice-language-${langCode}")
	string(REPLACE "-" "_" languagePackageFilteredName "${languagePackageName}")

	set(voicesGroupName "RHVoice-voices-${langCode}")
	string(REPLACE "-" "_" voicesGroupFilteredName "${voicesGroupName}")
	string(TOUPPER "${voicesGroupFilteredName}" voicesGroupFilteredNameUpper)
	if(NOT DEFINED "CPACK_COMPONENT_${voicesGroupFilteredNameUpper}_GROUP")
		cpack_add_component_group("${voicesGroupFilteredName}"
			DISPLAY_NAME "${langName}"
			DESCRIPTION "Voices for ${langName} (${langCode}) language for RHVoice"
			PARENT_GROUP "voices"
			EXPANDED
		)
	endif()

	set(voicePackageName "RHVoice-voice-${langCode}-${voiceName}")
	string(REPLACE "-" "_" voicePackageFilteredName "${voicePackageName}")
	file(GLOB_RECURSE SRCFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${voiceDir}/*")
	add_custom_target(
		"${voicePackageFilteredName}"
		SOURCES "${SRCFILES}"
	)

	string(TOUPPER "${voicePackageFilteredName}" voicePackageFilteredNameUpper)

	cpack_add_component("${voicePackageFilteredName}" GROUP "${voicesGroupFilteredName}" DISPLAY_NAME "${voiceName}"
		DESCRIPTION "${voiceName} voice for ${langName} (${langCode}) language for RHVoice licensed under `${license}` license."
		DEPENDS "${languagePackageFilteredName}"
	)
	list(APPEND CPACK_COMPONENTS_ALL "${voicePackageFilteredName}")

	set("CPACK_DEBIAN_${voicePackageFilteredNameUpper}_PACKAGE_NAME" "${voicePackageName}")
	set("CPACK_RPM_${voicePackageFilteredNameUpper}_PACKAGE_NAME" "${voicePackageName}")
	set("CPACK_DEBIAN_${voicePackageFilteredNameUpper}_PACKAGE_DEPENDS" "${CPACK_DEBIAN_CORE_PACKAGE_NAME}, RHVoice-language-${langCode}" PARENT_SCOPE)
	set("CPACK_DEBIAN_${languagePackageNameUpper}_PACKAGE_SECTION" "contrib" PARENT_SCOPE)
	
	install(DIRECTORY "${voiceDir}"
		DESTINATION "${voices_dir}"
		COMPONENT "${voicePackageFilteredName}"
		PATTERN "SConscript" EXCLUDE
		PATTERN "CMakeLists.txt" EXCLUDE
	)

	if(EXISTS "${licenseFileCandidate}")
		install(FILES "${licenseFileCandidate}"
			DESTINATION "${common_doc_dir}/${voiceName}/"
			COMPONENT "${voicePackageFilteredName}"
			RENAME "copyright"
		)
	else()
		set(licenseDirCandidate "${voice_licenses_dir}/${voiceName}")
		if(EXISTS "${licenseDirCandidate}")
			file(GLOB_RECURSE licenseFiles RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${licenseDirCandidate}/*")
			list(LENGTH "${licenseFiles}" licenseFilesLen)
			if(licenseFilesLen EQUAL "1")
				install(FILES ${licenseFiles}
					DESTINATION "${common_doc_dir}/${voiceName}/"
					COMPONENT "${voicePackageFilteredName}"
					RENAME "copyright"
				)
			else()
				install(FILES ${licenseFiles}
					DESTINATION "${common_doc_dir}/${voiceName}/"
					COMPONENT "${voicePackageFilteredName}"
				)
			endif()
		else()
			message(FATAL_ERROR "License files are missing for the voice ${voiceName}!")
		endif()
	endif()

	pass_through_cpack_vars()
endmacro()


#set("CPACK_DEBIAN_VOICES_PACKAGE_NAME" "rhvoice-voices" PARENT_SCOPE)

cpack_add_component_group("voices"
	DISPLAY_NAME "Voices"
	DESCRIPTION "Different voices"
	PARENT_GROUP "data"
	EXPANDED
)

file(GLOB voicesDirzInThisDir
	LIST_DIRECTORIES true
	"${CMAKE_CURRENT_SOURCE_DIR}/*"
)
foreach(voiceDir IN LISTS voicesDirzInThisDir)
	if(IS_DIRECTORY "${voiceDir}")
		get_filename_component(voiceName "${voiceDir}" NAME)
		if(EXISTS "${voiceDir}/CMakeLists.txt")
			add_subdirectory("${voiceDir}")
		else()
			voiceInDir("${voiceName}" "${voiceDir}" )
		endif()
	endif()
endforeach()

pass_through_cpack_vars()

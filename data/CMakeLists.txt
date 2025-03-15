# Copyright (C) 2010, 2011, 2012, 2013, 2014, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com>

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
cpack_add_component_group("data")


set("ISO639_1_NAME2CODE_Russian" "ru")
set("ISO639_1_NAME2CODE_English" "en")
set("ISO639_1_NAME2CODE_Ukrainian" "uk")
set("ISO639_1_NAME2CODE_Brazilian" "br")
set("ISO639_1_NAME2CODE_Portuguese" "pt")
set("ISO639_1_NAME2CODE_Polish" "pl")
set("ISO639_1_NAME2CODE_Brazilian-Portuguese" "${ISO639_1_NAME2CODE_Portuguese}-BR")

set("ISO639_1_NAME2CODE_Esperanto" "eo")
set("ISO639_1_NAME2CODE_Georgian" "ka")
set("ISO639_1_NAME2CODE_Kyrgyz" "ky")
set("ISO639_1_NAME2CODE_Macedonian" "mk")
set("ISO639_1_NAME2CODE_Albanian" "sq")
set("ISO639_1_NAME2CODE_Tatar" "tt")
set("ISO639_1_NAME2CODE_Czech" "cs")
set("ISO639_1_NAME2CODE_Slovak" "sk")
set("ISO639_1_NAME2CODE_Uzbek" "uz")
set("ISO639_1_NAME2CODE_Serbian" "sr")
set("ISO639_1_NAME2CODE_Croatian" "hr")
set("ISO639_1_NAME2CODE_Spanish" "es")

function(createLangCodeToNameMapping)
	get_cmake_property(varsHere VARIABLES)
	foreach(varName ${varsHere})
		if(varName MATCHES "^ISO639_1_NAME2CODE_(.+)$")
			set("ISO639_1_CODE2NAME_${${varName}}" "${CMAKE_MATCH_1}" PARENT_SCOPE)
		endif()
	endforeach()
endfunction()
createLangCodeToNameMapping()

set("VOICE_talgat_LICENSE" "Proprietary, non-commercial") # Tatar
set("VOICE_natia_LICENSE" "Proprietary, personal use only") # Georgian
set("VOICE_Leticia-F123_LICENSE" "by-sa-4.0") # Brazilian Portuguese

set("VOICE_aleksandr_LANG" "ru")
set("VOICE_aleksandr-hq_LANG" "ru")
set("VOICE_anna_LANG" "ru")
set("VOICE_vitaliy_LANG" "ru")
set("VOICE_vitaliy-ng_LANG" "ru")
set("VOICE_umka_LANG" "ru")
set("VOICE_yuriy_LANG" "ru")
set("VOICE_arina_LANG" "ru")
set("VOICE_artemiy_LANG" "ru")
set("VOICE_evgeniy-rus_LANG" "ru")
set("VOICE_elena_LANG" "ru")
set("VOICE_irina_LANG" "ru")
set("VOICE_pavel_LANG" "ru")
set("VOICE_victoria_LANG" "ru")
set("VOICE_tatiana_LANG" "ru")
set("VOICE_timofey_LANG" "ru")
set("VOICE_mikhail_LANG" "ru")
set("VOICE_vsevolod_LANG" "ru")

set("VOICE_evgeniy-eng_LANG" "en")
set("VOICE_lyubov_LANG" "en")
set("VOICE_alan_LANG" "en")
set("VOICE_bdl_LANG" "en")
set("VOICE_clb_LANG" "en")
set("VOICE_ksp_LANG" "en")
set("VOICE_slt_LANG" "en")

set("VOICE_volodymyr_LANG" "uk")
set("VOICE_marianna_LANG" "uk")
set("VOICE_anatol_LANG" "uk")
set("VOICE_natalia_LANG" "uk")

set("VOICE_Leticia-F123_LANG" "${ISO639_1_NAME2CODE_Brazilian-Portuguese}")

set("VOICE_spomenka_LANG" "eo")

set("VOICE_natia_LANG" "ka")

set("VOICE_azamat_LANG" "ky")
set("VOICE_nazgul_LANG" "ky")

set("VOICE_kiko_LANG" "mk")
set("VOICE_suze_LANG" "mk")

set("VOICE_hana_LANG" "sq")

set("VOICE_talgat_LANG" "tt")

set("VOICE_alicja_LANG" "pl")
set("VOICE_cezary_LANG" "pl")
set("VOICE_magda_LANG" "pl")
set("VOICE_michal_LANG" "pl")
set("VOICE_natan_LANG" "pl")
set("VOICE_zdenek_LANG" "cs")
set("VOICE_radek_LANG" "cs")
set("VOICE_ondro_LANG" "sk")
set("VOICE_jasietka_LANG" "sk")
set("VOICE_sevinch_LANG" "uz")
set("VOICE_islom_LANG" "uz")
set("VOICE_dilnavoz_LANG" "uz")
set("VOICE_dragana_LANG" "sr")
set("VOICE_karmela_LANG" "hr")
set("VOICE_mateo_LANG" "es")
set("VOICE_seva_LANG" "ru")

cpack_add_component_group("data"
	DISPLAY_NAME "Data files"
	DESCRIPTION "Various data required for synthesis"
	EXPANDED
)

add_subdirectory("languages")
add_subdirectory("voices")
pass_through_cpack_vars()

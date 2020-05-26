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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
cpack_add_component_group("data")


set("ISO639_1_NAME2CODE_Russian" "ru")
set("ISO639_1_NAME2CODE_English" "en")
set("ISO639_1_NAME2CODE_Ukrainian" "uk")
set("ISO639_1_NAME2CODE_Brazilian" "br")
set("ISO639_1_NAME2CODE_Portuguese" "pt")
set("ISO639_1_NAME2CODE_Brazilian-Portuguese" "${ISO639_1_NAME2CODE_Portuguese}-BR")

set("ISO639_1_NAME2CODE_Esperanto" "eo")
set("ISO639_1_NAME2CODE_Georgian" "ka")
set("ISO639_1_NAME2CODE_Kyrgyz" "ky")
set("ISO639_1_NAME2CODE_Tatar" "tt")


set("VOICE_talgat_LICENSE" "Proprietary, non-commercial") # Tatar
set("VOICE_natia_LICENSE" "Proprietary, personal use only") # Georgian
set("VOICE_Leticia-F123_LICENSE" "by-nc-sa-4.0") # Brazilian Portuguese

set("VOICE_aleksandr_LANG" "ru")
set("VOICE_anna_LANG" "ru")
set("VOICE_elena_LANG" "ru")
set("VOICE_irina_LANG" "ru")

set("VOICE_alan_LANG" "en")
set("VOICE_bdl_LANG" "en")
set("VOICE_clb_LANG" "en")
set("VOICE_slt_LANG" "en")

set("VOICE_anatol_LANG" "uk")
set("VOICE_natalia_LANG" "uk")

set("VOICE_Leticia-F123_LANG" "${ISO639_1_NAME2CODE_Brazilian-Portuguese}")

set("VOICE_spomenka_LANG" "eo")

set("VOICE_natia_LANG" "ka")

set("VOICE_azamat_LANG" "ky")
set("VOICE_nazgul_LANG" "ky")

set("VOICE_talgat_LANG" "tt")

add_subdirectory("languages")
add_subdirectory("voices")
pass_through_cpack_vars()

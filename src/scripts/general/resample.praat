# Copyright (C) 2012  Olga Yakovleva <yakovleva.o.v@gmail.com>

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

form Parameters
word Input_directory recordings
word Output_directory wav
integer Sample_rate 0
real Normalization_factor 0
endform
Create Strings as file list... files 'input_directory$'/*.wav
nfiles=Get number of strings
for i to nfiles
select Strings files
file_name$=Get string... i
sound=Read from file... 'input_directory$'/'file_name$'
if sample_rate>0
sound=Resample... sample_rate 50
endif
if normalization_factor>0
Scale peak... normalization_factor
endif
Save as WAV file... 'output_directory$'/'file_name$'
select all
minus Strings files
Remove
endfor

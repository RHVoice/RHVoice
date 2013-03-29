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
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

form Parameters
word Input_file recording.wav
word Output_directory .
word Output_file_name recording
integer Target_sample_rate 16000
boolean Trim_silences 0
real volume 1
endform
Read from file... 'input_file$'
source=Convert to mono
if trim_silences
To TextGrid (silences)... 100 0 -25 0.2 0.05 silent sounding
num_intervals=Get number of intervals... 1
start=0
label$=Get label of interval... 1 1
if label$=="silent"
start_of_speech=Get end point... 1 1
start=start_of_speech-0.2
endif
end=Get end point... 1 num_intervals
label$=Get label of interval... 1 num_intervals
if label$=="silent"
end_of_speech=Get start point... 1 num_intervals
end=end_of_speech+0.2
endif
select source
source=Extract part... start end rectangular 1 no
endif
source_sample_rate=Get sampling frequency
if source_sample_rate!=target_sample_rate
Resample... target_sample_rate 50
endif
Scale peak... volume
Save as raw 16-bit little-endian file... 'output_directory$'/data/raw/'output_file_name$'.raw
if target_sample_rate!=16000
select source
Resample... 16000 50
Scale peak... volume
endif
Save as WAV file... 'output_directory$'/data/wav/'output_file_name$'.wav

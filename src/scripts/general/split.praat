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
word Input_file recording.wav
word Output_directory wav
positive Silence_length 1.5
endform
index=1
long_sound=Open long sound file... 'input_file$'
start_time=Get start time
end_time=Get end time
fragment_start=start_time
while fragment_start<end_time
fragment_end=min(fragment_start+60,end_time)
sound=Extract part... fragment_start fragment_end yes
intensity=To Intensity... 100 0 yes
silences=To TextGrid (silences)... -25 silence_length 0.1 silent sounding
n=Get number of intervals... 1
for i to n
select silences
label$=Get label of interval... 1 i
if ((label$="silent") and (i>1)) or (n=1)
start=Get start point... 1 i
end=Get end point... 1 i
if n=1
fragment_end=end
elif end=end_time
fragment_end=end_time
else
fragment_end=(start+end)/2.0
endif
select sound
Extract part... fragment_start fragment_end rectangular 1 yes
str_index$="'index'"
output_file$=output_directory$+"/utt_"+right$("000000"+str_index$,6)+".wav"
Save as WAV file... 'output_file$'
Remove
fragment_start=fragment_end
index=index+1
endif
endfor
select intensity
Remove
select silences
Remove
select sound
Remove
select long_sound
endwhile

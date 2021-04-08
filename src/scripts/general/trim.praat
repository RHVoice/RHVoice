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
endform

Read from file... 'input_file$'
Extract one channel... 1
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
printline 'start''tab$''end'

# Copyright (C) 2012, 2013, 2017  Olga Yakovleva <yakovleva.o.v@gmail.com>

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
positive Lower_f0 40
positive Upper_f0 500
real Voicing_threshold 0.45
boolean Autocorrelation 1
endform
sound=Read from file... 'input_file$'
sample_rate=Get sampling frequency
shift=floor(0.005*sample_rate)/sample_rate
if autocorrelation
pitch=To Pitch (ac)... 0.001 lower_f0 15 no 0.03 voicing_threshold 0.01 0.35 0.14 upper_f0
else
pitch=To Pitch (cc)... 0.001 lower_f0 15 no 0.03 voicing_threshold 0.01 0.35 0.14 upper_f0
endif
duration=Get total duration
nvalues=duration/shift
for i to nvalues
time=(i-1)*shift
f0=Get value at time... time Hertz Linear
if f0==undefined
f0=0
endif
printline 'f0'
endfor

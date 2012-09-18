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
positive Lower_f0 40
positive Upper_f0 500
real Voicing_threshold 0.45
boolean Print_log_f0 0
endform
sound0=Read from file... 'input_file$'
nsamples=Get number of samples
nchannels=Get number of channels
sample_rate=Get sampling frequency
frame_shift_in_samples=floor(sample_rate*0.005)
frame_shift_in_seconds=frame_shift_in_samples/sample_rate
half_window_length_in_samples=ceiling(1.5/lower_f0*sample_rate)
half_window_length_in_seconds=half_window_length_in_samples/sample_rate
remainder=nsamples mod frame_shift_in_samples
if remainder==0
remainder=frame_shift_in_samples
endif
final_silence_length=(half_window_length_in_samples -remainder)/sample_rate
initial_silence=Create Sound from formula... initial_silence nchannels 0 half_window_length_in_seconds sample_rate 0
final_silence=Create Sound from formula... final_silence nchannels 0 final_silence_length sample_rate 0
select sound0
plus final_silence
sound1=Concatenate
select initial_silence
plus sound1
sound=Concatenate
pitch=To Pitch (ac)... frame_shift_in_seconds lower_f0 15 no 0.03 voicing_threshold 0.01 0.35 0.14 upper_f0
nframes=Get number of frames
for i to nframes
f0=Get value in frame... i hertz
if f0=undefined
if print_log_f0
printline -10000000000
else
printline 0
endif
else
if print_log_f0
lf0=ln(f0)
printline 'lf0'
else
printline 'f0'
endif
endif
endfor

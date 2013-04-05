/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

package com.github.olga_yakovleva.rhvoice;

public final class SynthesisParameters
{
    private VoiceInfo main_voice=null;
    private VoiceInfo extra_voice=null;
    private boolean ssml_mode=false;
    private double rate=0;
    private double pitch=0;
    private double volume=0;

    public void setMainVoice(VoiceInfo voice)
    {
        main_voice=voice;
    }

    public VoiceInfo getMainVoice()
    {
        return main_voice;
    }

    public void setExtraVoice(VoiceInfo voice)
    {
        extra_voice=voice;
    }

    public VoiceInfo getExtraVoice()
    {
        return extra_voice;
    }

    public void setSSMLMode(boolean mode)
    {
        ssml_mode=mode;
    }

    public boolean getSSMLMode()
    {
        return ssml_mode;
    }

    public void setRate(double value)
    {
        rate=Math.max(-1,Math.min(1,value));
    }

    public double getRate()
    {
        return rate;
    }

    public void setPitch(double value)
    {
        pitch=Math.max(-1,Math.min(1,value));
    }

    public double getPitch()
    {
        return pitch;
    }

    public void setVolume(double value)
    {
        volume=Math.max(-1,Math.min(1,value));
    }

    public double getVolume()
    {
        return volume;
    }
}

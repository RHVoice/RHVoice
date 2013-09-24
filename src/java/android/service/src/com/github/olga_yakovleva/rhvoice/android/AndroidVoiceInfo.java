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

package com.github.olga_yakovleva.rhvoice.android;

import com.github.olga_yakovleva.rhvoice.VoiceInfo;

public final class AndroidVoiceInfo
{
    private final VoiceInfo source;

    public AndroidVoiceInfo(VoiceInfo source)
    {
        this.source=source;
    }

    public VoiceInfo getSource()
    {
        return source;
    }

    public String getLanguage()
    {
        return source.getLanguage().getAlpha3Code();
    }

    public String getCountry()
    {
        return source.getLanguage().getAlpha3CountryCode();
    }

    public String getVariant()
    {
        return source.getName();
    }

    public int getSupportLevel(String language,String country,String variant)
    {
        int result=0;
        if((language!=null)&&language.equals(getLanguage()))
            {
                ++result;
                if((country!=null)&&country.equals(getCountry()))
                    {
                        ++result;
                        if((variant!=null)&&variant.equals(getVariant()))
                            ++result;
                    }
            }
        return result;
    }

    public boolean matches(String voice)
    {
        final String[] parts=voice.split("-");
        if((parts.length==0)||(parts.length>3))
            return false;
        final String language=parts[0];
        String country="";
        String variant="";
        if(parts.length>1)
            {
                country=parts[1];
                if(parts.length==3)
                    variant=parts[2];
            }
        return (getSupportLevel(language,country,variant)==parts.length);
    }

    @Override
    public String toString()
    {
        return (getLanguage()+"-"+getCountry()+"-"+getVariant());
    }
}

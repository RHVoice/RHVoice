/* Copyright (C) 2017, 2018, 2019, 2020  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

package com.github.olga_yakovleva.rhvoice.android;

import android.content.Context;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.HashMap;
import android.text.TextUtils;

public final class LanguagePack extends DataPack
{
    private final String code;
    private final String oldCode;
    private final String countryCode;
    private final String oldCountryCode;
    private final boolean showCountry;
    private final List<VoicePack> voices=new ArrayList<VoicePack>();
    private final Map<String,VoicePack> index=new HashMap<String,VoicePack>();
    private final Map<String,VoicePack> idIndex=new HashMap<String,VoicePack>();
    private VoicePack defaultVoice;

    public LanguagePack(String name,String code,String oldCode,String countryCode,String oldCountryCode,boolean showCountry,int format,int revision,byte[] checksum)
    {
        this(name,code,oldCode,countryCode,oldCountryCode,showCountry,format,revision,checksum,null,null);
}

    public LanguagePack(String name,String code,String oldCode,String countryCode,String oldCountryCode,boolean showCountry,int format,int revision,byte[] checksum,String altLink,String tempLink)
    {
        super(name,format,revision,checksum,altLink,tempLink);
        this.code=code;
        this.oldCode=oldCode;
        this.countryCode=countryCode;
        this.oldCountryCode=oldCountryCode;
        this.showCountry=showCountry;
}

    public String getType()
    {
        return "language";
}

    public String getDisplayName()
    {
        Locale loc=showCountry?new Locale(oldCode,oldCountryCode):new Locale(oldCode);
        return loc.getDisplayName();
}

    protected String getBaseFileName()
    {
        return String.format("RHVoice-language-%s",getName());
}

    public List<VoicePack> getVoices()
    {
        return voices;
}

    public VoicePack getDefaultVoice()
    {
        return defaultVoice;
}

    public LanguagePack addVoice(VoicePack voice)
    {
        voices.add(voice);
        index.put(voice.getName(),voice);
        idIndex.put(voice.getId(),voice);
        if(defaultVoice==null)
            defaultVoice=voice;
        return this;
}

    public LanguagePack addDefaultVoice(VoicePack voice)
    {
        addVoice(voice);
        defaultVoice=voice;
        return this;
}

    @Override
    public boolean getEnabled(Context context)
    {
        if(code.equals("eng"))
            return true;
        for(VoicePack voice: voices)
            {
                if(voice.getEnabled(context))
                    return true;
}
        return false;
}

    public List<String> getPaths(Context context)
    {
        List<String> paths=new ArrayList<String>();
        String languagePath=getPath(context);
        if(languagePath==null)
            return paths;
        for(VoicePack voice: voices)
            {
                if(!voice.getEnabled(context))
                    continue;
                String voicePath=voice.getPath(context);
                if(voicePath!=null)
                    paths.add(voicePath);
}
        if(paths.isEmpty()&&code!="eng")
            return paths;
        paths.add(languagePath);
        return paths;
}

    public String getCode()
    {
        return code;
}

    public String getCountryCode()
    {
        return countryCode;
}

    public String getTag()
    {
        if(TextUtils.isEmpty(countryCode))
            return code;
        else
            return (code+"-"+countryCode);
}

    @Override
    protected void notifyDownloadStart(IDataSyncCallback callback)
    {
        callback.onLanguageDownloadStart(this);
}

    @Override
    protected void notifyDownloadDone(IDataSyncCallback callback)
    {
        callback.onLanguageDownloadDone(this);
}

    @Override
    protected void notifyInstallation(IDataSyncCallback callback)
    {
        callback.onLanguageInstallation(this);
}

    @Override
    protected void notifyRemoval(IDataSyncCallback callback)
    {
        callback.onLanguageRemoval(this);
}

    public VoicePack findVoice(String name)
    {
        return index.get(name);
}

    public VoicePack findVoiceById(String id)
    {
        return idIndex.get(id);
}
}

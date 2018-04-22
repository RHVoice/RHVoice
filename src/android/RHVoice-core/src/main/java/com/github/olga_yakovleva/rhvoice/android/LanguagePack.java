/* Copyright (C) 2017, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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
    private final String countryCode;
    private final List<VoicePack> voices=new ArrayList<VoicePack>();
    private final Map<String,VoicePack> index=new HashMap<String,VoicePack>();

    public LanguagePack(String name,String code,String countryCode,int format,int revision)
    {
        super(name,format,revision);
        this.code=code;
        this.countryCode=countryCode;
}

    public LanguagePack(String name,String code,String countryCode,int format,int revision,String altLink,String tempLink)
    {
        super(name,format,revision,altLink,tempLink);
        this.code=code;
        this.countryCode=countryCode;
}

    public String getType()
    {
        return "language";
}

    public String getDisplayName()
    {
        Locale loc=new Locale(code);
        return loc.getDisplayLanguage();
}

    protected String getBaseFileName()
    {
        return String.format("RHVoice-language-%s",getName());
}

    public List<VoicePack> getVoices()
    {
        return voices;
}

    public LanguagePack addVoice(VoicePack voice)
    {
        voices.add(voice);
        index.put(voice.getName(),voice);
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

    @Override
    public boolean sync(Context context,IDataSyncCallback callback)
    {
        boolean done=true;
        for(VoicePack voice: voices)
            {
                if(!voice.sync(context,callback))
                    done=false;
}
        if(callback.isStopped())
            return true;
        done&=super.sync(context,callback);
        if(callback.isStopped())
            return true;
        return done;
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

    @Override
    public long getSyncFlags(Context context)
    {
        long flags=super.getSyncFlags(context);
        for(VoicePack voice: voices)
            {
                flags|=voice.getSyncFlags(context);
}
        return flags;
}
}

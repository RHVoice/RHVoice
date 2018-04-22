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

public final class VoicePack extends DataPack
{
    private final LanguagePack lang;

    public VoicePack(String name,LanguagePack lang,int format,int revision)
    {
        super(name,format,revision);
        this.lang=lang;
}

    public VoicePack(String name,LanguagePack lang,int format,int revision,String altLink,String tempLink)
    {
        super(name,format,revision,altLink,tempLink);
        this.lang=lang;
}

    public String getType()
    {
        return "voice";
}

    public String getDisplayName()
    {
        return getName();
}

    protected String getBaseFileName()
    {
        return String.format("RHVoice-voice-%s-%s",lang.getName(),getName());
}

    private String getEnabledKey()
    {
        return String.format("voice.%s.enabled",getName().toLowerCase());
}

    @Override
    public final boolean getEnabled(Context context)
    {
        return getPrefs(context).getBoolean(getEnabledKey(),getPackageInfo(context)!=null);
}

    public final void setEnabled(Context context,boolean value)
    {
        getPrefs(context).edit().putBoolean(getEnabledKey(),value).apply();
}

    @Override
    protected void notifyDownloadStart(IDataSyncCallback callback)
    {
        callback.onVoiceDownloadStart(this);
}

    @Override
    protected void notifyDownloadDone(IDataSyncCallback callback)
    {
        callback.onVoiceDownloadDone(this);
    }

    @Override
    protected void notifyInstallation(IDataSyncCallback callback)
    {
        callback.onVoiceInstallation(this);
    }

    @Override
    protected void notifyRemoval(IDataSyncCallback callback)
    {
        callback.onVoiceRemoval(this);
    }

    public LanguagePack getLanguage()
    {
        return lang;
}
}

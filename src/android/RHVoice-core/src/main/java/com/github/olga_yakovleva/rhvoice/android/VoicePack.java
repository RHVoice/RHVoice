/* Copyright (C) 2017, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import android.content.Context;
import android.content.Intent;

public final class VoicePack extends DataPack
{
    private final LanguagePack lang;

    public VoicePack(String id,String name,LanguagePack lang,int format,int revision,byte[] checksum)
    {
        super(id,name,format,revision,checksum);
        this.lang=lang;
}

    public VoicePack(String id,String name,LanguagePack lang,int format,int revision,byte[] checksum,String altLink,String tempLink)
    {
        super(id,name,format,revision,checksum,altLink,tempLink);
        this.lang=lang;
}

    public VoicePack(String name,LanguagePack lang,int format,int revision,byte[] checksum)
    {
        this(null,name,lang,format,revision,checksum);
}

    public VoicePack(String name,LanguagePack lang,int format,int revision,byte[] checksum,String altLink,String tempLink)
    {
        this(null,name,lang,format,revision,checksum,altLink,tempLink);
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
        return String.format("voice.%s.enabled",getId());
}

    @Override
    public final boolean getEnabled(Context context)
    {
        return getPrefs(context).getBoolean(getEnabledKey(),getPackageInfo(context)!=null);
}

    public final void setEnabled(Context context,boolean value)
    {
        boolean langEnabled=lang.getEnabled(context);
        boolean oldValue=getEnabled(context);
        getPrefs(context).edit().putBoolean(getEnabledKey(),value).apply();
        if(value==oldValue)
            return;
            LocalBroadcastManager.getInstance(context).sendBroadcast(new Intent(RHVoiceService.ACTION_CHECK_DATA));
            if(lang.getEnabled(context)!=langEnabled)
                lang.scheduleSync(context,true);
            scheduleSync(context,true);
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

    @Override
    protected androidx.work.Data.Builder setWorkInput(androidx.work.Data.Builder b)
    {
        lang.setWorkInput(b);
        return super.setWorkInput(b);
}
}

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

import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import com.evernote.android.job.Job;
import java.util.List;

public final class DataSyncJob extends Job implements IDataSyncCallback
{
    public static final String JOB_TAG="data_sync_job_tag";

    public static final String ACTION_DATA_STATE_CHANGED="com.github.olga_yakovleva.rhvoice.android.action.data_state_changed";
    public static final String ACTION_VOICE_DOWNLOADED="com.github.olga_yakovleva.rhvoice.android.action.voice_downloaded";
    public static final String ACTION_VOICE_INSTALLED="com.github.olga_yakovleva.rhvoice.android.action.voice_installed";
    public static final String ACTION_VOICE_REMOVED="com.github.olga_yakovleva.rhvoice.android.action.voice_removed";

    private static final String TAG="RHVoice.DataSyncJob";
    private Boolean connected;
    private long flags;

    @Override
        protected Job.Result onRunJob(Job.Params params)
    {
        flags=params.getExtras().getLong("flags",0);
        if((flags&SyncFlags.NETWORK)==0)
            connected=false;
        if(BuildConfig.DEBUG)
            Log.i(TAG,"Running, flags="+flags);
        List<String> oldPaths=Data.getPaths(getContext());
        boolean done=Data.sync(getContext(),this);
        List<String> newPaths=Data.getPaths(getContext());
        if(!oldPaths.equals(newPaths))
            LocalBroadcastManager.getInstance(getContext()).sendBroadcast(new Intent(ACTION_DATA_STATE_CHANGED));
        if(BuildConfig.DEBUG)
            Log.i(TAG,"Finished this run, done="+done);
        return done?Result.SUCCESS:Result.RESCHEDULE;
}

    public boolean isConnected()
    {
        if(connected==null)
            connected=isRequirementNetworkTypeMet();
        return connected;
}

    public void onLanguageDownloadStart(LanguagePack language)
    {

}

    public void onLanguageDownloadDone(LanguagePack language)
    {

}

    public void onLanguageInstallation(LanguagePack language)
    {

}

    public void onLanguageRemoval(LanguagePack language)
    {

}

    public void onVoiceDownloadStart(VoicePack voice)
    {

}

    public void onVoiceDownloadDone(VoicePack voice)
    {
        Intent event=new Intent(ACTION_VOICE_DOWNLOADED);
        event.putExtra("name",voice.getName());
        LocalBroadcastManager.getInstance(getContext()).sendBroadcast(event);
}

    public void onVoiceInstallation(VoicePack voice)
    {
        Intent event=new Intent(ACTION_VOICE_INSTALLED);
        event.putExtra("name",voice.getName());
        LocalBroadcastManager.getInstance(getContext()).sendBroadcast(event);
}

    public void onVoiceRemoval(VoicePack voice)
    {
        Intent event=new Intent(ACTION_VOICE_REMOVED);
        event.putExtra("name",voice.getName());
        LocalBroadcastManager.getInstance(getContext()).sendBroadcast(event);
}

    public void onNetworkError()
    {
}

    public boolean isStopped()
    {
        return isCanceled();
}
}

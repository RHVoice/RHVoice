/* Copyright (C) 2017  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import java.util.List;


class DataSyncAction implements Runnable,IDataSyncCallback
{
    private static final String TAG="RHVoice.DataSyncAction";
    private final Context context;
    private Boolean connected;
    private boolean networkError;

    public DataSyncAction(Context context)
    {
        this.context=context;
}

    public void run()
    {
        List<String> oldPaths=Data.getPaths(context);
        Data.sync(context,this);
        handleConnectivity();
        List<String> newPaths=Data.getPaths(context);
        if(!oldPaths.equals(newPaths))
            LocalBroadcastManager.getInstance(context).sendBroadcast(new Intent(DataService.ACTION_DATA_STATE_CHANGED));
        LocalBroadcastManager.getInstance(context).sendBroadcast(new Intent(DataService.ACTION_DATA_SYNC_FINISHED));
}

    public boolean isConnected()
    {
        if(connected==null)
            connected=checkConnectivity();
        return connected;
}

        private boolean checkConnectivity()
    {
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Checking connectivity");
        ConnectivityManager cm=(ConnectivityManager)context.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo ni=cm.getActiveNetworkInfo();
        if(ni==null)
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"No active network");
                return false;
            }
        if(!ni.isConnected())
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"Active network not connected");
                return false;
            }
        if(getWifiOnly()&&ni.getType()!=ConnectivityManager.TYPE_WIFI)
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"Active network not Wi-fi");
                return false;
            }
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Connected");
        return true;
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

    public void onVoiceDownloadStart(VoicePack voice)
    {

}

    public void onVoiceDownloadDone(VoicePack voice)
    {
        Intent event=new Intent(DataService.ACTION_VOICE_DOWNLOADED);
        event.putExtra("name",voice.getName());
        LocalBroadcastManager.getInstance(context).sendBroadcast(event);
}

    public void onVoiceInstallation(VoicePack voice)
    {
        Intent event=new Intent(DataService.ACTION_VOICE_INSTALLED);
        event.putExtra("name",voice.getName());
        LocalBroadcastManager.getInstance(context).sendBroadcast(event);
}

    public void onNetworkError()
    {
        networkError=true;
}

    static String getWifiOnlyKey()
    {
        return "wifi_only";
}

    private boolean getWifiOnly()
    {
        return DataPack.getPrefs(context).getBoolean(getWifiOnlyKey(),true);
}

    private void handleConnectivity()
    {
        if(BuildConfig.DEBUG)
            {
                if(networkError)
                    Log.v(TAG,"Enabling connectivity action receiver");
                else
                    Log.v(TAG,"Disabling connectivity action receiver");
}
        PackageManager pm=context.getPackageManager();
        ComponentName cn=new ComponentName(context,ConnectivityActionReceiver.class);
        int newState=networkError?PackageManager.COMPONENT_ENABLED_STATE_ENABLED:PackageManager.COMPONENT_ENABLED_STATE_DISABLED;
        pm.setComponentEnabledSetting(cn,newState,PackageManager.DONT_KILL_APP);
}
}

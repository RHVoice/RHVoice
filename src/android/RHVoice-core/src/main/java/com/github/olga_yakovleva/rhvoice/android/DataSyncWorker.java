/* Copyright (C) 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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
import android.content.Intent;

import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import androidx.work.WorkerParameters;

public class DataSyncWorker extends DataWorker implements IDataSyncCallback {
    public static final String ACTION_VOICE_DOWNLOADED = "com.github.olga_yakovleva.rhvoice.android.action.voice_downloaded";
    public static final String ACTION_VOICE_INSTALLED = "com.github.olga_yakovleva.rhvoice.android.action.voice_installed";
    public static final String ACTION_VOICE_REMOVED = "com.github.olga_yakovleva.rhvoice.android.action.voice_removed";
    public static final String ACTION_LANGUAGE_INSTALLED = "org.rhvoice.action.voice_installed";
    public static final String ACTION_LANGUAGE_REMOVED = "org.rhvoice.action.voice_removed";

    public boolean isConnected() {
        return false;
    }

    public void onLanguageDownloadStart(LanguagePack language) {

    }

    public void onLanguageDownloadDone(LanguagePack language) {

    }

    public void onLanguageInstallation(LanguagePack language) {
        Intent event = new Intent(ACTION_LANGUAGE_INSTALLED);
        event.putExtra("name", language.getName());
        LocalBroadcastManager.getInstance(getApplicationContext()).sendBroadcast(event);
    }

    public void onLanguageRemoval(LanguagePack language) {
        Intent event = new Intent(ACTION_LANGUAGE_REMOVED);
        event.putExtra("name", language.getName());
        LocalBroadcastManager.getInstance(getApplicationContext()).sendBroadcast(event);
    }

    public void onVoiceDownloadStart(VoicePack voice) {

    }

    public void onVoiceDownloadDone(VoicePack voice) {
        Intent event = new Intent(ACTION_VOICE_DOWNLOADED);
        event.putExtra("name", voice.getName());
        LocalBroadcastManager.getInstance(getApplicationContext()).sendBroadcast(event);
    }

    public void onVoiceInstallation(VoicePack voice) {
        Intent event = new Intent(ACTION_VOICE_INSTALLED);
        event.putExtra("name", voice.getName());
        LocalBroadcastManager.getInstance(getApplicationContext()).sendBroadcast(event);
    }

    public void onVoiceRemoval(VoicePack voice) {
        Intent event = new Intent(ACTION_VOICE_REMOVED);
        event.putExtra("name", voice.getName());
        LocalBroadcastManager.getInstance(getApplicationContext()).sendBroadcast(event);
    }

    public final boolean isTaskStopped() {
        return isStopped();
    }

    public DataSyncWorker(Context context, WorkerParameters params) {
        super(context, params);
    }

    protected final boolean doSync(DataPack p) {
        boolean done = p.sync(getApplicationContext(), this);
        if (done)
            LocalBroadcastManager.getInstance(getApplicationContext()).sendBroadcast(new Intent(RHVoiceService.ACTION_CHECK_DATA));
        return done;
    }

    @Override
    protected Result doWork(DataPack p) {
        doSync(p);
        return (p.getSyncFlag(getApplicationContext()) != SyncFlags.LOCAL) ? Result.success(getInputData()) : Result.retry();
    }
}

/* Copyright (C) 2013, 2014, 2016, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

package com.github.olga_yakovleva.rhvoice.android;


import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;

import com.github.olga_yakovleva.rhvoice.VoiceInfo;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import android.util.Log;

public final class CheckTTSData extends Activity {
    private static final String TAG = "RHVoiceCheckDataActivity";
    private DataManager dm;
    private ArrayList<String> installedLanguages = new ArrayList<String>();
    private ArrayList<String> notInstalledLanguages = new ArrayList<String>();

    private void checkData() {
        installedLanguages.clear();
        notInstalledLanguages.clear();
        for (VoiceAccent accent : dm.getAccents()) {
            boolean installed = false;
            boolean notInstalled = false;
            for (VoicePack voice : accent.getVoices()) {
                if (voice.getEnabled(this) && voice.isUpToDate(this))
                    installed = true;
                else
                    notInstalled = true;
            }
            String tag = accent.getTag().toString3();
            if (installed) {
                if (BuildConfig.DEBUG)
                    Log.v(TAG, "Installed language: " + tag);
                installedLanguages.add(tag);
            }
            if (notInstalled)
                notInstalledLanguages.add(tag);
        }
        dm.scheduleSync(this, false);
    }

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        dm = Repository.get().createDataManager();
        if (BuildConfig.DEBUG)
            Log.v(TAG, "checking data");
        checkData();
        Intent resultIntent = new Intent();
        resultIntent.putStringArrayListExtra(TextToSpeech.Engine.EXTRA_AVAILABLE_VOICES, installedLanguages);
        resultIntent.putStringArrayListExtra(TextToSpeech.Engine.EXTRA_UNAVAILABLE_VOICES, notInstalledLanguages);
        setResult(TextToSpeech.Engine.CHECK_VOICE_DATA_PASS, resultIntent);
        finish();
    }
}

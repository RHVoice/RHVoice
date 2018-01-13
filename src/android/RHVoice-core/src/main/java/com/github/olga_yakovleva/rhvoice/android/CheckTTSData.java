/* Copyright (C) 2013, 2014, 2016  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

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

public final class CheckTTSData extends Activity
{
    private static final String TAG="RHVoiceCheckDataActivity";
    private DataManager dm;
    private ArrayList<String> languageTags=new ArrayList<String>();

    private void checkData()
    {
        if(BuildConfig.DEBUG)
            Log.i(TAG,"Checking");
        DataManager dm=new DataManager(this);
        dm.checkFiles();
        dm.checkVoices();
        List<VoiceInfo> voices=dm.getVoices();
        for(VoiceInfo voice: voices)
            {
                String languageTag=voice.getLanguage().getTag3();
                if(languageTags.contains(languageTag))
                    continue;
                languageTags.add(languageTag);
                if(BuildConfig.DEBUG)
                    Log.i(TAG,languageTag);
}
        Collections.sort(languageTags);
        if(!dm.isUpToDate())
            startService(new Intent(this,DataService.class));
}

    @Override
    protected void onCreate(Bundle state)
    {
        super.onCreate(state);
        checkData();
        Intent resultIntent=new Intent();
        if(!languageTags.isEmpty())
            {
                resultIntent.putStringArrayListExtra(TextToSpeech.Engine.EXTRA_AVAILABLE_VOICES,languageTags);
                resultIntent.putStringArrayListExtra(TextToSpeech.Engine.EXTRA_UNAVAILABLE_VOICES,new ArrayList<String>());
                setResult(TextToSpeech.Engine.CHECK_VOICE_DATA_PASS,resultIntent);
}
        else
            setResult(TextToSpeech.Engine.CHECK_VOICE_DATA_FAIL,resultIntent);
        finish();
}
}

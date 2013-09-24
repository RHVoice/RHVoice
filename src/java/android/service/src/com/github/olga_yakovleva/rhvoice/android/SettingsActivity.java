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

import java.util.ArrayList;
import java.util.List;

import android.content.Intent;
import android.os.Bundle;
import android.preference.MultiSelectListPreference;
import android.preference.PreferenceActivity;

import com.github.olga_yakovleva.rhvoice.TTSEngine;
import com.github.olga_yakovleva.rhvoice.VoiceInfo;

public final class SettingsActivity extends PreferenceActivity
{
    private static final String TAG="RHVoiceSettingsActivity";

    private List<VoiceInfo> getVoices()
    {
        TTSEngine engine=null;
        try
            {
                final Intent intent=getIntent();
                final String[] paths=intent.getStringArrayExtra("paths");
                if(paths==null)
                    return null;
                engine=new TTSEngine("",getDir("config",0).getAbsolutePath(),paths);
                return engine.getVoices();
            }
        catch(Exception e)
            {
                return null;
            }
        finally
            {
                if(engine!=null)
                    engine.shutdown();
            }
    }

    @Override
    protected void onCreate(Bundle bundle)
    {
        super.onCreate(bundle);
        addPreferencesFromResource(R.xml.settings);
        List<VoiceInfo> voices=getVoices();
        if(voices==null)
            return;
        List<String> voiceNames=new ArrayList<String>();
        for(VoiceInfo voice: voices)
            {
                voiceNames.add(voice.getName());
            }
        MultiSelectListPreference voicePref=(MultiSelectListPreference)findPreference("preferred_voices");
        String[] voiceValues=new String[voiceNames.size()];
        voiceNames.toArray(voiceValues);
        voicePref.setEntries(voiceValues);
        voicePref.setEntryValues(voiceValues);
        voicePref.setEnabled(true);
    }
}

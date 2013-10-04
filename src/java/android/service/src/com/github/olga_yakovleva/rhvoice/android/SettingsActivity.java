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
import java.util.Locale;
import java.util.Map;
import java.util.TreeMap;

import android.content.Intent;
import android.os.Bundle;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;
import com.github.olga_yakovleva.rhvoice.LanguageInfo;
import com.github.olga_yakovleva.rhvoice.TTSEngine;
import com.github.olga_yakovleva.rhvoice.VoiceInfo;

public final class SettingsActivity extends PreferenceActivity
{
    private static final String TAG="RHVoiceSettingsActivity";

    private Preference.OnPreferenceChangeListener onVoiceChange=new Preference.OnPreferenceChangeListener() {
            public boolean onPreferenceChange(Preference pref,Object obj)
            {
                pref.setSummary(obj.toString());
                return true;
            }
        };

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

    private Map<String,List<VoiceInfo>> groupVoicesByLanguage(List<VoiceInfo> voices)
    {
        Map<String,List<VoiceInfo>> result=new TreeMap<String,List<VoiceInfo>>();
        for(VoiceInfo voice:voices)
            {
                LanguageInfo language=voice.getLanguage();
                String code=language.getAlpha2Code();
                List<VoiceInfo> languageVoices=result.get(code);
                if(languageVoices==null)
                    {
                        languageVoices=new ArrayList<VoiceInfo>();
                        result.put(code,languageVoices);
                    }
                languageVoices.add(voice);
            }
        return result;
    }

    private void buildLanguagePreferenceCategory(List<VoiceInfo> voices)
    {
        PreferenceCategory cat=new PreferenceCategory(this);
        getPreferenceScreen().addPreference(cat);
        LanguageInfo language=voices.get(0).getLanguage();
        String firstVoiceName=voices.get(0).getName();
        String code2=language.getAlpha2Code();
        String code3=language.getAlpha3Code();
        cat.setKey("language."+code3);
        Locale locale=new Locale(code2);
        cat.setTitle(locale.getDisplayName());
        ListPreference voicePref=new ListPreference(this);
        voicePref.setOnPreferenceChangeListener(onVoiceChange);
        cat.addPreference(voicePref);
        voicePref.setKey("language."+code3+".voice");
        voicePref.setTitle(R.string.default_voice_title);
        voicePref.setSummary(firstVoiceName);
        voicePref.setDialogTitle(R.string.default_voice_dialog_title);
        int voiceCount=voices.size();
        String[] voiceNames=new String[voiceCount];
        for(int i=0;i<voiceCount;++i)
            {
                voiceNames[i]=voices.get(i).getName();
            }
        voicePref.setEntries(voiceNames);
        voicePref.setEntryValues(voiceNames);
        voicePref.setDefaultValue(firstVoiceName);
    }

    @Override
    protected void onCreate(Bundle bundle)
    {
        super.onCreate(bundle);
        addPreferencesFromResource(R.xml.settings);
        List<VoiceInfo> voices=getVoices();
        if(voices==null)
            return;
        Map<String,List<VoiceInfo>> voiceGroups=groupVoicesByLanguage(voices);
        for(Map.Entry<String,List<VoiceInfo>> entry: voiceGroups.entrySet())
            {
                buildLanguagePreferenceCategory(entry.getValue());
            }
    }
}

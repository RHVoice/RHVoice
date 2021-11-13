/* Copyright (C) 2013, 2014, 2016, 2017, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.os.Bundle;
import androidx.preference.CheckBoxPreference;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceManager;
import androidx.preference.PreferenceScreen;
import android.text.InputFilter;
import android.text.InputType;
import com.github.olga_yakovleva.rhvoice.LanguageInfo;
import com.github.olga_yakovleva.rhvoice.VoiceInfo;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceFragmentCompat;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.TreeMap;

    public final class SettingsFragment extends PreferenceFragmentCompat implements SharedPreferences.OnSharedPreferenceChangeListener
    {
        private static final String TAG="RHVoiceSettingsActivity";
        public static final String NAME="settings";
        public static final String ARG_LANGUAGE_KEY="language_key";

        private Map<String,List<VoiceInfo>> groupVoicesByLanguage(List<VoiceInfo> voices)
        {
            Map<String,List<VoiceInfo>> result=new TreeMap<String,List<VoiceInfo>>();
            for(VoiceInfo voice:voices)
                {
                    LanguageInfo language=voice.getLanguage();
                    String code=language.getAlpha3Code();
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

        private void buildLanguagePreferenceCategory(PreferenceCategory cat0,List<VoiceInfo> voices)
        {
            Context ctx=getPreferenceManager().getContext();
            PreferenceScreen cat=getPreferenceManager().createPreferenceScreen(ctx);
            cat.setPersistent(false);
            LanguageInfo language=voices.get(0).getLanguage();
            String firstVoiceName=voices.get(0).getName();
            String code3=language.getAlpha3Code();
            cat.setKey("language."+code3);
            cat.setTitle(Data.getLanguage(language.getTag3()).getDisplayName());
            cat0.addPreference(cat);
            ListPreference voicePref=new ListPreference(ctx);
            voicePref.setKey("language."+code3+".voice");
            voicePref.setTitle(R.string.default_voice_title);
            voicePref.setSummary("%s");
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
            cat.addPreference(voicePref);
            CheckBoxPreference detectPref=new CheckBoxPreference(ctx);
            detectPref.setKey("language."+code3+".detect");
            detectPref.setTitle(R.string.detect_language_title);
            detectPref.setSummary(R.string.detect_language_desc);
            detectPref.setDefaultValue(true);
            cat.addPreference(detectPref);
            if(language.getPseudoEnglish())
                {
                    CheckBoxPreference engPref=new CheckBoxPreference(ctx);
                    engPref.setKey("language."+code3+".use_pseudo_english");
                    engPref.setTitle(R.string.pseudo_english_title);
                    engPref.setSummary(R.string.pseudo_english_desc);
                    engPref.setDefaultValue(true);
                    cat.addPreference(engPref);
                }
            VolumePreference volumePref=new VolumePreference(ctx);
            volumePref.setKey("language."+code3+".volume");
            cat.addPreference(volumePref);
            RatePreference ratePref=new RatePreference(ctx);
            ratePref.setKey("language."+code3+".rate");
            cat.addPreference(ratePref);
        }

        @Override
        public void onCreatePreferences(Bundle state,String rootKey)
        {
            setPreferencesFromResource(R.xml.settings,null);
            List<VoiceInfo> voices=Data.getVoices(getActivity());
            if(voices.isEmpty())
                return;
            Map<String,List<VoiceInfo>> voiceGroups=groupVoicesByLanguage(voices);
            findPreference("version").setSummary(BuildConfig.VERSION_NAME);
            PreferenceCategory cat=new PreferenceCategory(getPreferenceManager().getContext());
            cat.setOrder(100);
            cat.setKey("languages");
            cat.setTitle(R.string.languages);
            getPreferenceScreen().addPreference(cat);
            for(Map.Entry<String,List<VoiceInfo>> entry: voiceGroups.entrySet())
                {
                    buildLanguagePreferenceCategory(cat,entry.getValue());
                }
            String argLanguageKey=null;
            Bundle args=getArguments();
            if(args!=null)
                argLanguageKey=args.getString(ARG_LANGUAGE_KEY);
            if(argLanguageKey!=null)
                {
                    Preference pref=findPreference(argLanguageKey);
                    if(pref!=null)
                        setPreferenceScreen((PreferenceScreen)pref);
}
        }

        @Override
        public void onSharedPreferenceChanged(SharedPreferences prefs,String key)
        {
            if("wifi_only".equals(key))
                Data.scheduleSync(getActivity(),true);
        }

        @Override
        public void onResume()
        {
            super.onResume();
            PreferenceManager.getDefaultSharedPreferences(getActivity()).registerOnSharedPreferenceChangeListener(this);
        }

        @Override
        public void onPause()
        {
            super.onPause();
            PreferenceManager.getDefaultSharedPreferences(getActivity()).unregisterOnSharedPreferenceChangeListener(this);
        }
    }

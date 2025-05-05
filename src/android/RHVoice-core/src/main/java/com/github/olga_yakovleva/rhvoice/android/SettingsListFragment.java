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
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.view.View;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import androidx.preference.CheckBoxPreference;
import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.PreferenceCategory;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;
import androidx.preference.PreferenceScreen;
import androidx.preference.TwoStatePreference;
import androidx.recyclerview.widget.RecyclerView;

import com.google.common.collect.FluentIterable;

import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public final class SettingsListFragment extends PreferenceFragmentCompat implements SharedPreferences.OnSharedPreferenceChangeListener {
    private static final String TAG = "RHVoiceSettingsActivity";
    public static final String NAME = "settingslist";
    public static final String ARG_LANGUAGE_KEY = "language_key";
    private static final Pattern RE_LANG_KEY = Pattern.compile("^language\\.([a-z]{3})$");
    private final ActivityResultLauncher<String[]> openUserDict = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT ? registerForActivityResult(new ActivityResultContracts.OpenDocument(), this::onUserDictSelected) : null;
    private final ActivityResultLauncher<String[]> openConfigFile = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT ? registerForActivityResult(new ActivityResultContracts.OpenDocument(), this::onConfigFileSelected) : null;


    private void buildLanguagePreferenceCategory(PreferenceCategory cat0, LanguagePack lp, List<VoicePack> voices) {
        Context ctx = getPreferenceManager().getContext();
        PreferenceScreen cat = getPreferenceManager().createPreferenceScreen(ctx);
        cat.setPersistent(false);
        String firstVoiceName = voices.get(0).getName();
        String code3 = lp.getCode();
        cat.setKey("language." + code3);
        cat.setTitle(lp.getDisplayName());
        cat0.addPreference(cat);
        ListPreference voicePref = new ListPreference(ctx);
        voicePref.setKey("language." + code3 + ".voice");
        voicePref.setTitle(R.string.default_voice_title);
        voicePref.setSummary("%s");
        voicePref.setDialogTitle(R.string.default_voice_dialog_title);
        int voiceCount = voices.size();
        String[] voiceNames = new String[voiceCount];
        for (int i = 0; i < voiceCount; ++i) {
            voiceNames[i] = voices.get(i).getName();
        }
        voicePref.setEntries(voiceNames);
        voicePref.setEntryValues(voiceNames);
        voicePref.setDefaultValue(firstVoiceName);
        cat.addPreference(voicePref);
        CheckBoxPreference detectPref = new CheckBoxPreference(ctx);
        detectPref.setKey("language." + code3 + ".detect");
        detectPref.setTitle(R.string.detect_language_title);
        detectPref.setSummary(R.string.detect_language_desc);
        detectPref.setDefaultValue(true);
        cat.addPreference(detectPref);
        if (lp.getRes().pseudoEnglish) {
            CheckBoxPreference engPref = new CheckBoxPreference(ctx);
            engPref.setKey("language." + code3 + ".use_pseudo_english");
            engPref.setTitle(R.string.pseudo_english_title);
            engPref.setSummary(R.string.pseudo_english_desc);
            engPref.setDefaultValue(true);
            cat.addPreference(engPref);
        }
        VolumePreference volumePref = new VolumePreference(ctx);
        volumePref.setKey("language." + code3 + ".volume");
        cat.addPreference(volumePref);
        RatePreference ratePref = new RatePreference(ctx);
        ratePref.setKey("language." + code3 + ".rate");
        cat.addPreference(ratePref);
        if (openUserDict == null)
            return;
        PreferenceCategory dCat = new PreferenceCategory(ctx);
        dCat.setKey("language." + code3 + ".user_dicts");
        dCat.setTitle(R.string.user_dicts);
        cat.addPreference(dCat);
        Preference addDictPref = new Preference(ctx);
        addDictPref.setKey("language." + code3 + ".add_user_dict");
        addDictPref.setTitle(R.string.add);
        addDictPref.setOnPreferenceClickListener(p -> {
            openUserDict.launch(new String[]{"text/plain"});
            return true;
        });
        dCat.addPreference(addDictPref);
        Preference rmDictPref = new Preference(ctx);
        rmDictPref.setKey("language." + code3 + ".rm_user_dict");
        rmDictPref.setTitle(R.string.remove);
        rmDictPref.setOnPreferenceClickListener(p -> {
            RemoveUserDictDialogFragment.show(getActivity(), getLanguageName());
            return true;
        });
        dCat.addPreference(rmDictPref);
    }

    @Override
    public void onCreatePreferences(Bundle state, String rootKey) {
        setPreferencesFromResource(R.xml.settings, null);
        findPreference("version").setSummary(BuildConfig.VERSION_NAME);
        if (openConfigFile != null) {
            final TwoStatePreference configFilePref = findPreference("config_file");
            configFilePref.setOnPreferenceChangeListener(this::onConfigFilePrefChange);
            configFilePref.setChecked(Config.getConfigFile(requireActivity()).exists());
            configFilePref.setVisible(true);
        }
        PreferenceCategory cat = null;
        final DataManager dm = Repository.get().createDataManager();
        for (LanguagePack lp : dm.iterLanguages()) {
            final List<VoicePack> voices = lp.iterVoices().filter(v -> v.getEnabled(requireContext()) && v.isInstalled(requireContext())).toList();
            if (voices.isEmpty())
                continue;
            if (cat == null) {
                cat = new PreferenceCategory(getPreferenceManager().getContext());
                cat.setOrder(100);
                cat.setKey("languages");
                cat.setTitle(R.string.languages);
                getPreferenceScreen().addPreference(cat);
            }
            buildLanguagePreferenceCategory(cat, lp, voices);
        }
        String argLanguageKey = null;
        Bundle args = getArguments();
        if (args != null)
            argLanguageKey = args.getString(ARG_LANGUAGE_KEY);
        if (argLanguageKey != null) {
            Preference pref = findPreference(argLanguageKey);
            if (pref != null)
                setPreferenceScreen((PreferenceScreen) pref);
        }
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        RecyclerView recyclerView = view.findViewById(androidx.preference.R.id.recycler_view);
        InsetUtil.setInsets(recyclerView, (left, top, right, bottom) -> recyclerView.setPadding(left, 0, right, bottom));
    }

    @Override
    public void onSharedPreferenceChanged(SharedPreferences prefs, String key) {
        if ("wifi_only".equals(key))
            Repository.get().createDataManager().scheduleSync(requireActivity(), true);
    }

    @Override
    public void onResume() {
        super.onResume();
        PreferenceManager.getDefaultSharedPreferences(getActivity()).registerOnSharedPreferenceChangeListener(this);
    }

    @Override
    public void onPause() {
        super.onPause();
        PreferenceManager.getDefaultSharedPreferences(getActivity()).unregisterOnSharedPreferenceChangeListener(this);
    }

    private void onUserDictSelected(Uri uri) {
        if (uri == null)
            return;
        final String lang = getLanguageName();
        if (lang == null)
            return;
        final Intent intent = new Intent(requireActivity(), ImportConfigService.class);
        intent.setAction(ImportConfigService.ACTION_IMPORT_USER_DICT);
        intent.setData(uri);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        intent.putExtra(ImportConfigService.EXTRA_LANGUAGE, lang);
        requireActivity().startService(intent);
    }

    private void onConfigFileSelected(Uri uri) {
        if (uri == null)
            return;
        final Intent intent = new Intent(requireActivity(), ImportConfigService.class);
        intent.setAction(ImportConfigService.ACTION_IMPORT_CONFIG_FILE);
        intent.setData(uri);
        intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
        requireActivity().startService(intent);
        final TwoStatePreference configFilePref = findPreference("config_file");
        configFilePref.setChecked(true);
    }

    private String getLanguageCode() {
        final Bundle args = getArguments();
        if (args == null)
            return null;
        final String key = args.getString(ARG_LANGUAGE_KEY);
        if (key == null)
            return null;
        final Matcher m = RE_LANG_KEY.matcher(key);
        if (!m.find())
            return null;
        return m.group(1);
    }

    private String getLanguageName() {
        final DataManager dm = Repository.get().createDataManager();
        final String code = getLanguageCode();
        if (code == null)
            return null;
        return FluentIterable.from(dm.getLanguages()).firstMatch(l -> l.getCode().equals(code)).transform(l -> l.getName()).orNull();
    }

    private boolean onConfigFilePrefChange(Preference pref, Object val) {
        if ((Boolean) val) {
            openConfigFile.launch(new String[]{"*/*"});
            return false;
        } else {
            Config.getConfigFile(requireActivity()).delete();
            LocalBroadcastManager.getInstance(requireActivity()).sendBroadcast(new Intent(RHVoiceService.ACTION_CONFIG_CHANGE));
            return true;
        }
    }
}

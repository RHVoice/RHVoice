/* Copyright (C) 2021  Olga Yakovleva <olga@rhvoice.org> */

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
import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.MainThread;

import com.github.olga_yakovleva.rhvoice.RHVoiceException;
import com.github.olga_yakovleva.rhvoice.TTSEngine;
import com.github.olga_yakovleva.rhvoice.VoiceInfo;
import com.google.common.collect.FluentIterable;
import com.google.common.collect.ImmutableList;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.google.common.base.Optional;
import com.google.common.collect.Ordering;

public final class DataManager {
    private static final String TAG = "RHVoice.DataManager";
    public static final String WORK_PREFIX_STRING = "com.github.olga_yakovleva.rhvoice.android";
    public static final String WORK_TAG = WORK_PREFIX_STRING + ".data";
    private PackageDirectory dir;

    public DataManager() {
    }

    public DataManager(PackageDirectory dir) {
        this.dir = dir;
    }

    public void setPackageDirectory(PackageDirectory dir) {
        this.dir = dir;
    }

    public LanguagePack getLanguageById(String id) {
        if (dir == null)
            return null;
        LanguageResource res = dir.languageIndexById.get(id);
        if (res == null)
            return null;
        return new LanguagePack(res);
    }

    public LanguagePack getLanguageByName(String name) {
        if (dir == null)
            return null;
        LanguageResource res = dir.languageIndexByName.get(name);
        if (res == null)
            return null;
        return new LanguagePack(res);
    }

    public LanguagePack getLanguageByCode(String code) {
        if (dir == null)
            return null;
        LanguageResource res = dir.languageIndexByCode.get(code);
        if (res == null)
            return null;
        return new LanguagePack(res);
    }

    public List<LanguagePack> getLanguages() {
        return iterLanguages().toList();
    }

    FluentIterable<LanguagePack> iterLanguages() {
        if (dir == null)
            return FluentIterable.of();
        return FluentIterable.from(dir.languages).transform(r -> new LanguagePack(r));
    }

    public List<VoiceAccent> getAccents() {
        return iterLanguages().transformAndConcat(lp -> lp.getAccents()).toList();
    }

    public List<String> getPaths(Context context) {
        List<String> paths = new ArrayList<String>();
        for (LanguagePack language : getLanguages()) {
            paths.addAll(language.getPaths(context));
        }
        return paths;
    }

    public List<VoiceInfo> getVoices(Context context) {
        TTSEngine engine = null;
        try {
            engine = new TTSEngine("", Config.getDir(context).getAbsolutePath(), getPaths(context), PackageClient.getPath(context), CoreLogger.instance);
            return engine.getVoices();
        } catch (RHVoiceException e) {
            if (BuildConfig.DEBUG)
                Log.e(TAG, "Engine initialization failed", e);
            return Collections.<VoiceInfo>emptyList();
        } finally {
            if (engine != null)
                engine.shutdown();
        }
    }


    @MainThread
    public void scheduleSync(Context context, boolean replace) {
        for (LanguagePack lang : getLanguages()) {
            lang.scheduleSync(context, replace);
            for (VoicePack voice : lang.getVoices())
                voice.scheduleSync(context, replace);
        }
    }

    public Optional<VoicePack> getDefaultVoice(LanguagePack lang, String c3) {
        final String l2 = lang.getOldCode();
        Optional<VoicePack> voice = Optional.absent();
        final Map<String, String> langPrefs = dir.defaultVoices.get(l2);
        if (langPrefs != null && !langPrefs.isEmpty()) {
            String key = "*";
            if (!TextUtils.isEmpty(c3)) {
                key = c3.toUpperCase();
                if (!langPrefs.containsKey(key))
                    key = "*";
            }
            final FluentIterable<String> ids = (langPrefs.containsKey(key) ? FluentIterable.of(langPrefs.get(key)) : FluentIterable.<String>of()).append(langPrefs.values());
            voice = ids.firstMatch(id -> lang.hasVoiceWithId(id)).transform(id -> lang.findVoiceById(id));
        }
        return voice;
    }

    public void autoInstallVoice(Context context, String l3, String c3) {
        if (TextUtils.isEmpty(l3))
            return;
        if (BuildConfig.DEBUG)
            Log.v(TAG, "autoInstallVoice, " + l3 + ", " + c3);
        final LanguagePack lang = getLanguageByCode(l3);
        if (lang == null)
            return;
        if (!lang.hasVoices())
            return;
        if (lang.hasEnabledVoice(context))
            return;
        Optional<VoicePack> voice = getDefaultVoice(lang, c3);
        voice = voice.or(lang.iterVoices().first());
        if (BuildConfig.DEBUG)
            Log.v(TAG, "Chosen voice " + voice.get().getId());
        voice.get().setEnabled(context, true);
    }

    public boolean hasDir() {
        return (dir != null);
    }
}

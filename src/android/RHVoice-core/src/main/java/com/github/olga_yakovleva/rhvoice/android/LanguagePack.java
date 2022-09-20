/* Copyright (C) 2017, 2018, 2019, 2020, 2021  Olga Yakovleva <olga@rhvoice.org> */

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

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.HashMap;

import android.text.TextUtils;

import com.google.common.collect.FluentIterable;
import com.google.common.collect.ImmutableList;

import java.util.Set;

public final class LanguagePack extends DataPack {
    private final LanguageResource res;

    public LanguagePack(LanguageResource res) {
        this.res = res;
    }

    public LanguageResource getRes() {
        return res;
    }

    public String getType() {
        return "language";
    }

    public String getDisplayName() {
        Locale loc = new Locale(res.lang2code);
        return loc.getDisplayName();
    }

    protected String getBaseFileName() {
        return String.format("RHVoice-language-%s", getName());
    }

    FluentIterable<VoicePack> iterVoices() {
        return FluentIterable.from(res.voices).transform(res -> new VoicePack(this, res));
    }

    public List<VoicePack> getVoices() {
        return iterVoices().toList();
    }

    @Override
    public boolean getEnabled(Context context) {
        if (res.lang3code.equals("eng"))
            return true;
        for (VoicePack voice : getVoices()) {
            if (voice.getEnabled(context))
                return true;
        }
        return false;
    }

    public List<String> getPaths(Context context) {
        List<String> paths = new ArrayList<String>();
        String languagePath = getPath(context);
        if (languagePath == null)
            return paths;
        for (VoicePack voice : getVoices()) {
            if (!voice.getEnabled(context))
                continue;
            String voicePath = voice.getPath(context);
            if (voicePath != null)
                paths.add(voicePath);
        }
        if (paths.isEmpty() && !res.lang3code.equals("eng"))
            return paths;
        paths.add(languagePath);
        return paths;
    }

    public String getCode() {
        return res.lang3code;
    }

    @Override
    protected void notifyDownloadStart(IDataSyncCallback callback) {
        callback.onLanguageDownloadStart(this);
    }

    @Override
    protected void notifyDownloadDone(IDataSyncCallback callback) {
        callback.onLanguageDownloadDone(this);
    }

    @Override
    protected void notifyInstallation(IDataSyncCallback callback) {
        callback.onLanguageInstallation(this);
    }

    @Override
    protected void notifyRemoval(IDataSyncCallback callback) {
        callback.onLanguageRemoval(this);
    }

    public VoicePack findVoice(String name) {
        VoiceResource vr = res.voiceIndexByName.get(name);
        return vr == null ? null : new VoicePack(this, vr);
    }

    public VoicePack findVoiceById(String id) {
        VoiceResource vr = res.voiceIndexById.get(id);
        return vr == null ? null : new VoicePack(this, vr);
    }

    public String getTestMessage() {
        return res.testMessage;
    }

    public String getOldCode() {
        return res.lang2code;
    }

    public Set<AccentTag> getAccentTags() {
        return iterVoices().transform(v -> v.getAccentTag()).toSet();
    }

    public List<VoiceAccent> getAccents() {
        return FluentIterable.from(getAccentTags()).transform(t -> new VoiceAccent(this, t)).toList();
    }

    public VoiceAccent getAccent(String country, String variant) {
        return FluentIterable.from(getAccentTags()).firstMatch(t -> t.country3.equals(country) && t.variant.equals(variant)).transform(t -> new VoiceAccent(this, t)).orNull();
    }

    public boolean hasEnabledVoice(Context context) {
        for (VoicePack voice : iterVoices()) {
            if (voice.getEnabled(context))
                return true;
        }
        return false;
    }

    public boolean hasVoiceWithId(String id) {
        return res.voiceIndexById.containsKey(id);
    }

    public boolean hasVoiceWithName(String name) {
        return res.voiceIndexByName.containsKey(name);
    }

    public boolean hasVoices() {
        return !res.voices.isEmpty();
    }
}

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

/* Copyright (C) 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import java.util.List;
import java.util.Locale;

import com.google.common.collect.FluentIterable;

public final class VoiceAccent {
    private final LanguagePack lang;
    private final AccentTag tag;

    public VoiceAccent(LanguagePack lang, AccentTag tag) {
        this.lang = lang;
        this.tag = tag;
    }

    public String getDisplayName() {
        return tag.getDisplayName();
    }

    @Override
    public String toString() {
        return getDisplayName();
    }

    public FluentIterable<VoicePack> iterVoices() {
        return lang.iterVoices().filter(v -> tag.equals(v.getAccentTag()));
    }

    public List<VoicePack> getVoices() {
        return iterVoices().toList();
    }

    public LanguagePack getLanguage() {
        return lang;
    }

    public AccentTag getTag() {
        return tag;
    }

    public Locale createLocale() {
        return tag.createLocale();
    }

    public boolean isInstalled(Context context) {
        return iterVoices().anyMatch(v -> v.isInstalled(context));
    }
}

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

import android.os.SystemClock;

import com.google.common.collect.Maps;
import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableMap;

import java.time.Instant;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

public final class PackageDirectory {
    public long ttl = 86400;
    public long localTtl = 0;
    public List<LanguageResource> languages = ImmutableList.of();
    public Map<String, Map<String, String>> defaultVoices = ImmutableMap.of();
    public transient Map<String, LanguageResource> languageIndexByName;
    public transient Map<String, LanguageResource> languageIndexById;
    public transient Map<String, LanguageResource> languageIndexByCode;
    public transient Instant nextUpdateTime;

    public PackageDirectory() {
    }

    public void index() {
        languageIndexByName = Maps.uniqueIndex(languages, r -> r.name);
        languageIndexById = Maps.uniqueIndex(languages, LanguageResource::getId);
        languageIndexByCode = Maps.uniqueIndex(languages, r -> r.lang3code);
        for (LanguageResource lr : languages)
            lr.index();
    }
}

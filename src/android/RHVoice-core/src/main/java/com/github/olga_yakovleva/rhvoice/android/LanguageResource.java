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

import java.util.List;
import java.util.ArrayList;
import java.util.Map;

import com.google.common.collect.Maps;
import com.google.common.collect.ImmutableList;

public class LanguageResource extends TtsResource {
    public String lang2code = "";
    public String lang3code = "";
    public String testMessage = "";
    public List<VoiceResource> voices = ImmutableList.of();
    public boolean pseudoEnglish;
    public transient Map<String, VoiceResource> voiceIndexById;
    public transient Map<String, VoiceResource> voiceIndexByName;

    public LanguageResource() {
    }

    public final void index() {
        voiceIndexById = Maps.uniqueIndex(voices, VoiceResource::getId);
        voiceIndexByName = Maps.uniqueIndex(voices, r -> r.name);
    }
}

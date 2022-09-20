/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

package com.github.olga_yakovleva.rhvoice;

public final class VoiceInfo {
    private String name = null;
    private String id;
    private LanguageInfo language = null;

    void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    void setId(String id) {
        this.id = id;
    }

    public String getId() {
        return id;
    }


    void setLanguage(LanguageInfo lang) {
        language = lang;
    }

    public LanguageInfo getLanguage() {
        return language;
    }
}

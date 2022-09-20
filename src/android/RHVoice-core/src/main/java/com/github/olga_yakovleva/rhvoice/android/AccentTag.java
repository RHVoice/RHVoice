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

import com.google.common.base.Objects;

import java.util.Locale;

public final class AccentTag {
    public final String language3;
    public final String language2;
    public final String country3;
    public final String country2;
    public final String variant;

    public AccentTag(String l3, String l2, String c3, String c2, String v) {
        language3 = l3;
        country3 = c3;
        language2 = l2;
        country2 = c2;
        variant = v;
    }

    @Override
    public boolean equals(Object other) {
        if (this == other)
            return true;
        if (!(other instanceof AccentTag))
            return false;
        final AccentTag otherTag = (AccentTag) other;
        return Objects.equal(language3, otherTag.language3) && Objects.equal(country3, otherTag.country3) && Objects.equal(variant, otherTag.variant);
    }

    @Override
    public int hashCode() {
        return Objects.hashCode(language3, country3, variant);
    }

    public Locale createLocale() {
        if (country2.isEmpty())
            return new Locale(language2);
        if (variant.isEmpty())
            return new Locale(language2, country2);
        return new Locale(language2, country2, variant);
    }

    public String getDisplayName() {
        return createLocale().getDisplayName();
    }

    public String toString3() {
        if (country3.isEmpty())
            return language3;
        if (variant.isEmpty())
            return (language3 + "-" + country3);
        return (language3 + "-" + country3 + "-" + variant);
    }
}

/* Copyright (C) 2017  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import java.util.Comparator;
import java.text.Collator;

final class DataPackNameComparator<T extends DataPack> implements Comparator<T> {
    private final Collator col;

    public DataPackNameComparator() {
        col = Collator.getInstance();
        col.setStrength(Collator.PRIMARY);
    }

    public int compare(T p1, T p2) {
        return col.compare(p1.toString(), p2.toString());
    }
}

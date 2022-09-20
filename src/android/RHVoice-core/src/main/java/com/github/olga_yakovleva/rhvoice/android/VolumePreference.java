/* Copyright (C) 2020  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

public final class VolumePreference extends ProsodyPreference {
    public VolumePreference(Context context) {
        super(context);
        setTitle(R.string.speech_volume);
        setDialogTitle(R.string.speech_volume);
    }

    @Override
    public int getMaxProgress() {
        return 96;
    }

    @Override
    protected int convertFromProgress(int progress) {
        double db = (progress - 48) / 4.0;
        double m = Math.pow(10, db / 20);
        return (int) Math.round(m * 100);
    }

    @Override
    protected int convertToProgress(int value) {
        if (value > 400)
            value = 400;
        else if (value < 25)
            value = 25;
        double m = value / 100.0;
        double db = 20.0 * Math.log10(m);
        return (int) Math.round((db + 12) * 4);
    }

    @Override
    public int getProgressStep() {
        return 4;
    }
}

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

public final class RatePreference extends ProsodyPreference {
    public RatePreference(Context context) {
        super(context);
        setTitle(R.string.speech_rate);
        setDialogTitle(R.string.speech_rate);
    }

    @Override
    public int getMaxProgress() {
        return 250;
    }

    @Override
    protected int convertFromProgress(int progress) {
        return (progress + 50);
    }

    @Override
    protected int convertToProgress(int value) {
        if (value > 300)
            value = 300;
        else if (value < 50)
            value = 50;
        return (value - 50);
    }

    @Override
    public int getProgressStep() {
        return 5;
    }
}

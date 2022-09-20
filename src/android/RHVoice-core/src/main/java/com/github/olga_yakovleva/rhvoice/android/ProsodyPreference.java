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
import android.content.res.TypedArray;

import androidx.preference.DialogPreference;

import android.util.Log;

public abstract class ProsodyPreference extends DialogPreference {
    private static final String TAG = "RHVoice.ProsodyPreference";
    private String prefValue = "100";

    public ProsodyPreference(Context context) {
        super(context);
        setDefaultValue("100");
        setPersistent(true);
        setDialogLayoutResource(R.layout.prosody_dialog);
    }

    public abstract int getMaxProgress();

    protected abstract int convertFromProgress(int progress);

    protected abstract int convertToProgress(int value);

    public void setProgress(int progress) {
        if (progress > getMaxProgress())
            progress = getMaxProgress();
        else if (progress < 0)
            progress = 0;
        int value = convertFromProgress(progress);
        prefValue = String.valueOf(value);
        if (BuildConfig.DEBUG)
            Log.v(TAG, "Persisting value " + prefValue);
        persistString(prefValue);
        notifyChanged();
    }

    public int getProgress() {
        int value = Integer.parseInt(prefValue);
        int progress = convertToProgress(value);
        if (progress > getMaxProgress())
            progress = getMaxProgress();
        else if (progress < 0)
            progress = 0;
        return progress;
    }

    @Override
    protected Object onGetDefaultValue(TypedArray a, int index) {
        return a.getString(index);
    }

    @Override
    protected void onSetInitialValue(Object defaultValue) {
        prefValue = getPersistedString((String) defaultValue);
    }

    public int getDefaultProgress() {
        return convertToProgress(100);
    }

    public abstract int getProgressStep();
}

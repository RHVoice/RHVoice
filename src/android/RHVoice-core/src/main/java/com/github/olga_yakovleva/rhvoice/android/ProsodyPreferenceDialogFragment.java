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

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;

import androidx.appcompat.app.AlertDialog;
import androidx.preference.PreferenceDialogFragmentCompat;

public final class ProsodyPreferenceDialogFragment extends PreferenceDialogFragmentCompat implements SeekBar.OnSeekBarChangeListener, View.OnClickListener {
    public static final String TAG = "ProsodyPreferenceDialogFragment";
    private static final String SAVE_STATE_PROGRESS = "ProsodyPreferenceDialogFragment.progress";
    private static final String SAVE_STATE_MAX_PROGRESS = "ProsodyPreferenceDialogFragment.maxProgress";

    private int progress;
    private int maxProgress;

    private SeekBar slider;
    private Button defBtn;
    private Button incBtn;
    private Button decBtn;

    public static ProsodyPreferenceDialogFragment newInstance(String key) {
        ProsodyPreferenceDialogFragment f = new ProsodyPreferenceDialogFragment();
        Bundle b = new Bundle(1);
        b.putString(ARG_KEY, key);
        f.setArguments(b);
        return f;
    }

    private ProsodyPreference getProsodyPreference() {
        return (ProsodyPreference) getPreference();
    }

    @Override
    public void onCreate(Bundle state) {
        super.onCreate(state);
        if (state == null) {
            ProsodyPreference pref = getProsodyPreference();
            maxProgress = pref.getMaxProgress();
            progress = pref.getProgress();
        } else {
            maxProgress = state.getInt(SAVE_STATE_MAX_PROGRESS, 100);
            progress = state.getInt(SAVE_STATE_PROGRESS, 50);
        }
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putInt(SAVE_STATE_MAX_PROGRESS, maxProgress);
        outState.putInt(SAVE_STATE_PROGRESS, progress);
    }

    @Override
    protected void onBindDialogView(View view) {
        super.onBindDialogView(view);
        slider = view.findViewById(R.id.slider);
        slider.setMax(maxProgress);
        slider.setProgress(progress);
        slider.setOnSeekBarChangeListener(this);
        defBtn = view.findViewById(R.id.def_btn);
        defBtn.setOnClickListener(this);
        incBtn = view.findViewById(R.id.inc_btn);
        incBtn.setOnClickListener(this);
        decBtn = view.findViewById(R.id.dec_btn);
        decBtn.setOnClickListener(this);
        syncButtonStates(progress);
    }

    @Override
    protected void onPrepareDialogBuilder(AlertDialog.Builder builder) {
        super.onPrepareDialogBuilder(builder);
        builder.setPositiveButton(null, null);
        builder.setNegativeButton(null, null);
    }

    @Override
    public void onDialogClosed(boolean positiveResult) {
    }

    @Override
    public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
        this.progress = progress;
        getProsodyPreference().setProgress(progress);
        syncButtonStates(progress);
    }

    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {
    }

    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
    }

    @Override
    public void onClick(View view) {
        if (view.getId() == R.id.def_btn) {
            slider.setProgress(getProsodyPreference().getDefaultProgress());
            return;
        }
        int step = getProsodyPreference().getProgressStep();
        if (view.getId() == R.id.dec_btn)
            step = -step;
        else if (view.getId() != R.id.inc_btn)
            return;
        int newProgress = slider.getProgress() + step;
        if (newProgress < 0)
            newProgress = 0;
        else if (newProgress > getProsodyPreference().getMaxProgress())
            newProgress = getProsodyPreference().getMaxProgress();
        slider.setProgress(newProgress);
    }

    private void syncButtonStates(int progress) {
        if (progress >= maxProgress)
            incBtn.setEnabled(false);
        else if (!incBtn.isEnabled())
            incBtn.setEnabled(true);
        if (progress <= 0)
            decBtn.setEnabled(false);
        else if (!decBtn.isEnabled())
            decBtn.setEnabled(true);
    }
}

package com.github.olga_yakovleva.rhvoice.android;

import android.content.Context;
import android.widget.SeekBar;
import android.support.v7.preference.PreferenceViewHolder;

public final class SeekBarPreference extends android.support.v7.preference.SeekBarPreference
{
    public SeekBarPreference(Context ctx)
    {
        super(ctx);
}

    @Override
    public void onBindViewHolder(PreferenceViewHolder view)
    {
        super.onBindViewHolder(view);
        SeekBar seekBar=(SeekBar)view.findViewById(android.support.v7.preference.R.id.seekbar);
        seekBar.setFocusable(true);
        seekBar.setContentDescription(getTitle());
}
}

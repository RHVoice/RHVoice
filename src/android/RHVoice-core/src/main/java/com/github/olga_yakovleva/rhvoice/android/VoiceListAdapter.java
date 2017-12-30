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
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

package com.github.olga_yakovleva.rhvoice.android;

import android.app.Activity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ProgressBar;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public final class VoiceListAdapter extends BaseAdapter
{
    private class VoiceSelectedListener implements CompoundButton.OnCheckedChangeListener
    {
        private final VoicePack voice;

        public VoiceSelectedListener(VoicePack voice)
        {
            this.voice=voice;
}

        public void onCheckedChanged(CompoundButton v,boolean newState)
        {
            ((AvailableVoicesFragment.Listener)activity).onVoiceSelected(voice,newState);
}
}

    private final Activity activity;
    private final List<VoicePack> voices;
    private final LayoutInflater inflater;

    public VoiceListAdapter(Activity activity,LanguagePack lang)
    {
        this.activity=activity;
        inflater=(LayoutInflater)activity.getSystemService(Activity.LAYOUT_INFLATER_SERVICE);
        voices=new ArrayList<VoicePack>(lang.getVoices());
        Collections.<VoicePack>sort(voices,new DataPackNameComparator<VoicePack>());
}

    @Override
    public boolean hasStableIds()
    {
        return true;
}

    @Override
    public int getCount()
    {
        return voices.size();
}

    @Override
    public VoicePack getItem(int pos)
    {
        return voices.get(pos);
}

    @Override
    public long getItemId(int pos)
    {
        return pos;
}

    @Override
    public View getView(int pos,View convertView,ViewGroup parent)
    {
        View v=convertView;
        if(v==null)
            v=inflater.inflate(R.layout.voice_list_item,parent,false);
        VoicePack voice=voices.get(pos);
        boolean enabled=voice.getEnabled(activity);
        CheckBox cv=(CheckBox)v.findViewById(R.id.voice);
        cv.setOnCheckedChangeListener(null);
        cv.setText(voice.getName());
        cv.setChecked(enabled);
        cv.setOnCheckedChangeListener(this.new VoiceSelectedListener(voice));
        ProgressBar pv=(ProgressBar)v.findViewById(R.id.progress);
        if(enabled&&!voice.isUpToDate(activity))
            pv.setVisibility(View.VISIBLE);
        else
            pv.setVisibility(View.GONE);
        return v;
}
}

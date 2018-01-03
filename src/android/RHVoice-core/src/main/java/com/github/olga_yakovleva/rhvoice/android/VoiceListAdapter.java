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
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.TextView;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public final class VoiceListAdapter extends BaseAdapter
{
    private class ActionButtonListener implements View.OnClickListener
    {
        private final VoicePack voice;
        private final boolean flag;

        public ActionButtonListener(VoicePack voice,boolean flag)
        {
            this.voice=voice;
            this.flag=flag;
}

        public void onClick(View v)
        {
            ((AvailableVoicesFragment.Listener)activity).onVoiceSelected(voice,flag);
            if(flag)
                notifyDataSetChanged();
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
        boolean installed=voice.isInstalled(activity);
        boolean upToDate=voice.isUpToDate(activity);
        TextView tv=(TextView)v.findViewById(R.id.voice);
        tv.setText(voice.getName());
        ProgressBar pv=(ProgressBar)v.findViewById(R.id.progress);
        if(enabled&&!upToDate)
            pv.setVisibility(View.VISIBLE);
        else
            pv.setVisibility(View.GONE);
        ImageButton bv=(ImageButton)(v.findViewById(R.id.action));
        if(enabled)
            {
                if(installed)
                    {
                        bv.setImageResource(R.drawable.ic_delete);
                        bv.setContentDescription(activity.getString(R.string.uninstall));
}
                else
                    {
                        bv.setImageResource(R.drawable.ic_cancel);
                        bv.setContentDescription(activity.getString(android.R.string.cancel));
}
                bv.setOnClickListener(this.new ActionButtonListener(voice,false));
}
        else
            {
                bv.setImageResource(R.drawable.ic_download);
                bv.setContentDescription(activity.getString(R.string.install));
                bv.setOnClickListener(this.new ActionButtonListener(voice,true));
            }
        return v;
}
}

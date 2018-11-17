/* Copyright (C) 2017, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import android.support.v4.app.FragmentActivity;
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

    private class PlayButtonListener implements View.OnClickListener
    {
        private final VoicePack voice;

        public PlayButtonListener(VoicePack voice)
        {
            this.voice=voice;
}

        public void onClick(View v)
        {
            PlayerFragment f=findPlayerFragment();
            if(f==null)
                return;
            if(f.isPlaying(voice))
                f.stopPlayback();
            else
                f.play(voice);
}
}

    private final FragmentActivity activity;
    private final List<VoicePack> voices;
    private final LayoutInflater inflater;

    private PlayerFragment findPlayerFragment()
    {
        return (PlayerFragment)(activity.getSupportFragmentManager().findFragmentByTag("player"));
}

    public VoiceListAdapter(FragmentActivity activity,LanguagePack lang)
    {
        this.activity=activity;
        inflater=(LayoutInflater)activity.getSystemService(FragmentActivity.LAYOUT_INFLATER_SERVICE);
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
        TextView textView=(TextView)v.findViewById(R.id.voice);
        textView.setText(voice.getName());
        ProgressBar progressBar=(ProgressBar)v.findViewById(R.id.progress);
        if(enabled&&!upToDate)
            progressBar.setVisibility(View.VISIBLE);
        else
            progressBar.setVisibility(View.GONE);
        ImageButton actionButton=(ImageButton)(v.findViewById(R.id.action));
        if(enabled)
            {
                if(installed)
                    {
                        actionButton.setImageResource(R.drawable.ic_delete);
                        actionButton.setContentDescription(activity.getString(R.string.uninstall));
}
                else
                    {
                        actionButton.setImageResource(R.drawable.ic_cancel);
                        actionButton.setContentDescription(activity.getString(android.R.string.cancel));
}
                actionButton.setOnClickListener(this.new ActionButtonListener(voice,false));
}
        else
            {
                actionButton.setImageResource(R.drawable.ic_download);
                actionButton.setContentDescription(activity.getString(R.string.install));
                actionButton.setOnClickListener(this.new ActionButtonListener(voice,true));
            }
        PlayerFragment pf=findPlayerFragment();
        ImageButton playButton=(ImageButton)(v.findViewById(R.id.play));
        if(pf!=null&&pf.canPlay(voice))
            {
                if(pf.isPlaying(voice))
                    {
                        playButton.setImageResource(R.drawable.ic_stop);
                        playButton.setContentDescription(activity.getString(R.string.stop));
}
                else
                    {
                        playButton.setImageResource(R.drawable.ic_play);
                        playButton.setContentDescription(activity.getString(R.string.play));
}
                playButton.setVisibility(View.VISIBLE);
                playButton.setOnClickListener(this.new PlayButtonListener(voice));
}
        else
            {
                playButton.setOnClickListener(null);
                playButton.setVisibility(View.GONE);
            }
        return v;
}
}

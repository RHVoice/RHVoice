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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.support.v4.app.ListFragment;
import android.support.v4.content.LocalBroadcastManager;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import java.util.ArrayList;

public final class AvailableVoicesFragment extends ListFragment
{
    public interface Listener
    {
        public void onVoiceSelected(VoicePack voice,boolean state);
}

    public static final String ARG_LANGUAGE="language";
    private LanguagePack language;
    private VoiceListAdapter adapter;

    private final BroadcastReceiver voiceInstalledReceiver=new BroadcastReceiver()
        {
            @Override
            public void onReceive(Context context,Intent intent)
            {
                String name=intent.getStringExtra("name");
                if(language.findVoice(name)!=null)
                    adapter.notifyDataSetChanged();
}
        };

    @Override
    public void onActivityCreated(Bundle state)
    {
        super.onActivityCreated(state);
        language=Data.getLanguage(getArguments().getString(ARG_LANGUAGE));
        adapter=new VoiceListAdapter(getActivity(),language);
        ListView listView=getListView();
        listView.setChoiceMode(ListView.CHOICE_MODE_NONE);
        setListAdapter(adapter);
}

    @Override
    public void onStart()
    {
        super.onStart();
        refresh();
        IntentFilter filter=new IntentFilter(DataSyncJob.ACTION_VOICE_INSTALLED);
        filter.addAction(DataSyncJob.ACTION_VOICE_REMOVED);
        LocalBroadcastManager.getInstance(getActivity()).registerReceiver(voiceInstalledReceiver,filter);
        ActionBar actionBar=((AppCompatActivity)getActivity()).getSupportActionBar();
        if(actionBar!=null)
            actionBar.setSubtitle(language.getDisplayName());
}

    @Override
    public void onStop()
    {
        super.onStop();
        LocalBroadcastManager.getInstance(getActivity()).unregisterReceiver(voiceInstalledReceiver);
}

    public void refresh()
    {
        adapter.notifyDataSetChanged();
}
}

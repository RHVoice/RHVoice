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

import android.app.ActionBar;
import android.app.ListFragment;
import android.os.Bundle;
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

    @Override
    public void onActivityCreated(Bundle state)
    {
        super.onActivityCreated(state);
        language=Data.getLanguage(getArguments().getString(ARG_LANGUAGE));
        ArrayAdapter<VoicePack> voices=new ArrayAdapter<VoicePack>(getActivity(),android.R.layout.simple_list_item_multiple_choice,new ArrayList<VoicePack>(language.getVoices()));
        voices.sort(new DataPackNameComparator<VoicePack>());
        ListView listView=getListView();
        listView.setChoiceMode(ListView.CHOICE_MODE_MULTIPLE);
        setListAdapter(voices);
        int n=voices.getCount();
        for(int i=0;i<n;++i)
            {
                listView.setItemChecked(i,voices.getItem(i).getEnabled(getActivity()));
}
}

    @Override
    public void onListItemClick(ListView lv,View v,int pos,long id)
    {
        VoicePack voice=(VoicePack)lv.getItemAtPosition(pos);
        ((Listener)getActivity()).onVoiceSelected(voice,lv.isItemChecked(pos));
}

    @Override
    public void onStart()
    {
        super.onStart();
        ActionBar actionBar=getActivity().getActionBar();
        if(actionBar!=null)
            actionBar.setSubtitle(language.getDisplayName());
}
}

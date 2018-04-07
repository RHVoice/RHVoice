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

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.support.v4.app.ListFragment;
import android.support.v7.app.ActionBar;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import java.util.ArrayList;

public final class AvailableLanguagesFragment extends ListFragment
{
    public interface Listener
    {
        public void onLanguageSelected(LanguagePack language);
}

    @Override
    public void onActivityCreated(Bundle state)
    {
        super.onActivityCreated(state);
        ArrayAdapter<LanguagePack> languages=new ArrayAdapter<LanguagePack>(getActivity(),android.R.layout.simple_list_item_1,new ArrayList<LanguagePack>(Data.getLanguages()));
        languages.sort(new DataPackNameComparator<LanguagePack>());
        setListAdapter(languages);
}

    @Override
    public void onListItemClick(ListView lv,View v,int pos,long id)
    {
        LanguagePack language=(LanguagePack)lv.getItemAtPosition(pos);
        ((Listener)getActivity()).onLanguageSelected(language);
}

    @Override
    public void onStart()
    {
        super.onStart();
        ActionBar actionBar=((AppCompatActivity)getActivity()).getSupportActionBar();
        if(actionBar!=null)
            actionBar.setSubtitle(R.string.languages);
}
}

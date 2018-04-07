/* Copyright (C) 2013, 2014, 2016, 2017, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */


package com.github.olga_yakovleva.rhvoice.android;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.preference.PreferenceFragmentCompat;
import android.support.v7.preference.PreferenceScreen;

public final class SettingsActivity extends AppCompatActivity implements PreferenceFragmentCompat.OnPreferenceStartScreenCallback
{
    @Override
    public void onCreate(Bundle state)
    {
        super.onCreate(state);
        setContentView(R.layout.frame);
        if(state==null)
            {
                getSupportFragmentManager().beginTransaction().replace(R.id.frame,new SettingsFragment(),SettingsFragment.NAME).commit();
            }
    }

    @Override
    public boolean onPreferenceStartScreen(PreferenceFragmentCompat caller,PreferenceScreen pref)
    {
        String key=pref.getKey();
        if(key==null)
            throw new IllegalStateException();
        Bundle args=new Bundle();
        args.putString(SettingsFragment.ARG_LANGUAGE_KEY,key);
        SettingsFragment frag=new SettingsFragment();
        frag.setArguments(args);
        getSupportFragmentManager().beginTransaction().replace(R.id.frame,frag,key).addToBackStack(null).commit();
        return true;
}
}

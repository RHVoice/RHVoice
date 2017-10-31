/* Copyright (C) 2016  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.os.Bundle;

public abstract class UIActivity extends Activity implements DataInstallationFragment.SyncCompletionListener
{
    protected void replaceFragment(Fragment frag,String tag)
    {
        FragmentManager fm=getFragmentManager();
        FragmentTransaction ft=fm.beginTransaction();
        ft.replace(R.id.frame,frag,tag);
        ft.commit();
}

    @Override
    protected void onCreate(Bundle state)
    {
        super.onCreate(state);
        setContentView(R.layout.frame);
        if(state==null)
            replaceFragment(new DataInstallationFragment(),"data_installation_fragment");
}
}

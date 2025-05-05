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
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */


package com.github.olga_yakovleva.rhvoice.android;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceScreen;

public final class SettingsActivity extends AppCompatActivity implements PreferenceFragmentCompat.OnPreferenceStartScreenCallback, PreferenceFragmentCompat.OnPreferenceDisplayDialogCallback {
    @Override
    public void onCreate(Bundle state) {
        EdgeToEdge.enable(this);
        super.onCreate(state);
        setContentView(R.layout.frame);
        if (state == null) {
            getSupportFragmentManager().beginTransaction().replace(R.id.frame, new SettingsFragment(), SettingsFragment.NAME).commit();
        }
    }

    @Override
    public boolean onPreferenceStartScreen(PreferenceFragmentCompat caller, PreferenceScreen pref) {
        String key = pref.getKey();
        if (key == null)
            throw new IllegalStateException();
        Bundle args = new Bundle();
        args.putString(SettingsListFragment.ARG_LANGUAGE_KEY, key);
        SettingsListFragment frag = new SettingsListFragment();
        frag.setArguments(args);
        getSupportFragmentManager().beginTransaction().replace(R.id.frame, frag, key).addToBackStack(null).commit();
        return true;
    }

    @Override
    public boolean onPreferenceDisplayDialog(PreferenceFragmentCompat caller, Preference pref) {
        if (!(pref instanceof ProsodyPreference))
            return false;
        if (getSupportFragmentManager().findFragmentByTag(ProsodyPreferenceDialogFragment.TAG) != null)
            return true;
        ProsodyPreferenceDialogFragment f = ProsodyPreferenceDialogFragment.newInstance(pref.getKey());
        f.setTargetFragment(caller, 0);
        f.show(getSupportFragmentManager(), ProsodyPreferenceDialogFragment.TAG);
        return true;
    }

    public static void show(Context context) {
        Intent intent = new Intent(context, SettingsActivity.class);
        context.startActivity(intent);
    }
}

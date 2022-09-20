/* Copyright (C) 2017, 2018, 2021  Olga Yakovleva <olga@rhvoice.org> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

package com.github.olga_yakovleva.rhvoice.android;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;

import androidx.fragment.app.ListFragment;
import androidx.appcompat.app.ActionBar;

import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import java.util.ArrayList;

import com.google.common.collect.Ordering;

public final class AvailableLanguagesFragment extends ListFragment {
    private ArrayAdapter<VoiceAccent> accents;
    private DataManager dm;

    public interface Listener {
        public void onAccentSelected(VoiceAccent accent);
    }

    @Override
    public void onActivityCreated(Bundle state) {
        super.onActivityCreated(state);
        accents = new ArrayAdapter<VoiceAccent>(getActivity(), android.R.layout.simple_list_item_1);
        setListAdapter(accents);
        dm = new DataManager();
        Repository.get().getPackageDirectoryLiveData().observe(getViewLifecycleOwner(), this::onPackageDirectory);
    }

    @Override
    public void onListItemClick(ListView lv, View v, int pos, long id) {
        VoiceAccent accent = (VoiceAccent) lv.getItemAtPosition(pos);
        ((Listener) getActivity()).onAccentSelected(accent);
    }

    @Override
    public void onStart() {
        super.onStart();
        ActionBar actionBar = ((AppCompatActivity) getActivity()).getSupportActionBar();
        if (actionBar != null)
            actionBar.setSubtitle(R.string.languages);
    }

    private void onPackageDirectory(PackageDirectory dir) {
        dm.setPackageDirectory(dir);
        accents.setNotifyOnChange(false);
        accents.clear();
        accents.addAll(dm.getAccents());
        accents.sort(Ordering.usingToString());
        accents.notifyDataSetChanged();
    }
}

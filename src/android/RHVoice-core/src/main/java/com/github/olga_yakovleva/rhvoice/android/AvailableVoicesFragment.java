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
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

package com.github.olga_yakovleva.rhvoice.android;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;

import androidx.fragment.app.Fragment;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;
import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import java.util.ArrayList;

public final class AvailableVoicesFragment extends Fragment {
    public interface Listener {
        public void onVoiceSelected(VoicePack voice, boolean state);
    }

    public static final String ARG_LANGUAGE = "language";
    public static final String ARG_COUNTRY = "country";
    public static final String ARG_VARIANT = "variant";
    private DataManager dm;
    private VoiceAccent accent;
    private VoiceListAdapter adapter;

    private final BroadcastReceiver voiceInstalledReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String name = intent.getStringExtra("name");
            VoicePack voice = accent.getLanguage().findVoice(name);
            if (voice != null)
                refresh(voice, VoiceViewChange.INSTALLED);
        }
    };

    private final BroadcastReceiver languageInstalledReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String name = intent.getStringExtra("name");
            if (name.equals(accent.getLanguage().getName()))
                refresh();
        }
    };

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle state) {
        return inflater.inflate(R.layout.voice_list, container, false);
    }

    @Override
    public void onViewCreated(View view, Bundle state) {
        super.onViewCreated(view, state);
        dm = new DataManager();
        adapter = new VoiceListAdapter(getActivity());
        RecyclerView listView = view.findViewById(R.id.voice_list);
        listView.setHasFixedSize(true);
        listView.setLayoutManager(new LinearLayoutManager(getActivity()));
        listView.setAdapter(adapter);
        Repository.get().getPackageDirectoryLiveData().observe(getViewLifecycleOwner(), this::onPackageDirectory);
    }

    private void onPackageDirectory(PackageDirectory dir) {
        dm.setPackageDirectory(dir);
        final Bundle args = getArguments();
        LanguagePack language = dm.getLanguageById(args.getString(ARG_LANGUAGE));
        accent = language.getAccent(args.getString(ARG_COUNTRY), args.getString(ARG_VARIANT));
        ActionBar actionBar = ((AppCompatActivity) getActivity()).getSupportActionBar();
        if (actionBar != null)
            actionBar.setSubtitle(accent.getDisplayName());
        adapter.setAccent(accent);
    }

    @Override
    public void onStart() {
        super.onStart();
        IntentFilter filter = new IntentFilter(DataSyncWorker.ACTION_VOICE_INSTALLED);
        filter.addAction(DataSyncWorker.ACTION_VOICE_REMOVED);
        LocalBroadcastManager.getInstance(getActivity()).registerReceiver(voiceInstalledReceiver, filter);
        filter = new IntentFilter(DataSyncWorker.ACTION_LANGUAGE_INSTALLED);
        filter.addAction(DataSyncWorker.ACTION_LANGUAGE_REMOVED);
        LocalBroadcastManager.getInstance(getActivity()).registerReceiver(languageInstalledReceiver, filter);
        if (accent != null)
            refresh();
    }

    @Override
    public void onStop() {
        super.onStop();
        LocalBroadcastManager.getInstance(getActivity()).unregisterReceiver(voiceInstalledReceiver);
        PlayerFragment pf = (PlayerFragment) (getActivity().getSupportFragmentManager().findFragmentByTag("player"));
        if (pf != null)
            pf.stopPlayback();
    }

    public void refresh() {
        adapter.notifyDataSetChanged();
    }

    public void refresh(VoicePack v) {
        adapter.notifyVoiceItemChanged(v, VoiceViewChange.ALL);
    }

    public void refresh(VoicePack v, Long flags) {
        adapter.notifyVoiceItemChanged(v, flags);
    }
}

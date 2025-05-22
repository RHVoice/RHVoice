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

import android.text.Spanned;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.TextView;

import androidx.fragment.app.FragmentActivity;
import androidx.recyclerview.widget.RecyclerView;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.google.common.collect.Iterables;

public final class VoiceListAdapter extends RecyclerView.Adapter<VoiceViewHolder> {
    private class ActionButtonListener implements View.OnClickListener {
        private final VoicePack voice;
        private final boolean flag;

        public ActionButtonListener(VoicePack voice, boolean flag) {
            this.voice = voice;
            this.flag = flag;
        }

        public void onClick(View v) {
            ((AvailableVoicesFragment.Listener) activity).onVoiceSelected(voice, flag);
        }
    }

    private class PlayButtonListener implements View.OnClickListener {
        private final VoicePack voice;

        public PlayButtonListener(VoicePack voice) {
            this.voice = voice;
        }

        public void onClick(View v) {
            PlayerFragment f = findPlayerFragment();
            if (f == null)
                return;
            if (f.isPlaying(voice))
                f.stopPlayback();
            else
                f.play(voice);
        }
    }

    private final FragmentActivity activity;
    private VoiceAccent accent;
    private final List<VoicePack> voices = new ArrayList<>();
    private final LayoutInflater inflater;

    private PlayerFragment findPlayerFragment() {
        return (PlayerFragment) (activity.getSupportFragmentManager().findFragmentByTag("player"));
    }

    private String getVersionString(VoicePack voice) {
        if (!voice.getEnabled(activity))
            return null;
        final Version ver = voice.getInstalledVersion(activity);
        if (ver == null)
            return null;
        final Version langVer = voice.getLanguage().getInstalledVersion(activity);
        if (langVer == null)
            return ver.toString();
        return (ver.toString() + "." + langVer.toString());
    }

    public VoiceListAdapter(FragmentActivity activity) {
        this.activity = activity;
        inflater = (LayoutInflater) activity.getSystemService(FragmentActivity.LAYOUT_INFLATER_SERVICE);
    }

    public void setAccent(VoiceAccent accent) {
        this.accent = accent;
        voices.clear();
        voices.addAll(accent.getVoices());
        Collections.<VoicePack>sort(voices, new DataPackNameComparator<VoicePack>());
        notifyDataSetChanged();
    }

    @Override
    public int getItemCount() {
        return voices.size();
    }

    @Override
    public VoiceViewHolder onCreateViewHolder(ViewGroup parent, int viewType) {
        return new VoiceViewHolder(inflater.inflate(R.layout.voice_list_item, parent, false));
    }

    @Override
    public void onBindViewHolder(VoiceViewHolder vh, int pos) {
        onBindViewHolder(vh, pos, null);
    }

    @Override
    public void onBindViewHolder(VoiceViewHolder vh, int pos, List<Object> changes) {
        long flags = 0;
        if (changes == null || changes.isEmpty())
            flags = VoiceViewChange.ALL;
        else {
            for (Object change : changes)
                flags |= ((Long) change);
        }
        VoicePack voice = voices.get(pos);
        if ((flags & VoiceViewChange.NAME) != 0)
            vh.nameView.setText(voice.getName());
        if ((flags & VoiceViewChange.INSTALLED) != 0) {
            Spanned attrib = voice.getAttribution(activity);
            vh.attribView.setVisibility(attrib == null ? View.GONE : View.VISIBLE);
            if (attrib != null) {
                vh.attribView.setText(attrib);
            }
            String ver = getVersionString(voice);
            vh.versionView.setVisibility(ver == null ? View.GONE : View.VISIBLE);
            if (ver != null)
                vh.versionView.setText(ver);
            boolean enabled = voice.getEnabled(activity);
            boolean installed = voice.isInstalled(activity);
            boolean upToDate = voice.isUpToDate(activity);
            if (enabled && !upToDate)
                vh.progressBar.setVisibility(View.VISIBLE);
            else
                vh.progressBar.setVisibility(View.GONE);
            if (enabled) {
                if (installed) {
                    vh.actionButton.setIconResource(R.drawable.ic_delete);
                    vh.actionButton.setContentDescription(activity.getString(R.string.uninstall));
                } else {
                    vh.actionButton.setIconResource(R.drawable.ic_cancel);
                    vh.actionButton.setContentDescription(activity.getString(android.R.string.cancel));
                }
                vh.actionButton.setOnClickListener(this.new ActionButtonListener(voice, false));
            } else {
                vh.actionButton.setIconResource(R.drawable.ic_download);
                vh.actionButton.setContentDescription(activity.getString(R.string.install));
                vh.actionButton.setOnClickListener(this.new ActionButtonListener(voice, true));
            }
        }
        if ((flags & VoiceViewChange.PLAYING) != 0) {
            PlayerFragment pf = findPlayerFragment();
            if (pf != null && pf.canPlay(voice)) {
                if (pf.isPlaying(voice)) {
                    vh.playButton.setIconResource(R.drawable.ic_stop);
                    vh.playButton.setContentDescription(activity.getString(R.string.stop));
                } else {
                    vh.playButton.setIconResource(R.drawable.ic_play);
                    vh.playButton.setContentDescription(activity.getString(R.string.play));
                }
                vh.playButton.setVisibility(View.VISIBLE);
                vh.playButton.setOnClickListener(this.new PlayButtonListener(voice));
            } else {
                vh.playButton.setOnClickListener(null);
                vh.playButton.setVisibility(View.GONE);
            }
        }
    }

    public void notifyVoiceItemChanged(VoicePack v, Long change) {
        int i = Iterables.indexOf(voices, a -> a.getId().equals(v.getId()));
        if (i < 0)
            throw new IllegalArgumentException();
        notifyItemChanged(i, change);
    }
}

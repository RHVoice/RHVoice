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

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatDialogFragment;

import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;

import com.google.android.material.dialog.MaterialAlertDialogBuilder;

public final class ConfirmVoiceRemovalDialogFragment extends AppCompatDialogFragment {
    public interface Listener {
        public void onConfirmVoiceRemovalResponse(VoicePack voice, boolean response);
    }

    private static final String ARG_LANGUAGE = "language";
    private static final String ARG_VOICE = "voice";

    private VoicePack voice;

    private void onResponse(boolean response) {
        if (voice == null)
            return;
        ((Listener) getActivity()).onConfirmVoiceRemovalResponse(voice, response);
    }

    private class ClickListener implements DialogInterface.OnClickListener {
        private final boolean response;

        public ClickListener(boolean response) {
            this.response = response;
        }

        @Override
        public void onClick(DialogInterface dialog, int id) {
            onResponse(response);
        }
    }

    @Override
    public void onCreate(Bundle state) {
        super.onCreate(state);
        Bundle args = getArguments();
        String languageId = args.getString(ARG_LANGUAGE);
        String voiceId = args.getString(ARG_VOICE);
        LanguagePack language = Repository.get().createDataManager().getLanguageById(languageId);
        if (language != null)
            voice = language.findVoiceById(voiceId);
    }

    @Override
    public Dialog onCreateDialog(Bundle state) {
        MaterialAlertDialogBuilder builder = new MaterialAlertDialogBuilder(getActivity());
        if (voice != null)
            builder.setTitle(voice.getName());
        builder.setMessage(R.string.voice_remove_question);
        builder.setPositiveButton(android.R.string.ok, this.new ClickListener(true));
        builder.setNegativeButton(android.R.string.cancel, this.new ClickListener(false));
        return builder.create();
    }

    @Override
    public void onCancel(DialogInterface dialog) {
        onResponse(false);
    }

    public static void show(AppCompatActivity activity, VoicePack voice) {
        Bundle args = new Bundle();
        args.putString(ARG_LANGUAGE, voice.getLanguage().getId());
        args.putString(ARG_VOICE, voice.getId());
        ConfirmVoiceRemovalDialogFragment frag = new ConfirmVoiceRemovalDialogFragment();
        frag.setArguments(args);
        frag.show(activity.getSupportFragmentManager(), "confirm_voice_removal");
    }
}

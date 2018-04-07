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
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatDialogFragment;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;

public final class ConfirmVoiceRemovalDialogFragment extends AppCompatDialogFragment
{
    public interface Listener
    {
        public void onConfirmVoiceRemovalResponse(VoicePack voice,boolean response);
}

    private static final String ARG_LANGUAGE="language";
    private static final String ARG_VOICE="voice";

    private VoicePack voice;

    private void onResponse(boolean response)
    {
        if(voice==null)
            return;
        ((Listener)getActivity()).onConfirmVoiceRemovalResponse(voice,response);
    }

    private class ClickListener implements DialogInterface.OnClickListener
    {
        private final boolean response;

        public ClickListener(boolean response)
        {
            this.response=response;
}

        @Override
        public void onClick(DialogInterface dialog,int id)
        {
            onResponse(response);
}
}

    @Override
    public void onCreate(Bundle state)
    {
        super.onCreate(state);
        Bundle args=getArguments();
        String languageName=args.getString(ARG_LANGUAGE);
        String voiceName=args.getString(ARG_VOICE);
        LanguagePack language=Data.getLanguage(languageName);
        if(language!=null)
            voice=language.findVoice(voiceName);
}

    @Override
    public Dialog onCreateDialog(Bundle state)
    {
        AlertDialog.Builder builder=new AlertDialog.Builder(getActivity());
        builder.setMessage(R.string.voice_remove_question);
        builder.setPositiveButton(android.R.string.ok,this.new ClickListener(true));
        builder.setNegativeButton(android.R.string.cancel,this.new ClickListener(false));
        return builder.create();
}

    @Override
    public void onCancel(DialogInterface dialog)
    {
        onResponse(false);
}

    public static void show(AppCompatActivity activity,VoicePack voice)
    {
        Bundle args=new Bundle();
        args.putString(ARG_LANGUAGE,voice.getLanguage().getCode());
        args.putString(ARG_VOICE,voice.getName());
        ConfirmVoiceRemovalDialogFragment frag=new ConfirmVoiceRemovalDialogFragment();
        frag.setArguments(args);
        frag.show(activity.getSupportFragmentManager(),"confirm_voice_removal");
}
}

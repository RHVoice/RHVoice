/* Copyright (C) 2021  Olga Yakovleva <olga@rhvoice.org> */

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

import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;

import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDialogFragment;
import androidx.fragment.app.FragmentActivity;
import androidx.localbroadcastmanager.content.LocalBroadcastManager;

import com.google.common.collect.FluentIterable;

import java.io.File;
import java.util.Arrays;

public final class RemoveUserDictDialogFragment extends AppCompatDialogFragment {
    private static final String ARG_LANGUAGE = "language";
    private String languageName;
    private String[] userDicts;

    public void onCreate(Bundle state) {
        super.onCreate(state);
        Bundle args = getArguments();
        languageName = args.getString(ARG_LANGUAGE);
        final File[] dictFiles = Config.getLangDictsDir(requireActivity(), languageName).listFiles();
        if (dictFiles == null)
            userDicts = new String[]{};
        else
            userDicts = FluentIterable.from(dictFiles).filter(f -> f.isFile()).transform(f -> f.getName()).toArray(String.class);
        Arrays.sort(userDicts);
    }

    @Override
    public Dialog onCreateDialog(Bundle state) {
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setTitle(R.string.remove);
        builder.setItems(userDicts, this::onDictSelected);
        return builder.create();
    }

    private void onDictSelected(DialogInterface dialog, int index) {
        final File dictFile = new File(Config.getLangDictsDir(getActivity(), languageName), userDicts[index]);
        dictFile.delete();
        LocalBroadcastManager.getInstance(getActivity()).sendBroadcast(new Intent(RHVoiceService.ACTION_CONFIG_CHANGE));
    }

    public static void show(FragmentActivity activity, String langName) {
        if (langName == null)
            return;
        Bundle args = new Bundle();
        args.putString(ARG_LANGUAGE, langName);
        RemoveUserDictDialogFragment frag = new RemoveUserDictDialogFragment();
        frag.setArguments(args);
        frag.show(activity.getSupportFragmentManager(), "remove_user_dict");
    }
}

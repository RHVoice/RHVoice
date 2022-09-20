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

import android.app.IntentService;
import android.content.Intent;

import androidx.documentfile.provider.DocumentFile;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.FileOutputStream;

import androidx.localbroadcastmanager.content.LocalBroadcastManager;

public final class ImportConfigService extends IntentService {
    public static final String ACTION_IMPORT_USER_DICT = "org.rhvoice.action.IMPORT_USER_DICT";
    public static final String ACTION_IMPORT_CONFIG_FILE = "org.rhvoice.action.IMPORT_CONFIG_FILE";
    public static final String EXTRA_LANGUAGE = "language";

    public ImportConfigService() {
        super("RHVoice.ImportConfigService");
    }

    private void importUserDict(Intent intent) {
        final DocumentFile inFile = DocumentFile.fromSingleUri(this, intent.getData());
        final String lang = intent.getStringExtra(EXTRA_LANGUAGE);
        final File outDir = Config.getLangDictsDir(this, lang);
        outDir.mkdirs();
        final File outFile = new File(outDir, inFile.getName());
        try (InputStream in = getContentResolver().openInputStream(intent.getData());
             FileOutputStream out = new FileOutputStream(outFile)) {
            DataPack.copyBytes(in, out, null);
        } catch (IOException e) {
            outFile.delete();
            return;
        }
        LocalBroadcastManager.getInstance(this).sendBroadcast(new Intent(RHVoiceService.ACTION_CONFIG_CHANGE));
    }

    private void importConfigFile(Intent intent) {
        final DocumentFile inFile = DocumentFile.fromSingleUri(this, intent.getData());
        final File outFile = Config.getConfigFile(this);
        outFile.getParentFile().mkdirs();
        try (InputStream in = getContentResolver().openInputStream(intent.getData());
             FileOutputStream out = new FileOutputStream(outFile)) {
            DataPack.copyBytes(in, out, null);
        } catch (IOException e) {
            outFile.delete();
            return;
        }
        LocalBroadcastManager.getInstance(this).sendBroadcast(new Intent(RHVoiceService.ACTION_CONFIG_CHANGE));
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        switch (intent.getAction()) {
            case ACTION_IMPORT_USER_DICT:
                importUserDict(intent);
                break;
            case ACTION_IMPORT_CONFIG_FILE:
                importConfigFile(intent);
                break;
            default:
                break;
        }
    }
}

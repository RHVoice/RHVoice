/* Copyright (C) 2014  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.Context;
import android.util.Log;

public final class Config {
    private static final String TAG = "RHVoiceConfig";
    private static final String CONFIG_FILE_NAME = "RHVoice.conf";

    public static File getDir(Context context) {
        if (BuildConfig.DEBUG)
            Log.d(TAG, "Requesting path to the private external storage directory");
        File dir = context.getExternalFilesDir(null);
        if (dir == null) {
            if (BuildConfig.DEBUG)
                Log.d(TAG, "The private external storage directory does not exist");
            return context.getDir("config", 0);
        }
        if (BuildConfig.DEBUG)
            Log.d(TAG, "The path to the private external storage directory is " + dir.getAbsolutePath());
        return dir;
    }

    public static File getDictsRootDir(Context ctx) {
        return new File(getDir(ctx), "dicts");
    }

    public static File getLangDictsDir(Context ctx, String langName) {
        return new File(getDictsRootDir(ctx), langName);
    }

    public static File getConfigFile(Context ctx) {
        return new File(getDir(ctx), CONFIG_FILE_NAME);
    }
}

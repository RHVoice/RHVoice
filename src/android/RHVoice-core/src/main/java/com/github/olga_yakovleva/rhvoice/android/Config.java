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

    public static File getEngineDir(Context context) {
        if (DirectBoot.isInDirectBoot(context))
            return getDirectBootDir(context);
        return getDir(context);
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

    private static File getDirectBootDir(Context context) {
        if (!DirectBoot.isSupported())
            return getDir(context);
        return DirectBoot.getDir(context, "config");
    }

    public static File getDirectBootDictsRootDir(Context ctx) {
        return new File(getDirectBootDir(ctx), "dicts");
    }

    public static File getDirectBootLangDictsDir(Context ctx, String langName) {
        return new File(getDirectBootDictsRootDir(ctx), langName);
    }

    public static File getDirectBootConfigFile(Context ctx) {
        return new File(getDirectBootDir(ctx), CONFIG_FILE_NAME);
    }

    public static boolean hasConfigFile(Context ctx) {
        return getConfigFile(ctx).exists() || getDirectBootConfigFile(ctx).exists();
    }

    public static void deleteConfigFile(Context ctx) {
        getConfigFile(ctx).delete();
        getDirectBootConfigFile(ctx).delete();
    }

    public static void deleteUserDict(Context ctx, String langName, String fileName) {
        new File(getLangDictsDir(ctx, langName), fileName).delete();
        new File(getDirectBootLangDictsDir(ctx, langName), fileName).delete();
    }

    public static void syncToDirectBootStorage(Context ctx) {
        if (!DirectBoot.isSupported() || !DirectBoot.isUserUnlocked(ctx))
            return;
        final File sourceDir = getDir(ctx);
        final File destinationDir = getDirectBootDir(ctx);
        if (sourceDir.equals(destinationDir))
            return;
        boolean configSynced = syncFile(getConfigFile(ctx), getDirectBootConfigFile(ctx));
        boolean dictsSynced = syncDirectory(getDictsRootDir(ctx), getDirectBootDictsRootDir(ctx));
        if (BuildConfig.DEBUG && !configSynced)
            Log.w(TAG, "Unable to synchronize configuration file to direct boot storage");
        if (BuildConfig.DEBUG && !dictsSynced)
            Log.w(TAG, "Unable to synchronize dictionaries to direct boot storage");
    }

    private static boolean syncFile(File source, File destination) {
        if (source.exists())
            return DirectBoot.copyDirectory(source, destination);
        return destination.delete() || !destination.exists();
    }

    private static boolean syncDirectory(File source, File destination) {
        if (source.exists()) {
            if (!DirectBoot.copyDirectory(source, destination))
                return false;
            deleteMissingChildren(source, destination);
            return true;
        } else {
            return DirectBoot.delete(destination);
        }
    }

    private static void deleteMissingChildren(File source, File destination) {
        File[] children = destination.listFiles();
        if (children == null)
            return;
        for (File child : children) {
            File sourceChild = new File(source, child.getName());
            if (!sourceChild.exists()) {
                DirectBoot.delete(child);
            } else if (child.isDirectory()) {
                deleteMissingChildren(sourceChild, child);
            }
        }
    }
}

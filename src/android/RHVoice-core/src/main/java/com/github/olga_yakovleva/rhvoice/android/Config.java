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
        File dir = null;
        try {
            dir = context.getExternalFilesDir(null);
        } catch (RuntimeException e) {
            if (BuildConfig.DEBUG)
                Log.w(TAG, "Unable to get the private external storage directory", e);
        }
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
        return exists(getConfigFile(ctx)) || exists(getDirectBootConfigFile(ctx));
    }

    public static void deleteConfigFile(Context ctx) {
        DirectBoot.delete(getConfigFile(ctx));
        DirectBoot.delete(getDirectBootConfigFile(ctx));
    }

    public static void deleteUserDict(Context ctx, String langName, String fileName) {
        DirectBoot.delete(new File(getLangDictsDir(ctx, langName), fileName));
        DirectBoot.delete(new File(getDirectBootLangDictsDir(ctx, langName), fileName));
    }

    public static void syncToDirectBootStorage(Context ctx) {
        if (!DirectBoot.isSupported() || !DirectBoot.isUserUnlocked(ctx))
            return;
        try {
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
        } catch (RuntimeException e) {
            if (BuildConfig.DEBUG)
                Log.w(TAG, "Unable to synchronize configuration to direct boot storage", e);
        }
    }

    private static boolean syncFile(File source, File destination) {
        if (exists(source))
            return DirectBoot.copyDirectory(source, destination);
        return DirectBoot.delete(destination);
    }

    private static boolean syncDirectory(File source, File destination) {
        if (exists(source)) {
            if (!DirectBoot.copyDirectory(source, destination))
                return false;
            deleteMissingChildren(source, destination);
            return true;
        } else {
            return DirectBoot.delete(destination);
        }
    }

    private static void deleteMissingChildren(File source, File destination) {
        File[] children;
        try {
            children = destination.listFiles();
        } catch (SecurityException e) {
            if (BuildConfig.DEBUG)
                Log.w(TAG, "Unable to list " + destination.getAbsolutePath(), e);
            return;
        }
        if (children == null)
            return;
        for (File child : children) {
            File sourceChild = new File(source, child.getName());
            if (!exists(sourceChild)) {
                DirectBoot.delete(child);
            } else if (isDirectory(sourceChild) != isDirectory(child)) {
                DirectBoot.delete(child);
            } else if (isDirectory(child)) {
                deleteMissingChildren(sourceChild, child);
            }
        }
    }

    private static boolean exists(File file) {
        try {
            return file.exists();
        } catch (SecurityException e) {
            if (BuildConfig.DEBUG)
                Log.w(TAG, "Unable to check " + file.getAbsolutePath(), e);
            return false;
        }
    }

    private static boolean isDirectory(File file) {
        try {
            return file.isDirectory();
        } catch (SecurityException e) {
            if (BuildConfig.DEBUG)
                Log.w(TAG, "Unable to check whether " + file.getAbsolutePath() + " is a directory", e);
            return false;
        }
    }
}

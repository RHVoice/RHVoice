/* Copyright (C) 2026  RHVoice contributors */

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

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.UserManager;
import android.util.Log;

import androidx.core.content.ContextCompat;
import androidx.preference.PreferenceManager;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

final class DirectBoot {
    private static final String TAG = "RHVoice.DirectBoot";
    private static final String[] BACKGROUND_PRIVATE_DIRS = {"data"};
    private static final String PACKAGE_DIR = "packages";
    private static final String MIGRATION_PREFS = "direct_boot";
    private static final String MIGRATED_PREF_PREFIX = "migrated.preferences.";
    private static final String MIGRATED_DIR_PREFIX = "migrated.dir.";
    private static final ExecutorService migrationExecutor = Executors.newSingleThreadExecutor();

    private DirectBoot() {
    }

    public static boolean isSupported() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.N;
    }

    public static boolean isUserUnlocked(Context context) {
        if (!isSupported())
            return true;
        UserManager manager = (UserManager) context.getSystemService(Context.USER_SERVICE);
        return manager == null || manager.isUserUnlocked();
    }

    public static boolean isInDirectBoot(Context context) {
        return isSupported() && !isUserUnlocked(context);
    }

    public static Context getDeviceProtectedContext(Context context) {
        Context appContext = context.getApplicationContext();
        if (!isSupported())
            return appContext;
        if (appContext.isDeviceProtectedStorage())
            return appContext;
        Context directContext = ContextCompat.createDeviceProtectedStorageContext(appContext);
        return directContext == null ? appContext : directContext;
    }

    public static SharedPreferences getDefaultSharedPreferences(Context context) {
        return PreferenceManager.getDefaultSharedPreferences(getDeviceProtectedContext(context));
    }

    public static SharedPreferences getSharedPreferences(Context context, String name) {
        return getDeviceProtectedContext(context).getSharedPreferences(name, Context.MODE_PRIVATE);
    }

    public static File getDir(Context context, String name) {
        return getDeviceProtectedContext(context).getDir(name, Context.MODE_PRIVATE);
    }

    public static void migrate(Context context) {
        migrate(context, null);
    }

    public static void migrate(Context context, Runnable onComplete) {
        if (!isSupported()) {
            if (onComplete != null)
                onComplete.run();
            return;
        }
        if (!isUserUnlocked(context))
            return;
        Context appContext = context.getApplicationContext();
        Context directContext = getDeviceProtectedContext(appContext);
        if (appContext == directContext) {
            if (onComplete != null)
                onComplete.run();
            return;
        }
        SharedPreferences state = getMigrationState(directContext);
        synchronized (DirectBoot.class) {
            migrateSharedPreferences(appContext, directContext, state);
            migratePrivateDir(appContext, directContext, state, PACKAGE_DIR);
        }
        migrationExecutor.execute(() -> {
            Config.syncToDirectBootStorage(appContext);
            migratePrivateDirs(appContext, directContext, state);
            if (onComplete != null)
                onComplete.run();
        });
    }

    private static SharedPreferences getMigrationState(Context directContext) {
        return directContext.getSharedPreferences(MIGRATION_PREFS, Context.MODE_PRIVATE);
    }

    private static boolean isMigrationDone(SharedPreferences state, String key) {
        return state.getBoolean(key, false);
    }

    private static void markMigrationDone(SharedPreferences state, String key) {
        if (!state.edit().putBoolean(key, true).commit() && BuildConfig.DEBUG)
            Log.w(TAG, "Unable to persist migration marker " + key);
    }

    private static void migrateSharedPreferences(Context sourceContext, Context directContext, SharedPreferences state) {
        moveSharedPreferences(sourceContext, directContext, state, getDefaultSharedPreferencesName(sourceContext));
        moveSharedPreferences(sourceContext, directContext, state, "dir");
    }

    private static String getDefaultSharedPreferencesName(Context context) {
        return context.getPackageName() + "_preferences";
    }

    private static void moveSharedPreferences(Context sourceContext, Context directContext, SharedPreferences state, String name) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N)
            return;
        String key = MIGRATED_PREF_PREFIX + name;
        if (isMigrationDone(state, key))
            return;
        if (!sharedPreferencesFileExists(sourceContext, name)) {
            markMigrationDone(state, key);
            return;
        }
        try {
            if (directContext.moveSharedPreferencesFrom(sourceContext, name) || !sharedPreferencesFileExists(sourceContext, name)) {
                markMigrationDone(state, key);
            } else if (BuildConfig.DEBUG) {
                Log.w(TAG, "Unable to migrate shared preferences " + name);
            }
        } catch (RuntimeException e) {
            if (BuildConfig.DEBUG)
                Log.w(TAG, "Unable to migrate shared preferences " + name, e);
        }
    }

    private static boolean sharedPreferencesFileExists(Context context, String name) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N)
            return false;
        File prefsDir = new File(context.getDataDir(), "shared_prefs");
        return new File(prefsDir, name + ".xml").exists() || new File(prefsDir, name + ".xml.bak").exists();
    }

    private static void migratePrivateDirs(Context sourceContext, Context directContext, SharedPreferences state) {
        for (String name : BACKGROUND_PRIVATE_DIRS)
            migratePrivateDir(sourceContext, directContext, state, name);
    }

    private static void migratePrivateDir(Context sourceContext, Context directContext, SharedPreferences state, String name) {
        File source = getPrivateDir(sourceContext, name);
        String key = MIGRATED_DIR_PREFIX + name;
        if (isMigrationDone(state, key)) {
            delete(source);
            return;
        }
        if (!source.exists()) {
            markMigrationDone(state, key);
            return;
        }
        File destination = getPrivateDir(directContext, name);
        if (sameFile(source, destination)) {
            markMigrationDone(state, key);
            return;
        }
        File parent = destination.getParentFile();
        if (parent == null || (!parent.isDirectory() && !parent.mkdirs()))
            return;
        if (!destination.exists() && source.renameTo(destination)) {
            markMigrationDone(state, key);
            return;
        }
        if (copyDirectory(source, destination, true)) {
            markMigrationDone(state, key);
            delete(source);
        }
    }

    private static File getPrivateDir(Context context, String name) {
        if (isSupported())
            return new File(context.getDataDir(), "app_" + name);
        return context.getDir(name, Context.MODE_PRIVATE);
    }

    private static boolean sameFile(File first, File second) {
        try {
            return first.getCanonicalFile().equals(second.getCanonicalFile());
        } catch (IOException e) {
            return first.getAbsolutePath().equals(second.getAbsolutePath());
        }
    }

    static boolean copyDirectory(File source, File destination) {
        return copyDirectory(source, destination, false);
    }

    private static boolean copyDirectory(File source, File destination, boolean keepNewerDestination) {
        if (!source.exists())
            return true;
        if (source.isDirectory()) {
            if (destination.exists() && !destination.isDirectory() && !delete(destination))
                return false;
            if (!destination.isDirectory() && !destination.mkdirs())
                return false;
            File[] children = source.listFiles();
            if (children == null)
                return false;
            for (File child : children) {
                File childDestination = new File(destination, child.getName());
                if (!copyDirectory(child, childDestination, keepNewerDestination))
                    return false;
            }
            return true;
        } else {
            return copyFile(source, destination, keepNewerDestination);
        }
    }

    private static boolean copyFile(File source, File destination, boolean keepNewerDestination) {
        if (destination.exists()) {
            if (destination.isDirectory()) {
                if (!delete(destination))
                    return false;
            } else {
                if (keepNewerDestination && destination.lastModified() >= source.lastModified())
                    return true;
                if (destination.lastModified() == source.lastModified() && destination.length() == source.length())
                    return true;
            }
        }
        File parent = destination.getParentFile();
        if (parent == null || (!parent.isDirectory() && !parent.mkdirs()))
            return false;
        File tmp;
        try {
            tmp = File.createTempFile(getTempFilePrefix(destination), ".tmp", parent);
        } catch (IOException e) {
            if (BuildConfig.DEBUG)
                Log.w(TAG, "Unable to create temporary file for " + destination.getAbsolutePath(), e);
            return false;
        }
        try (BufferedInputStream in = new BufferedInputStream(new FileInputStream(source));
             BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(tmp))) {
            DataPack.copyBytes(in, out, null);
        } catch (IOException e) {
            if (BuildConfig.DEBUG)
                Log.w(TAG, "Unable to copy " + source.getAbsolutePath(), e);
            delete(tmp);
            return false;
        }
        tmp.setLastModified(source.lastModified());
        if (destination.exists() && !delete(destination)) {
            delete(tmp);
            return false;
        }
        if (!tmp.renameTo(destination)) {
            delete(tmp);
            return false;
        }
        destination.setLastModified(source.lastModified());
        return true;
    }

    private static String getTempFilePrefix(File destination) {
        String name = destination.getName();
        return name.length() >= 3 ? name : "rhv";
    }

    static boolean delete(File obj) {
        if (!obj.exists())
            return true;
        if (obj.isDirectory()) {
            File[] children = obj.listFiles();
            if (children != null) {
                for (File child : children) {
                    if (!delete(child))
                        return false;
                }
            }
        }
        return obj.delete();
    }
}

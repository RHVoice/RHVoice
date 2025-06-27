/* Copyright (C) 2025  Darko Milosevic <daremc86@gmail.com> */

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
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.work.Worker;
import androidx.work.WorkerParameters;
import androidx.work.ExistingWorkPolicy;

import android.os.Build;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;

public final class MigrationWorker extends Worker {
    private static final String TAG = "RHVoice.MigrationWorker";

    public MigrationWorker(@NonNull Context context, @NonNull WorkerParameters params) {
        super(context, params);
    }

    @Override
    public Result doWork() {
        Context deCtx = MyApplication.getStorageContext();
        Context ceCtx = getApplicationContext();


        migrateDir("app_data", new File(ceCtx.getDataDir(), "app_data"), new File(deCtx.getDataDir(), "app_data"));
        migrateDir("app_packages", new File(ceCtx.getDataDir(), "app_packages"), new File(deCtx.getDataDir(), "app_packages"));
        migrateDir("cache", new File(ceCtx.getDataDir(), "cache"), new File(deCtx.getDataDir(), "cache"));
        migrateDir("code_cache", new File(ceCtx.getDataDir(), "code_cache"), new File(deCtx.getDataDir(), "code_cache"));
        migrateDir("files", new File(ceCtx.getDataDir(), "files"), new File(deCtx.getDataDir(), "files"));
        migrateDir("shared_prefs", new File(ceCtx.getDataDir(), "shared_prefs"), new File(deCtx.getDataDir(), "shared_prefs"));
        migrateDir("databases", new File(ceCtx.getDataDir(), "databases"), new File(deCtx.getDataDir(), "databases"));
        migrateDir("no_backup", new File(ceCtx.getNoBackupFilesDir().getPath()), new File(deCtx.getNoBackupFilesDir().getPath()));

        return Result.success();
    }

    private boolean copyFile(File src, File dst) {
        try (InputStream in = new FileInputStream(src);
            OutputStream out = new FileOutputStream(dst)) {
            byte[] buffer = new byte[4096];
            int length;
            while ((length = in.read(buffer)) > 0) {
                out.write(buffer, 0, length);
            }
            return true;
        } catch (Exception e) {
            Log.w(TAG, "Failed to copy file: " + src.getName(), e);
            return false;
        }
    }

    private void migrateDir(String name, File srcDir, File dstDir) {
        if (!srcDir.exists()) return;
        if (!dstDir.exists()) dstDir.mkdirs();

        File[] files = srcDir.listFiles();
        if (files == null) return;

        for (File srcFile : files) {
            File dstFile = new File(dstDir, srcFile.getName());
            if (dstFile.exists()) continue;

            boolean success = copyFile(srcFile, dstFile);
            if (!success) {
                if (BuildConfig.DEBUG)
                    Log.w(TAG, "Failed to move " + name + ": " + srcFile.getName());
            } else {
                srcFile.delete();
                if (BuildConfig.DEBUG)
                    Log.v(TAG, "Moved " + srcFile.getAbsolutePath() + " to " + dstFile.getAbsolutePath());
            }
        }
    }
}

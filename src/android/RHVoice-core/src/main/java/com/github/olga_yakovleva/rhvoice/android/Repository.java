/* Copyright (C) 2021, 2022  Olga Yakovleva <olga@rhvoice.org> */

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
import android.os.SystemClock;
import android.util.Log;

import androidx.annotation.MainThread;
import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.work.Constraints;
import androidx.work.ExistingPeriodicWorkPolicy;
import androidx.work.NetworkType;
import androidx.work.PeriodicWorkRequest;
import androidx.work.WorkManager;

import com.github.olga_yakovleva.rhvoice.TTSEngine;
import com.google.common.base.MoreObjects;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.ListeningExecutorService;
import com.google.common.util.concurrent.MoreExecutors;
import com.squareup.moshi.JsonAdapter;
import com.squareup.moshi.Moshi;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.time.Instant;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import okio.BufferedSource;
import okio.Okio;

import java.io.InputStreamReader;

import com.google.common.io.CharStreams;

import android.content.SharedPreferences;

import androidx.core.content.ContextCompat;

final class Repository {
    private static final String TAG = "RHVoice.Repository";
    private static volatile Repository instance;
    private final Context context;
    private volatile PackageDirectory pkgDir;
    private final MutableLiveData<PackageDirectory> pkgDirLiveData;
    private final Moshi moshi;
    private final JsonAdapter<PackageDirectory> jsonAdapter;
    private final ListeningExecutorService exec;
    private final TTSEngine engine;

    @MainThread
    private Repository(Context context) {
        this.context = context.getApplicationContext();
        try {
            {
                engine = new TTSEngine("", Config.getDir(this.context).getAbsolutePath(), new String[0], PackageClient.getPath(this.context), CoreLogger.instance);
            }
        } catch (Exception e) {
            throw new IllegalStateException(e);
        }
        pkgDirLiveData = new MutableLiveData<>();
        moshi = new Moshi.Builder().build();
        jsonAdapter = moshi.adapter(PackageDirectory.class).nonNull();
        exec = MoreExecutors.listeningDecorator(Executors.newSingleThreadExecutor());
	exec.submit(this::initCerts);
        initialLoad();
    }

    public static Repository get() {
        return instance;
    }

    @MainThread
    public static void initialize(Context context) {
        if (instance != null)
            throw new IllegalStateException();
        instance = new Repository(context);
    }

    public PackageDirectory getPackageDirectory() {
        return pkgDir;
    }

    public DataManager createDataManager() {
        DataManager dm = new DataManager();
        dm.setPackageDirectory(getPackageDirectory());
        return dm;
    }

    public LiveData<PackageDirectory> getPackageDirectoryLiveData() {
        return pkgDirLiveData;
    }

    private boolean parse(String str) throws IOException {
        if (str == null)
            return false;
        final PackageDirectory dir = jsonAdapter.fromJson(str);
        dir.nextUpdateTime = Instant.now().plusSeconds(dir.localTtl);
        dir.index();
        pkgDir = dir;
        pkgDirLiveData.postValue(dir);
        return true;
    }

    private String getPackageDirFromResources() throws IOException {
        try (InputStreamReader reader = new InputStreamReader(context.getResources().openRawResource(R.raw.packages), "utf-8")) {
            return CharStreams.toString(reader);
        }
    }

    private void initialLoad() {
        try {
            {
                String str = engine.getCachedPackageDir();
                if (str == null)
                    str = getPackageDirFromResources();
                parse(str);
                scheduleUpdates();
            }
        } catch (Exception e) {
            if (BuildConfig.DEBUG)
                Log.e(TAG, "Error on initial load", e);
        }
    }

    private boolean updateFromServer() {
        if (BuildConfig.DEBUG)
            Log.v(TAG, "Updating from server");
        try {
            String str = engine.getPackageDirFromServer();
            if (BuildConfig.DEBUG && str != null)
                Log.v(TAG, "Response:\n" + str);
            final PackageDirectory oldDir = getPackageDirectory();
            final boolean res = parse(str);
            if (res && oldDir != null && oldDir.ttl != getPackageDirectory().ttl)
                ContextCompat.getMainExecutor(context).execute(this::scheduleUpdates);
            return res;
        } catch (Exception e) {
            if (BuildConfig.DEBUG)
                Log.e(TAG, "Error on update from server", e);
        }
        return false;
    }

    private boolean doCheck() {
        final PackageDirectory dir = getPackageDirectory();
        if (dir != null && dir.nextUpdateTime.isAfter(Instant.now()))
            return true;
        return updateFromServer();
    }

    public ListenableFuture<Boolean> refresh() {
        return exec.submit(this::updateFromServer);
    }

    public ListenableFuture<Boolean> check() {
        return exec.submit(this::doCheck);
    }

    private void scheduleUpdates() {
        final PackageDirectory dir = getPackageDirectory();
        final SharedPreferences prefs = context.getSharedPreferences("dir", 0);
        final long newTtl = dir.ttl;
        final long oldTtl = prefs.getLong("ttl", 0);
        Constraints constraints = new Constraints.Builder()
                .setRequiredNetworkType(NetworkType.CONNECTED)
                .build();
        PeriodicWorkRequest.Builder requestBuilder = new PeriodicWorkRequest.Builder(PackageDirectoryWorker.class, newTtl, TimeUnit.SECONDS);
        requestBuilder.setConstraints(constraints);
        if (newTtl != oldTtl && dir.localTtl != 0)
            requestBuilder.setInitialDelay(dir.localTtl, TimeUnit.SECONDS);
        PeriodicWorkRequest request = requestBuilder.build();
        ExistingPeriodicWorkPolicy policy = newTtl == oldTtl ? ExistingPeriodicWorkPolicy.KEEP : ExistingPeriodicWorkPolicy.REPLACE;
        WorkManager.getInstance(context).enqueueUniquePeriodicWork("dir.packages.rhvoice.org", policy, request);
        prefs.edit().putLong("ttl", newTtl).commit();
    }

    private void initCerts() {
	final File file_path=new File(new File(PackageClient.getPath(context)), "cacert.pem");
	final File tmp_path=new File(file_path.getPath()+".tmp");
        try(InputStream is=context.getResources().openRawResource(R.raw.cacert))
            {
                try(FileOutputStream os=new FileOutputStream(tmp_path))
                    {
                        DataPack.copyBytes(is, os, null);
		    }
		    }
        catch(IOException e)
            {
		return;
            }
		tmp_path.renameTo(file_path);
	    }

    }

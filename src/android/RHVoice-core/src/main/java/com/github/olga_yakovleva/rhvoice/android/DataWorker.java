/* Copyright (C) 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import androidx.work.Worker;
import androidx.work.WorkerParameters;

import com.google.common.util.concurrent.Striped;

import java.util.concurrent.locks.Lock;

abstract class DataWorker extends Worker {
    protected static final String TAG = "RHVoice.DataWorker";
    private static final Striped<Lock> locks = Striped.lazyWeakLock(4 * Runtime.getRuntime().availableProcessors());
    private final DataManager dm = new DataManager();

    protected abstract Result doWork(DataPack p);

    protected DataWorker(Context context, WorkerParameters params) {
        super(context, params);
    }

    private DataPack getTarget() {
        androidx.work.Data d = getInputData();
        if (d == null)
            return null;
        String langId = d.getString("language_id");
        if (langId == null)
            return null;
        LanguagePack lang = dm.getLanguageById(langId);
        if (lang == null)
            return null;
        String voiceId = d.getString("voice_id");
        if (voiceId == null)
            return lang;
        return lang.findVoiceById(voiceId);
    }

    @Override
    public final Result doWork() {
        final PackageDirectory dir = Repository.get().getPackageDirectory();
        if (dir == null)
            return Result.retry();
        dm.setPackageDirectory(dir);
        if (BuildConfig.DEBUG)
            Log.v(TAG, "doWork: " + getClass().getName());
        DataPack p = getTarget();
        if (p == null) {
            if (BuildConfig.DEBUG)
                Log.e(TAG, "No target defined");
            return Result.failure();
        }
        if (BuildConfig.DEBUG)
            Log.v(TAG, "Target: " + p.getId());
        final String key = p.getType() + "." + p.getId();
        Lock lock = locks.get(key);
        if (!lock.tryLock())
            return Result.retry();
        try {
            Result res = doWork(p);
            if (BuildConfig.DEBUG)
                Log.v(TAG, "Result from " + getClass().getName() + ": " + res);
            return res;
        } finally {
            lock.unlock();
        }
    }
}

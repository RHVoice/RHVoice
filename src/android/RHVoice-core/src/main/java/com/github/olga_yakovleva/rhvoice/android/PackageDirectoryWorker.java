/* Copyright (C) 2021  Olga Yakovleva <olga@rhvoice.org> */

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

import android.util.Log;
import android.content.Context;

import androidx.work.ListenableWorker;
import androidx.work.WorkerParameters;

import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.ListenableFuture;
import com.google.common.util.concurrent.MoreExecutors;

public final class PackageDirectoryWorker extends ListenableWorker {
    private static final String TAG = "RHVoicePackageDirectoryWorker";

    public PackageDirectoryWorker(Context context, WorkerParameters params) {
        super(context, params);
    }

    public ListenableFuture<ListenableWorker.Result> startWork() {
        if (BuildConfig.DEBUG)
            Log.v(TAG, "startWork()");
        if (!Repository.get().getPackageDirectoryLiveData().hasActiveObservers()) {
            if (BuildConfig.DEBUG)
                Log.v(TAG, "Probably idle, don't check now");
            return Futures.immediateFuture(Result.success());
        }
        ListenableFuture<Boolean> ok = Repository.get().refresh();
        return Futures.transform(ok, r -> r ? Result.success() : Result.retry(), MoreExecutors.directExecutor());
    }
}

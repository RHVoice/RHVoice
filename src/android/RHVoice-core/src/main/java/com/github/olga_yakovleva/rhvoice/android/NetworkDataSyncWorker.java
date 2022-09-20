/* Copyright (C) 2019, 2021  Olga Yakovleva <olga@rhvoice.org> */

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

import androidx.work.WorkerParameters;

public class NetworkDataSyncWorker extends DataSyncWorker {
    @Override
    public boolean isConnected() {
        return true;
    }

    public NetworkDataSyncWorker(Context context, WorkerParameters params) {
        super(context, params);
    }

    @Override
    protected Result doWork(DataPack p) {
        boolean done = doSync(p);
        if (BuildConfig.DEBUG)
            Log.v(TAG, "Network download of " + p.getId() + " finished with result " + done);
        return done ? Result.success() : Result.retry();
    }
}

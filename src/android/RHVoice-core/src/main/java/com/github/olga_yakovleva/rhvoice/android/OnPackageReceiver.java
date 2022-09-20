/* Copyright (C) 2016, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public final class OnPackageReceiver extends BroadcastReceiver {
    private static final String TAG = "RHVoiceOnPackageReceiver";

    @Override
    public void onReceive(Context context, Intent intent) {
        int uid = context.getApplicationInfo().uid;
        if (intent.getIntExtra(Intent.EXTRA_UID, uid) != uid)
            return;
        String packageName = intent.getData().getSchemeSpecificPart();
        if (BuildConfig.DEBUG)
            Log.i(TAG, "Package " + packageName + " has been installed/updated/removed");
        Repository.get().createDataManager().scheduleSync(context, true);
    }
}

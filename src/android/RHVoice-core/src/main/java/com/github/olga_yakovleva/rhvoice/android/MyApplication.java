/* Copyright (C) 2018, 2021  Olga Yakovleva <olga@rhvoice.org> */

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

import androidx.multidex.MultiDexApplication;
import androidx.work.Configuration;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Build;
import android.util.Log;

import com.google.android.material.color.DynamicColors;

import java.security.Provider;
import java.security.Security;

import javax.net.ssl.HttpsURLConnection;
import javax.net.ssl.SSLContext;

import org.conscrypt.Conscrypt;

public final class MyApplication extends MultiDexApplication implements Configuration.Provider {
    private static final String TAG = "RHVoice.MyApplication";
    private BroadcastReceiver userUnlockedReceiver;

    @Override
    public Configuration getWorkManagerConfiguration() {
        return new Configuration.Builder().build();
    }

    private void registerUserUnlockedReceiver() {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.N || !DirectBoot.isInDirectBoot(this))
            return;
        userUnlockedReceiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                if (Intent.ACTION_USER_UNLOCKED.equals(intent.getAction()))
                    onUserUnlocked();
            }
        };
        registerReceiver(userUnlockedReceiver, new IntentFilter(Intent.ACTION_USER_UNLOCKED));
    }

    private void unregisterUserUnlockedReceiver() {
        if (userUnlockedReceiver == null)
            return;
        unregisterReceiver(userUnlockedReceiver);
        userUnlockedReceiver = null;
    }

    private void onUserUnlocked() {
        unregisterUserUnlockedReceiver();
        DirectBoot.migrate(this, () -> Repository.get().onUserUnlocked());
    }

    @Override
    public void onCreate() {
        super.onCreate();
        DynamicColors.applyToActivitiesIfAvailable(this);
        try {
            Provider provider = Conscrypt.newProvider();
            Security.insertProviderAt(provider, 1);
            SSLContext context = SSLContext.getInstance("TLS", provider);
            context.init(null, null, null);
            HttpsURLConnection.setDefaultSSLSocketFactory(context.getSocketFactory());
            if (BuildConfig.DEBUG)
                Log.d(TAG, "Replaced default ssl socket factory");
        } catch (Exception e) {
            if (BuildConfig.DEBUG)
                Log.e(TAG, "Error", e);
        }
        boolean wasInDirectBoot = DirectBoot.isInDirectBoot(this);
        DirectBoot.migrate(this);
        Repository.initialize(this);
        if (wasInDirectBoot) {
            if (DirectBoot.isInDirectBoot(this))
                registerUserUnlockedReceiver();
            else
                onUserUnlocked();
        }
    }
}

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

import android.util.Log;

import com.github.olga_yakovleva.rhvoice.LogLevel;
import com.github.olga_yakovleva.rhvoice.Logger;

public final class CoreLogger implements Logger {
    private static final String BASE_TAG = "RHVoiceCore";
    public static final CoreLogger instance = new CoreLogger();

    private CoreLogger() {
    }

    @Override
    public void log(String subtag, LogLevel level, String message) {
        if (!BuildConfig.DEBUG)
            return;
        final String tag = BASE_TAG + "/" + subtag;
        switch (level) {
            case ERROR:
                Log.e(tag, message);
                break;
            case WARNING:
                Log.w(tag, message);
                break;
            case INFO:
                Log.i(tag, message);
                break;
            case DEBUG:
                Log.d(tag, message);
                break;
            default:
                Log.v(tag, message);
                break;
        }
    }
}

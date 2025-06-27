/* Copyright (C) 2025  Darko Milosevic <daremc86@gmail.com> */

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

import android.content.Context;

import java.io.File;

final class PackageClient {
    private static Context storageContext = MyApplication.getStorageContext();
    public static final String getPath(Context context) {
        context = storageContext;
        File dir = context.getDir("packages", 0);
        if (!dir.isDirectory()) {
            dir.mkdirs();
        }
        return dir.getAbsolutePath();
    }
}

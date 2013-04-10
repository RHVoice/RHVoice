/* Copyright (C) 2013  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

package com.github.olga_yakovleva.rhvoice.android;

import java.io.File;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.SharedPreferences;
import android.util.Log;

public final class TTSData
{
    private static String TAG="RHVoice";

    private static int getVersion(Context context)
    {
        int version=0;
        try
            {
                version=context.getPackageManager().getPackageInfo(context.getPackageName(),0).versionCode;
            }
        catch(PackageManager.NameNotFoundException e)
            {
                Log.e(TAG,"Our own package not found?",e);
            }
        return version;
    }

    private static SharedPreferences getInfo(Context context)
    {
        return context.getSharedPreferences("data-info",0);
    }

    public static File getDir(Context context)
    {
        return context.getDir("data",0);
    }

    public static File getLangDir(Context context)
    {
        return new File(getDir(context),"languages");
    }

    public static File getVoxDir(Context context)
    {
        return new File(getDir(context),"voices");
    }

    public static boolean checkVersion(Context context)
    {
        int version=getVersion(context);
        if(version==0)
            return false;
        else
            return (getInfo(context).getInt("version",0)==version);
    }

    public static void resetVersion(Context context)
    {
        getInfo(context).edit().putInt("version",0).commit();
    }

    public static void updateVersion(Context context)
    {
        getInfo(context).edit().putInt("version",getVersion(context)).commit();
    }

    public static boolean isInstalled(Context context)
    {
        if(checkVersion(context))
            if(getLangDir(context).isDirectory())
                if(getVoxDir(context).isDirectory())
                    return true;
        return false;
    }
}

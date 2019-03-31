/* Copyright (C) 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

package com.github.olga_yakovleva.rhvoice.android;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;
import java.util.Set;
import android.util.Log;
import java.util.HashSet;

final class PrefsMigrator
{
    private static final String TAG="RHVoicePrefsMigrator";
    private static final int VERSION=2;
    private static final String VERSION_KEY="preferences_compat_version";
    private final Context context;
    private final SharedPreferences prefs;
    private final Set<String> keys=new HashSet<String>();
    private int oldVersion;
    private SharedPreferences.Editor editor;

    private PrefsMigrator(Context context)
    {
        this.context=context;
        prefs=PreferenceManager.getDefaultSharedPreferences(context);
}

    private boolean needsMigration()
    {
        oldVersion=prefs.getInt(VERSION_KEY,1);
        if(oldVersion>=VERSION)
            {
                if(BuildConfig.DEBUG)
                    Log.d(TAG,"No migration needed from version "+oldVersion+" to version "+VERSION);
                return false;
}
        else
            {
                if(BuildConfig.DEBUG)
                    Log.d(TAG,"Migrating from version "+oldVersion+" to version "+VERSION);
                return true;
}
}

    private void migrateProsodyPrefs1()
    {
        for(String oldKey: keys)
            {
                if(oldKey.endsWith(".rate")||oldKey.endsWith(".volume"))
                    {
                        String s=prefs.getString(oldKey,"1");
                        int value=100;
                        try
                            {
                                value=Integer.parseInt(s);
}
                        catch(NumberFormatException e)
                            {
                                if(BuildConfig.DEBUG)
                                    Log.w(TAG,"Failed to parse int: "+s);
}
                        String newKey=oldKey+"2";
                        if(BuildConfig.DEBUG)
                            Log.d(TAG,"Migrating from "+oldKey+"="+value+" to "+newKey+"="+value);
                        editor.putInt(newKey,value);
                        editor.remove(oldKey);
}
}
}

    private void doMigrate()
    {
        if(!needsMigration())
            return;
        keys.addAll(prefs.getAll().keySet());
        editor=prefs.edit();
        switch(oldVersion)
            {
            case 1:
                migrateProsodyPrefs1();
            default:
                break;
}
        editor.putInt(VERSION_KEY,VERSION);
        editor.apply();
}

    public static void migrate(Context context)
    {
        PrefsMigrator m=new PrefsMigrator(context);
        m.doMigrate();
}
}

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
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */


package com.github.olga_yakovleva.rhvoice.android;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.Context;
import android.util.Log;

public final class Config
{
    private static final String TAG="RHVoiceConfig";
    private static final String CONFIG_FILE_NAME="RHVoice.conf";

    private static void unpackFile(Context context,String inPath,File outFile) throws IOException
    {
        InputStream inStream=null;
        OutputStream outStream=null;
        try
            {
                inStream=context.getAssets().open(inPath);
                outStream=new FileOutputStream(outFile);
                byte[] buf=new byte[8092];
                int count=0;
                while((count=inStream.read(buf))>0)
                    outStream.write(buf,0,count);
            }
        finally
            {
                if(outStream!=null)
                    outStream.close();
                if(inStream!=null)
                    inStream.close();
            }
    }

    private static void unpackConfigFile(Context context,File configDir) throws IOException
    {
        File configFile=new File(configDir,CONFIG_FILE_NAME);
        if(configFile.exists())
            return;
        unpackFile(context,CONFIG_FILE_NAME,configFile);
    }

    public static File getDir(Context context)
    {
        if(BuildConfig.DEBUG)
            Log.d(TAG,"Requesting path to the private external storage directory");
        File dir=context.getExternalFilesDir(null);
        if(dir==null)
            {
                if(BuildConfig.DEBUG)
                    Log.d(TAG,"The private external storage directory does not exist");
                return context.getDir("config",0);
            }
        if(BuildConfig.DEBUG)
            Log.d(TAG,"The path to the private external storage directory is "+dir.getAbsolutePath());
        try
            {
                unpackConfigFile(context,dir);
            }
        catch(IOException e)
            {
            }
        return dir;
    }
}

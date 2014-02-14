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

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.Queue;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.os.Bundle;
import android.os.ResultReceiver;
import android.speech.tts.TextToSpeech;
import android.util.Log;
import com.github.olga_yakovleva.rhvoice.RHVoiceException;
import com.github.olga_yakovleva.rhvoice.TTSEngine;

public final class DataService extends IntentService 
{
    static private final String TAG="RHVoiceDataService";
    private boolean hasVoices=false;
    private ArrayList<String> upToDatePaths=new ArrayList<String>();
    private Queue<PackageInfo> newPackages=new LinkedList<PackageInfo>();
    private Queue<String> obsoletePaths=new LinkedList<String>();

    static private FutureDataResult sendRequest(Context context,FutureDataResult.Receiver receiver,boolean prepare)
    {
        FutureDataResult result=new FutureDataResult();
        result.setReceiver(receiver);
        Intent intent=new Intent(context,DataService.class);
        intent.putExtra("prepare",prepare);
        intent.putExtra("receiver",result);
        context.startService(intent);
        return result;
    }

    public static FutureDataResult checkData(Context context,FutureDataResult.Receiver receiver)
    {
        return sendRequest(context,receiver,false);
    }

    public static FutureDataResult prepareData(Context context,FutureDataResult.Receiver receiver)
    {
        return sendRequest(context,receiver,true);
    }

    public DataService()
    {
        super("DataService");
    }

    @Override
    protected void onHandleIntent(final Intent intent)
    {
        boolean prepare=intent.getBooleanExtra("prepare",false);
        ResultReceiver receiver=intent.getParcelableExtra("receiver");
        if(BuildConfig.DEBUG)
            {
                if(prepare)
                    Log.i(TAG,"Received request to prepare data");
                else
                    Log.i(TAG,"Received request to check data");
            }
        checkFiles();
        checkVoices();
        if(!hasVoices&&prepare)
            {
                prepareFiles();
                checkVoices();
                if(hasVoices)
                    {
                        Intent broadcast=new Intent(TextToSpeech.Engine.ACTION_TTS_DATA_INSTALLED);
                        sendBroadcast(broadcast);
                    }
            }
        if(hasVoices)
            {
                if(BuildConfig.DEBUG)
                    Log.i(TAG,"Data is installed");
                Bundle bundle=new Bundle();
                bundle.putStringArray("paths",upToDatePaths.toArray(new String[upToDatePaths.size()]));
                receiver.send(0,bundle);
            }
        else
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Data is not ready");
                receiver.send(-1,Bundle.EMPTY);
            }
    }

    private void checkFiles()
    {
        hasVoices=false;
        upToDatePaths.clear();
        obsoletePaths.clear();
        newPackages.clear();
        PackageManager pm=getPackageManager();
        String myPkg=getPackageName();
        int uid=getApplicationInfo().uid;
        String[] pkgs=pm.getPackagesForUid(uid);
        for (String pkg:pkgs)
            {
                if (pkg.equals(myPkg))
                    continue;
                try
                    {
                        PackageInfo pi=pm.getPackageInfo(pkg, 0);
                        File dir=getDataDir(pi);
                        if(dir.exists())
                            {
                                if(BuildConfig.DEBUG)
                                    Log.v(TAG,"Data from package "+pi.packageName+" version "+pi.versionCode+" is already installed");
                                upToDatePaths.add(dir.getAbsolutePath());
                            }
                        else
                            {
                                if(BuildConfig.DEBUG)
                                    Log.v(TAG,"Data package "+pi.packageName+" version "+pi.versionCode+" is new");
                                newPackages.add(pi);
                            }
                    }
                catch (PackageManager.NameNotFoundException e)
                    {
                        if(BuildConfig.DEBUG)
                            Log.e(TAG,"Data package disappeared", e);
                    }
            }
        File[] dirs=getDataDir().listFiles();
        for(File dir:dirs)
            {
                if(!dir.isDirectory())
                    continue;
                String absPath=dir.getAbsolutePath();
                if(!upToDatePaths.contains(absPath))
                    {
                        if(BuildConfig.DEBUG)
                            Log.v(TAG,"Data directory "+dir.getName()+" is obsolete");
                    obsoletePaths.add(absPath);
                    }
            }
    }

    private void prepareFiles()
    {
        removeObsoleteData();
        installNewData();
    }

    private void checkVoices()
    {
        if(upToDatePaths.isEmpty())
            return;
        if(!newPackages.isEmpty())
            return;
        if(!obsoletePaths.isEmpty())
            return;
        try
            {
                TTSEngine engine=new TTSEngine("",Config.getDir(this).getAbsolutePath(),upToDatePaths.toArray(new String[upToDatePaths.size()]));
                hasVoices=!(engine.getVoices().isEmpty());
                engine.shutdown();
            }
        catch(RHVoiceException e)
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Engine initialization failed",e);
            }
    }

    private File getDataDir()
    {
        return getDir("data",0);
    }

    private File getDataDir(PackageInfo pi)
    {
        return new File(getDataDir(),pi.packageName+"."+pi.versionCode);
    }

    private File getTempDir()
    {
        return getDir("tmp",0);
    }

    private boolean delete(File obj)
    {
        if(!obj.exists())
            return true;
        if(obj.isDirectory())
            {
                File[] children=obj.listFiles();
                for(File child:children)
                    {
                        if(!delete(child))
                            return false;
                    }
            }
        boolean result=obj.delete();
        if(BuildConfig.DEBUG)
            if(!result)
                Log.e(TAG,"Failed to delete "+obj.getAbsolutePath());
        return result;
    }

    private boolean clean(File dir)
    {
        if(dir.isDirectory())
            {
                File[] children=dir.listFiles();
                for(File child:children)
                    {
                        if(!delete(child))
                            return false;
                    }
                return true;
            }
        else
            return false;
    }

    private Boolean mkdir(File dir)
    {
        if(dir.isDirectory())
            return true;
        else
            {
                boolean result=dir.mkdirs();
                if(BuildConfig.DEBUG)
                    if(!result)
                        Log.e(TAG,"Failed to create directory "+dir.getAbsolutePath());
                return result;
            }
    }

    private boolean close(Closeable obj)
    {
        if(obj!=null)
            {
                try
                    {
                        obj.close();
                        return true;
                    }
                catch(IOException e)
                    {
                        return false;
                    }
                finally
                    {
                        obj=null;
                    }
            }
        else
            return true;
    }

    private boolean removeObsoleteDir(File dir)
    {
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Removing obsolete directory "+dir.getAbsolutePath());
        File tempDir=getTempDir();
        if(!delete(tempDir))
            return false;
        if(!dir.renameTo(tempDir))
            return false;
                delete(tempDir);
        return true;
    }

    private boolean installPackageData(PackageInfo pi)
    {
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Installing data from "+pi.packageName+" version "+pi.versionCode);
        Context context;
        try
            {
                context=createPackageContext(pi.packageName,0);
            }
        catch(Exception e)
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Failed to get a context for a data package "+pi.packageName,e);
                return false;
            }
        Resources resources=context.getResources();
        int id=resources.getIdentifier("data","raw",pi.packageName);
        if(id==0)
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Failed to get the data resource for "+pi.packageName);
                return false;
            }
        File tempDir=getTempDir();
        if(!mkdir(tempDir))
            return false;
        if(!clean(tempDir))
            return false;
        ZipInputStream inStream=null;
        OutputStream outStream=null;
        byte[] buffer=new byte[4096];
        int numBytes=0;
        try
            {
                inStream=new ZipInputStream(new BufferedInputStream(resources.openRawResource(id)));
                ZipEntry entry;
                while((entry=inStream.getNextEntry())!=null)
                    {
                        if(BuildConfig.DEBUG)
                            Log.v(TAG,"Extracting "+entry.getName()+" from "+pi.packageName);
                        File outObj=new File(tempDir,entry.getName());
                        if(entry.isDirectory())
                            {
                                if(!mkdir(outObj))
                                    return false;
                            }
                        else
                            {
                                if(!mkdir(outObj.getParentFile()))
                                    return false;
                                outStream=new BufferedOutputStream(new FileOutputStream(outObj));
                                while((numBytes=inStream.read(buffer))!=-1)
                                    {
                                        outStream.write(buffer,0,numBytes);
                                    }
                                outStream.close();
                            }
                        inStream.closeEntry();
                    }
                if(!tempDir.renameTo(getDataDir(pi)))
                    return false;
                return true;
            }
        catch(Exception e)
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Error while installing data from "+pi.packageName,e);
            }
        finally
            {
                close(outStream);
                close(inStream);
                delete(tempDir);
            }
        return false;
    }

    private boolean removeObsoleteData()
    {
        String path;
        while((path=obsoletePaths.peek())!=null)
            {
                if(!removeObsoleteDir(new File(path)))
                    return false;
                obsoletePaths.poll();
            }
        return true;
    }

    private boolean installNewData()
    {
        if(newPackages.isEmpty())
            return true;
        PackageInfo pkg;
        while((pkg=newPackages.peek())!=null)
            {
                if(!installPackageData(pkg))
                    return false;
                newPackages.poll();
                upToDatePaths.add(getDataDir(pkg).getAbsolutePath());
            }
        return true;
    }
}

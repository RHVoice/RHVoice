/* Copyright (C) 2017, 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.preference.PreferenceManager;
import android.speech.tts.TextToSpeech;
import android.util.Log;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.Properties;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public abstract class DataPack
{
    private static final String TAG="RHVoiceDataPack";
    protected final String name;
    protected final int format;
    protected final int revision;
    protected String altLink;
    protected String tempLink;

    protected DataPack(String name,int format,int revision)
    {
        this.name=name;
        this.format=format;
        this.revision=revision;
}

    protected DataPack(String name,int format,int revision,String altLink,String tempLink)
    {
        this(name,format,revision);
        this.altLink=altLink;
        this.tempLink=tempLink;
}

    public abstract String getType();

    public final String getName()
    {
        return name;
}

    public abstract String getDisplayName();

    protected abstract String getBaseFileName();

    public final String getVersionString()
    {
        return String.format("%s.%s",format,revision);
}

    protected final int getVersionCode(int format,int revision)
    {
        return (1000*format+10*revision);
}

    public final int getVersionCode()
    {
        return getVersionCode(format,revision);
}

    protected final int getVersionCode(File dir) throws IOException
    {
        File file=new File(dir,getType()+".info");
        InputStream str=new BufferedInputStream(new FileInputStream(file));
        try
            {
                Properties props=new Properties();
                props.load(str);
                String strFormat=props.getProperty("format");
                if(strFormat==null)
                    return 0;
                String strRevision=props.getProperty("revision");
                if(strRevision==null)
                    return 0;
                try
                    {
                        return getVersionCode(Integer.parseInt(strFormat),Integer.parseInt(strRevision));
}
                catch(NumberFormatException e)
                    {
                        return 0;
}
}
        finally
            {
                close(str);
}
}

    public final String getPackageName()
    {
        return String.format("com.github.olga_yakovleva.rhvoice.android.%s.%s",getType(),getName().toLowerCase());
}

    public final PackageInfo getPackageInfo(Context context)
    {
        PackageManager pm=context.getPackageManager();
        try
            {
                PackageInfo pi=pm.getPackageInfo(getPackageName(),0);
                if(context.getApplicationInfo().uid!=pi.applicationInfo.uid)
                    return null;
                return pi;
}
        catch(PackageManager.NameNotFoundException e)
            {
                return null;
}
}

    public final String getLink()
    {
        if(tempLink!=null)
            return tempLink;
        if(altLink!=null)
            return altLink;
        return String.format("https://dl.bintray.com/olga-yakovleva/Data/%s-v%s.zip",getBaseFileName(),getVersionString());
}

    private String getDownloadFileName()
    {
        return String.format("%s-v%s.zip",getBaseFileName(),getVersionString());
}

    protected final File getDataDir(Context context)
    {
        return context.getDir("data",0).getAbsoluteFile();
    }

    protected final File getTempDir(Context context)
    {
        return context.getDir("tmp",0);
    }

    private File getDownloadsDir(Context context)
    {
        return context.getDir("downloads-"+getType()+"-"+getName().toLowerCase(),0);
    }

    private File getDownloadFile(Context context)
    {
        return new File(getDownloadsDir(context),getDownloadFileName());
}

    private File getTempDownloadFile(Context context)
    {
        return new File(getDownloadsDir(context),getDownloadFileName()+".tmp");
}

    protected final boolean delete(File obj)
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
        return result;
    }

    protected final boolean clean(File dir)
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

    protected final Boolean mkdir(File dir)
    {
        if(dir.isDirectory())
            return true;
        else
            {
                boolean result=dir.mkdirs();
                return result;
            }
    }

    protected final boolean close(Closeable obj)
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

    public final File getInstallationDir(Context context,int versionCode)
    {
        File dataDir=getDataDir(context);
        return new File(dataDir,String.format("%s.%d",getPackageName(),versionCode));
}

    public final boolean isUpToDate(Context context)
    {
        return getInstallationDir(context,getVersionCode()).exists();
}

    protected final void copyBytes(InputStream in,OutputStream out) throws IOException
    {
        byte[] buf=new byte[8092];
        int numBytes;
        while((numBytes=in.read(buf))!=-1)
            {
                if(numBytes>0)
                    out.write(buf,0,numBytes);
            }
        out.flush();
    }

    private InputStream openResource(Context context) throws IOException
    {
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Trying to open resources in a package");
        PackageInfo pi=getPackageInfo(context);
        if(pi==null)
            {
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"No package found");
                throw new IOException("No package for "+getName());
            }
        if(pi.versionCode!=getVersionCode())
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"Package version mismatch");
                throw new IOException("Version mismatch for "+getName());
            }
        Context pkgContext=null;
try
    {
        pkgContext=context.createPackageContext(pi.packageName,0);
    }
catch(PackageManager.NameNotFoundException e)
    {
        if(BuildConfig.DEBUG)
            Log.e(TAG,"Failed to get package context");
        throw new IOException(e);
}
        Resources resources=pkgContext.getResources();
        int id=resources.getIdentifier("data","raw",pi.packageName);
        if(id==0)
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"Resource not found");
                throw new IOException("Resource not found");
            }
        return resources.openRawResource(id);
}

    private InputStream openLink(Context context,IDataSyncCallback callback) throws IOException
    {
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Trying to open the link");
        File file=getDownloadFile(context);
        if(!file.exists())
            downloadFile(context,callback);
        return new FileInputStream(file);
}

    private void downloadFile(Context context,IDataSyncCallback callback) throws IOException
    {
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Trying to download the file");
        if(!callback.isConnected())
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"No suitable internet connection");
                callback.onNetworkError();
                throw new IOException("No connection");
            }
        if(!mkdir(getDownloadsDir(context)))
            throw new IOException("Unable to create downloads directory");
        File tempFile=getTempDownloadFile(context);
        InputStream istr=null;
        OutputStream ostr=null;
        HttpURLConnection con=null;
        try
            {
                long start=0;
                if(tempFile.exists())
                    start=tempFile.length();
                URL url=new URL(getLink());
                con=(HttpURLConnection)url.openConnection();
                con.setRequestProperty("Accept-Encoding","identity");
                if(start>0)
{
    if(BuildConfig.DEBUG)
        Log.v(TAG,"We already have "+start+" bytes, trying to download only the rest");
    con.setRequestProperty("Range","bytes="+start+"-");
}
                con.connect();
                int status=con.getResponseCode();
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"Http status: "+status);
                if(status!=HttpURLConnection.HTTP_OK&&!(start>0&&status==HttpURLConnection.HTTP_PARTIAL))
                    throw new IOException("Http Status: "+status);
                istr=new BufferedInputStream(con.getInputStream());
                ostr=new BufferedOutputStream(new FileOutputStream(tempFile,status==HttpURLConnection.HTTP_PARTIAL));
                copyBytes(istr,ostr);
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"File downloaded");
}
        catch(IOException e)
            {
                callback.onNetworkError();
                throw e;
}
        finally
            {
                close(ostr);
                close(istr);
                if(con!=null)
                    con.disconnect();
}
        if(!tempFile.renameTo(getDownloadFile(context)))
            {
                tempFile.delete();
                throw new IOException("Unable to rename the file");
            }
}

    private InputStream open(Context context,IDataSyncCallback callback) throws IOException
    {
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Opening");
        try
            {
                return openResource(context);
            }
        catch(Exception e)
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"Error",e);
}
        return openLink(context,callback);
    }

    protected abstract void notifyDownloadStart(IDataSyncCallback callback);
    protected abstract void notifyDownloadDone(IDataSyncCallback callback);
    protected abstract void notifyInstallation(IDataSyncCallback callback);
    protected abstract void notifyRemoval(IDataSyncCallback callback);

    public boolean install(Context context,IDataSyncCallback callback)
    {
        if(isUpToDate(context))
            return true;
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Installing "+getType()+" "+getName());
        File tempDir=getTempDir(context);
        if(!mkdir(tempDir))
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Failed to create temporary directory");
                return false;
            }
        if(!clean(tempDir))
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Failed to clean temporary directory");
                return false;
            }
        ZipInputStream inStream=null;
        OutputStream outStream=null;
        try
            {
                notifyDownloadStart(callback);
                inStream=new ZipInputStream(new BufferedInputStream(open(context,callback)));
                ZipEntry entry;
                while((entry=inStream.getNextEntry())!=null)
                    {
                        if(BuildConfig.DEBUG)
                            Log.v(TAG,"Extracting "+entry.getName()+" from "+getName());
                        File outObj=new File(tempDir,entry.getName());
                        if(entry.isDirectory())
                            {
                                if(!mkdir(outObj))
                                    {
                                        if(BuildConfig.DEBUG)
                                            Log.e(TAG,"Failed to create directory");
                                        return false;
                                    }
                            }
                        else
                            {
                                if(!mkdir(outObj.getParentFile()))
                                    {
                                        if(BuildConfig.DEBUG)
                                            Log.e(TAG,"Failed to create parent directory");
                                        return false;
                                    }
                                outStream=new BufferedOutputStream(new FileOutputStream(outObj));
                                copyBytes(inStream,outStream);
                                close(outStream);
                            }
                        inStream.closeEntry();
                    }
                notifyDownloadDone(callback);
                int versionCode=getVersionCode(tempDir);
                if(versionCode!=getVersionCode())
                    {
                        if(BuildConfig.DEBUG)
                            Log.w(TAG,"Version mismatch");
                        return false;
                    }
                if(!tempDir.renameTo(getInstallationDir(context,versionCode)))
                    {
                        if(BuildConfig.DEBUG)
                            Log.e(TAG,"Failed to rename temporary directory");
                        return false;
                    }
                getPrefs(context).edit().putInt(getVersionKey(),versionCode).apply();
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"Installed "+getType()+" "+getName());
                cleanup(context,versionCode);
                notifyInstallation(callback);
                context.sendBroadcast(new Intent(TextToSpeech.Engine.ACTION_TTS_DATA_INSTALLED));
                return true;
            }
        catch(IOException e)
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Error while installing data from "+getName(),e);
                return false;
            }
        finally
            {
                close(outStream);
                close(inStream);
                delete(tempDir);
            }
    }

    private void cleanup(Context context,int versionCode)
    {
        delete(getDownloadsDir(context));
        String pkgName=getPackageName();
        File instDir=(versionCode>0)?getInstallationDir(context,versionCode):null;
        File dataDir=getDataDir(context);
        File[] dirs=dataDir.listFiles();
        if(dirs==null)
            return;
        for(File dir: dirs)
            {
                if(!dir.getName().startsWith(pkgName))
                    continue;
                if(instDir!=null&&instDir.getPath().equals(dir.getPath()))
                    continue;
                delete(dir);
}
}

    protected static final SharedPreferences getPrefs(Context context)
    {
        return PreferenceManager.getDefaultSharedPreferences(context);
}

    protected final String getVersionKey()
    {
        return String.format("%s.%s.version",getType(),getName().toLowerCase());
}

    public final String getPath(Context context)
    {
        File dir=getInstallationDir(context,getVersionCode());
        if(dir.exists())
            return dir.getPath();
        int versionCode=getPrefs(context).getInt(getVersionKey(),0);
        if(versionCode>0&&versionCode!=getVersionCode())
            {
                dir=getInstallationDir(context,versionCode);
                if(dir.exists())
                    return dir.getPath();
}
        PackageInfo pi=getPackageInfo(context);
        if(pi==null)
            return null;
        if(pi.versionCode==versionCode||pi.versionCode==getVersionCode())
            return null;
        dir=getInstallationDir(context,pi.versionCode);
        if(dir.exists())
            return dir.getPath();
        return null;
}

    public final boolean isInstalled(Context context)
    {
        return (getPath(context)!=null);
}

    @Override
    public String toString()
    {
        return getDisplayName();
}

    public void uninstall(Context context,IDataSyncCallback callback)
    {
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Removing "+getType()+" "+getName());
        cleanup(context,0);
        getPrefs(context).edit().remove(getVersionKey()).apply();
        notifyRemoval(callback);
}

    public abstract boolean getEnabled(Context context);

    public boolean sync(Context context,IDataSyncCallback callback)
    {
        if(callback.isStopped())
            return true;
        if(getEnabled(context))
            {
                if(!isUpToDate(context))
                    return install(context,callback);
                else
                    return true;
}
        else
            {
                if(isInstalled(context))
                    uninstall(context,callback);
                return true;
}
}

    public long getSyncFlags(Context context)
    {
        if(!getEnabled(context))
            {
                if(isInstalled(context))
                    return SyncFlags.LOCAL;
                else
                    return 0;
}
        if(isUpToDate(context))
            return 0;
        if(canBeInstalledFromPackage(context))
            return SyncFlags.LOCAL;
        return SyncFlags.NETWORK;
}

    public boolean canBeInstalledFromPackage(Context context)
    {
        PackageInfo pi=getPackageInfo(context);
        if(pi!=null&&pi.versionCode==getVersionCode())
            return true;
        else
            return false;
}
}

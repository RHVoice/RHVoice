/* Copyright (C) 2017, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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
import androidx.work.Constraints;
import androidx.work.ExistingWorkPolicy;
import androidx.work.NetworkType;
import androidx.work.OneTimeWorkRequest;
import androidx.work.WorkContinuation;
import androidx.work.WorkManager;
import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.Closeable;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
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
    private final String id;
    protected final byte[] checksum;
    protected final String workName;

    private String createWorkName()
    {
        StringBuilder b=new StringBuilder();
        b.append(Data.WORK_PREFIX_STRING).append('.');
        b.append(getType()).append('.');
        b.append(getId());
        return b.toString();
}

    protected DataPack(String id,String name,int format,int revision,byte[] checksum)
    {
        this.id=id;
        this.name=name;
        this.format=format;
        this.revision=revision;
        this.checksum=checksum;
        this.workName=createWorkName();
}

    protected DataPack(String name,int format,int revision,byte[] checksum)
    {
        this(null,name,format,revision,checksum);
}

    protected DataPack(String id,String name,int format,int revision,byte[] checksum,String altLink,String tempLink)
    {
        this(id,name,format,revision,checksum);
        this.altLink=altLink;
        this.tempLink=tempLink;
}

    protected DataPack(String name,int format,int revision,byte[] checksum,String altLink,String tempLink)
    {
        this(null,name,format,revision,checksum,altLink,tempLink);
}

    public abstract String getType();

    public final String getName()
    {
        return name;
}

    public final String getId()
    {
        if(id!=null)
            return id;
        else
            return name.toLowerCase().replace("-","_");
}

    public final String getWorkName()
    {
        return workName;
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
        InputStreamReader reader=new InputStreamReader(new BufferedInputStream(new FileInputStream(file)),"utf-8");
        try
            {
                Properties props=new Properties();
                props.load(reader);
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
                close(reader);
}
}

    public final String getPackageName()
    {
        return String.format("com.github.olga_yakovleva.rhvoice.android.%s.%s",getType(),getId());
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
        return String.format("%s/%s-v%s.zip", Data.REPO_URL, getBaseFileName(),getVersionString());
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
        return context.getDir("tmp-"+getType()+"-"+getId(),0);
    }

    private File getDownloadsDir(Context context)
    {
        return context.getDir("downloads-"+getType()+"-"+getId(),0);
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

    protected final boolean safeDelete(Context context,File file)
    {
        if(!file.exists())
            return true;
        if(!file.isDirectory())
            return delete(file);
        File tempDir=getTempDir(context);
        if(!delete(tempDir))
            return false;
        if(!file.renameTo(tempDir))
            return false;
        return delete(tempDir);
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

    protected final void copyBytes(InputStream in,OutputStream out,IDataSyncCallback callback) throws IOException
    {
        byte[] buf=new byte[8092];
        int numBytes;
        while((numBytes=in.read(buf))!=-1)
            {
                if(numBytes>0)
                    out.write(buf,0,numBytes);
                if(callback!=null&&callback.isTaskStopped())
                    {
                        out.flush();
                        if(BuildConfig.DEBUG)
                            Log.v(TAG,"The work was cancelled before we have downloaded the file");
                        throw new IOException("Cancelled");
}
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

    private void verifyFile(File file) throws IOException
    {
        if(checksum==null)
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"Checksum unknown");
                return;
            }
        InputStream str=null;
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Verifying file integrity: "+file.getName());
        MessageDigest md=null;
        try
            {
                md=MessageDigest.getInstance("MD5");
            }
        catch(NoSuchAlgorithmException e)
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Failed to get MessageDigest instance",e);
                return;
}
try
    {
        str=new BufferedInputStream(new FileInputStream(file));
        byte[] buf=new byte[8092];
        int numBytes;
        while((numBytes=str.read(buf))!=-1)
            {
                if(numBytes>0)
                    md.update(buf,0,numBytes);
            }
        close(str);
        str=null;
        byte[] value=md.digest();
        if(MessageDigest.isEqual(checksum,value))
            {
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"File integrity verified: "+file.getName());
                return;
}
        if(BuildConfig.DEBUG)
            Log.e(TAG,"File integrity verification failed: "+file.getName());
        file.delete();
        throw new IOException("Bad file");
}
finally
    {
        close(str);
}
}

    private InputStream openLink(Context context,IDataSyncCallback callback) throws IOException
    {
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Trying to open the link");
        File file=getDownloadFile(context);
        if(file.exists())
            verifyFile(file);
        else
            downloadFile(context,callback);
        return new FileInputStream(file);
}

    private void downloadFile(Context context,IDataSyncCallback callback) throws IOException
    {
        checkIfStopped(callback);
        String link=getLink();
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Trying to download the file from "+link);
        if(!callback.isConnected())
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"No suitable internet connection");
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
                URL url=new URL(link);
                con=(HttpURLConnection)url.openConnection();
                con.setConnectTimeout(60000);
                con.setReadTimeout(60000);
                con.setRequestProperty("Accept-Encoding","identity");
                if(start>0)
{
    if(BuildConfig.DEBUG)
        Log.v(TAG,"We already have "+start+" bytes, trying to download only the rest");
    con.setRequestProperty("Range","bytes="+start+"-");
}
                con.connect();
                checkIfStopped(callback);
                int status=con.getResponseCode();
                if(BuildConfig.DEBUG)
                    {
                        Log.v(TAG,"Http status: "+status);
                        Log.v(TAG,"Http headers:");
                        Map<String,List<String>> headers=con.getHeaderFields();
                        for(Map.Entry<String,List<String>> header: headers.entrySet())
                            {
                                StringBuilder b=new StringBuilder();
                                b.append(header.getKey());
                                boolean first=true;
                                for(String value: header.getValue())
                                    {
                                        if(first)
                                            {
                                                first=false;
                                                b.append(": ");
}
                                        else
                                            b.append(", ");
                                        b.append(value);
}
                                Log.v(TAG,"Header field "+b.toString());
}
                    }
                if(start>0&&status==416)
                    {
                        if(BuildConfig.DEBUG)
                            Log.w(TAG,"Received range not satisfiable");
                        tempFile.delete();
                        throw new IOException("Range not satisfiable");
}
                if(status!=HttpURLConnection.HTTP_OK&&!(start>0&&status==HttpURLConnection.HTTP_PARTIAL))
                    throw new IOException("Http Status: "+status);
                istr=con.getInputStream();
                checkIfStopped(callback);
                ostr=new BufferedOutputStream(new FileOutputStream(tempFile,status==HttpURLConnection.HTTP_PARTIAL));
                copyBytes(istr,ostr,callback);
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"File downloaded");
}
        finally
            {
                close(ostr);
                close(istr);
                if(con!=null)
                    con.disconnect();
}
        verifyFile(tempFile);
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

    private void checkIfStopped(IDataSyncCallback callback) throws IOException
    {
        if(!callback.isTaskStopped())
            return;
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Interrupting the work, because it has been cancelled");
        throw new IOException("Cancelled");
}

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
                        checkIfStopped(callback);
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
                                copyBytes(inStream,outStream,null);
                                close(outStream);
                                outStream=null;
                            }
                        inStream.closeEntry();
                    }
                close(inStream);
                inStream=null;
                notifyDownloadDone(callback);
                int versionCode=getVersionCode(tempDir);
                if(versionCode!=getVersionCode())
                    {
                        if(BuildConfig.DEBUG)
                            Log.w(TAG,"Version mismatch");
                        return false;
                    }
                if(!getEnabled(context))
                    {
                        if(BuildConfig.DEBUG)
                            Log.w(TAG,"Installation was canceled while downloading");
                        return false;
}
                if(!tempDir.renameTo(getInstallationDir(context,versionCode)))
                    {
                        if(BuildConfig.DEBUG)
                            Log.e(TAG,"Failed to rename temporary directory");
                        return false;
                    }
                getPrefs(context).edit().putInt(getVersionKey(),versionCode).commit();
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
                safeDelete(context,dir);
}
}

    protected static final SharedPreferences getPrefs(Context context)
    {
        return PreferenceManager.getDefaultSharedPreferences(context);
}

    protected final String getVersionKey()
    {
        return String.format("%s.%s.version",getType(),getId());
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
        getPrefs(context).edit().remove(getVersionKey()).commit();
        notifyRemoval(callback);
}

    public abstract boolean getEnabled(Context context);

    public boolean sync(Context context,IDataSyncCallback callback)
    {
        if(getEnabled(context))
            {
                if(!isUpToDate(context))
                    {
                    boolean installed=install(context,callback);
                    if(getEnabled(context))
                        return installed;
                    else
                        {
                            if(installed)
                                uninstall(context,callback);
                            else
                                cleanup(context,0);
                            return true;
                        }
                    }
                else
                    return true;
}
        else
            {
                if(isInstalled(context))
                    uninstall(context,callback);
                else
                    cleanup(context,0);
                return true;
}
}

    public long getSyncFlag(Context context)
    {
        return getSyncFlag(context,false);
}

    public long getSyncFlag(Context context,boolean checkPkg)
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
        if(checkPkg&&canBeInstalledFromPackage(context))
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

    @Override
    public boolean equals(Object other)
    {
        if(this==other)
            return true;
        if(other==null)
            return false;
        if(getClass()!=other.getClass())
            return false;
        DataPack pack=(DataPack)other;
        return (name.equalsIgnoreCase(pack.name)&&format==pack.format&&revision==pack.revision);
}

    public static NetworkType getNetworkTypeSetting(Context context)
    {
        boolean wifiOnly=getPrefs(context).getBoolean("wifi_only",false);
        return wifiOnly?NetworkType.UNMETERED:NetworkType.CONNECTED;
}

    protected androidx.work.Data.Builder setWorkInput(androidx.work.Data.Builder b)
    {
        b.putString(getType()+"_id",getId());
        return b;
}

    public final void scheduleSync(Context context,boolean replace)
    {
        long flag=getSyncFlag(context);
        if(flag==0)
            return;
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Scheduling "+getWorkName()+", replace="+replace);
        ExistingWorkPolicy policy=replace?ExistingWorkPolicy.REPLACE:ExistingWorkPolicy.KEEP;
        OneTimeWorkRequest localRequest=(new OneTimeWorkRequest.Builder(DataSyncWorker.class)).addTag(Data.WORK_TAG).setInputData(setWorkInput(new androidx.work.Data.Builder()).build()).build();
        WorkContinuation cont=WorkManager.getInstance().beginUniqueWork(getWorkName(),policy,localRequest);
        if(flag==SyncFlags.NETWORK)
            {
                cont=cont.then((new OneTimeWorkRequest.Builder(ConfirmNetworkDataWorker.class)).addTag(Data.WORK_TAG).build());
                cont=cont.then((new OneTimeWorkRequest.Builder(NetworkDataSyncWorker.class)).addTag(Data.WORK_TAG).setConstraints((new Constraints.Builder()).setRequiredNetworkType(getNetworkTypeSetting(context)).build()).build());
}
        cont.enqueue();
}
}

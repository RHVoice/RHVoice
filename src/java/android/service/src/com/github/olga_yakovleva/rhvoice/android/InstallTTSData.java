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
import java.io.BufferedInputStream;
import java.io.FileOutputStream;
import java.io.BufferedOutputStream;
import java.util.zip.ZipInputStream;
import java.util.zip.ZipEntry;
import android.app.Activity;
import android.os.Bundle;
import android.os.AsyncTask;
import android.content.res.Resources;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;
import android.util.Log;
import android.view.accessibility.AccessibilityEvent;
import android.speech.tts.TextToSpeech;

public final class InstallTTSData extends Activity
{
    static private final String TAG="RHVoice";

    static public final String BROADCAST_TTS_DATA_UPDATED="com.github.olga_yakovleva.rhvoice.BROADCAST_TTS_DATA_UPDATED";

    private class Installer extends AsyncTask<Void,Void,Integer>
    {
        private final File data_dir;
        private final Resources resources;

        public Installer()
            {
                data_dir=TTSData.getDir(InstallTTSData.this);
                resources=getResources();
            }

        private void clean(File obj,boolean delete_self)
            {
                if(obj.isDirectory())
                    {
                        final File[] children=obj.listFiles();
                        for(File child: children)
                            {
                                clean(child,true);
                            }
                    }
                if(delete_self)
                    obj.delete();
            }

        private void clean()
        {
            clean(data_dir,false);
        }

        protected void onPreExecute()
        {
            TTSData.resetVersion(InstallTTSData.this);
        }

        @Override
        protected Integer doInBackground(Void... params)
        {
            Integer result=RESULT_CANCELED;
            clean();
            try
                {
                    ZipInputStream in_stream=null;
                    try
                        {
                            in_stream=new ZipInputStream(new BufferedInputStream(resources.openRawResource(R.raw.data)));
                            ZipEntry entry=null;
                            final byte[] bytes=new byte[1024];
                            int count=0;
                            while(!isCancelled()&&((entry=in_stream.getNextEntry())!=null))
                                {
                                    Log.v(TAG,"Extracting "+entry.getName());
                                    File out_obj=new File(data_dir,entry.getName());
                                    if(entry.isDirectory())
                                        out_obj.mkdirs();
                                    else
                                        {
                                            out_obj.getParentFile().mkdirs();
                                            BufferedOutputStream out_stream=null;
                                            try
                                                {
                                                    out_stream=new BufferedOutputStream(new FileOutputStream(out_obj));
                                                    while(!isCancelled()&&((count=in_stream.read(bytes))!=-1))
                                                        {
                                                            out_stream.write(bytes,0,count);
                                                        }
                                                }
                                            finally
                                                {
                                                    if(out_stream!=null)
                                                        out_stream.close();
                                                }
                                        }
                                    in_stream.closeEntry();
                                }
                        }
                    finally
                        {
                            if(in_stream!=null)
                                in_stream.close();
                        }
                    if(!isCancelled())
                        result=RESULT_OK;
                }
            catch(Exception e)
                {
                    Log.e(TAG,"Error while installing voices",e);
                    clean();
                }
            return result;
        }

        @Override
        protected void onPostExecute(Integer result)
        {
            if(result==RESULT_OK)
                {
                    TTSData.updateVersion(InstallTTSData.this);
                    Log.i(TAG,"Voices installed successfully");
                    final Intent local_intent=new Intent(BROADCAST_TTS_DATA_UPDATED);
                    LocalBroadcastManager.getInstance(InstallTTSData.this).sendBroadcast(local_intent);
                }
            final Intent global_intent=new Intent(TextToSpeech.Engine.ACTION_TTS_DATA_INSTALLED);
            global_intent.putExtra(TextToSpeech.Engine.EXTRA_TTS_DATA_INSTALLED,(result==RESULT_OK)?TextToSpeech.SUCCESS:TextToSpeech.ERROR);
            sendBroadcast(global_intent);
            setResult(result);
            finish();
        }
    }

    private Installer installer=null;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.installer);
        installer=this.new Installer();
        Log.i(TAG,"Installing voices");
        installer.execute();
        findViewById(R.id.installing_voices).sendAccessibilityEvent(AccessibilityEvent.TYPE_VIEW_FOCUSED);
    }

    @Override
    protected void onDestroy()
    {
        installer.cancel(true);
        super.onDestroy();
    }
}

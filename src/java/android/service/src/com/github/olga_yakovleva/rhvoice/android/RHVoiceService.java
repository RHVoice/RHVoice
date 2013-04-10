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

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.io.File;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantReadWriteLock;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.support.v4.content.LocalBroadcastManager;
import android.media.AudioFormat;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeechService;
import android.speech.tts.SynthesisRequest;
import android.speech.tts.SynthesisCallback;
import android.util.Log;
import com.github.olga_yakovleva.rhvoice.*;

public final class RHVoiceService extends TextToSpeechService
{
    private static final String TAG="RHVoice";
    private static final int[] language_support_constants={TextToSpeech.LANG_NOT_SUPPORTED,TextToSpeech.LANG_AVAILABLE,TextToSpeech.LANG_COUNTRY_AVAILABLE,TextToSpeech.LANG_COUNTRY_VAR_AVAILABLE};
    private BroadcastReceiver reloader=null;
    private volatile boolean speaking=false;
    private final ReentrantReadWriteLock engine_lock=new ReentrantReadWriteLock();
    private final Lock engine_read_lock=engine_lock.readLock();
    private final Lock engine_write_lock=engine_lock.writeLock();
    private TTSEngine engine=null;
    private final List<AndroidVoiceInfo> voices=new ArrayList<AndroidVoiceInfo>();
    private volatile AndroidVoiceInfo current_voice=null;

    private class Player implements TTSClient
    {
        private SynthesisCallback callback;

        public Player(SynthesisCallback callback)
        {
            this.callback=callback;
        }

        public boolean playSpeech(short[] samples)
        {
            if(!speaking)
                return false;
            final ByteBuffer buffer=ByteBuffer.allocate(samples.length*2);
            buffer.order(ByteOrder.LITTLE_ENDIAN);
            buffer.asShortBuffer().put(samples);
            final byte[] bytes=buffer.array();
            final int size=callback.getMaxBufferSize();
            int offset=0;
            int count;
            while(offset<bytes.length)
                {
                    if(!speaking)
                        return false;
                    count=Math.min(size,bytes.length-offset);
                    if(callback.audioAvailable(bytes,offset,count)!=TextToSpeech.SUCCESS)
                        return false;
                    offset+=count;
                }
            return true;
        }
    };

    private class Reloader extends BroadcastReceiver
    {
        @Override
        public void onReceive(Context context,Intent intent)
        {
            Log.i(TAG,"The service is notified that voice data has been updated and will try to reload the engine");
            onStop();
            if(initialize())
                Log.i(TAG,"The engine has been reloaded");
            else
                Log.e(TAG,"Unable to reload the engine");
        }
    };

    private static class Candidate
    {
        final AndroidVoiceInfo voice;
        final int score;

        Candidate()
        {
            voice=null;
            score=0;
        }

        Candidate(AndroidVoiceInfo voice,String language,String country,String variant)
        {
            this.voice=voice;
            score=voice.getSupportLevel(language,country,variant);
        }
    };

    private Candidate find_best_voice(String language,String country,String variant)
    {
        Candidate best=new Candidate();
        for(AndroidVoiceInfo voice: voices)
            {
                Candidate candidate=new Candidate(voice,language,country,variant);
                if(candidate.score>best.score)
                    best=candidate;
            }
        return best;
    }

    private boolean initialize()
    {
        TTSEngine new_engine=null;
        try
            {
                File data_dir=TTSData.getDir(this);
                File config_dir=getDir("config",0);
                new_engine=new TTSEngine(data_dir.getCanonicalPath(),config_dir.getCanonicalPath());
            }
        catch(Exception e)
            {
                Log.e(TAG,"Error during engine initialization",e);
            }
        if(new_engine==null)
            return false;
        engine_write_lock.lock();
        try
            {
                if(engine!=null)
                    {
                        voices.clear();
                        engine.shutdown();
                    }
                engine=new_engine;
                AndroidVoiceInfo first_voice=null;
                List<VoiceInfo> engine_voices=engine.getVoices();
                for(VoiceInfo engine_voice: engine_voices)
                    {
                        AndroidVoiceInfo voice=new AndroidVoiceInfo(engine_voice);
                        voices.add(voice);
                        if(first_voice==null)
                            first_voice=voice;
                    }
                Locale locale=Locale.getDefault();
                Candidate best_match=find_best_voice(locale.getISO3Language(),locale.getISO3Country(),"");
                if(best_match.voice!=null)
                    current_voice=best_match.voice;
                else
                    current_voice=first_voice;
            }
        finally
            {
                engine_write_lock.unlock();
            }
        return true;
    }

    private void uninitialize()
    {
        engine_write_lock.lock();
        try
            {
                if(engine!=null)
                    {
                        engine.shutdown();
                        engine=null;
                    }
            }
        finally
            {
                engine_write_lock.unlock();
            }
    }
    
    @Override
    public void onCreate()
    {
        Log.i(TAG,"Initializing the service");
        final IntentFilter filter=new IntentFilter(InstallTTSData.BROADCAST_TTS_DATA_UPDATED);
        reloader=new Reloader();
        LocalBroadcastManager.getInstance(this).registerReceiver(reloader,filter);
        if(TTSData.isInstalled(this))
        {
            if(initialize())
                {
                    Log.i(TAG,"Service initialized successfully");
                    super.onCreate();
                }
            else
                Log.e(TAG,"Unable to initialize the service");
        }
        else
            {
                Log.i(TAG,"The service has found no voices and will try to install them");
                Intent intent=new Intent(this,InstallTTSData.class);
                intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                startActivity(intent);
            }
    }

    @Override
    public void onDestroy()
    {
        LocalBroadcastManager.getInstance(this).unregisterReceiver(reloader);
        onStop();
        super.onDestroy();
        uninitialize();
    }

    @Override
    protected String[] onGetLanguage()
    {
        String[] result={"rus","RUS",""};
        AndroidVoiceInfo voice=current_voice;
        if(voice!=null)
            {
                result[0]=voice.getLanguage();
                result[1]=voice.getCountry();
                result[2]=voice.getVariant();
            }
        return result;
    }

    @Override
    protected int onIsLanguageAvailable(String language,String country,String variant)
    {
        Candidate best_match=new Candidate();
        engine_read_lock.lock();
        try
            {
                if(engine!=null)
                    best_match=find_best_voice(language,country,variant);
            }
        finally
            {
                engine_read_lock.unlock();
            }
        return language_support_constants[best_match.score];
    }

    @Override
    protected int onLoadLanguage(String language,String country,String variant)
    {
        return onIsLanguageAvailable(language,country,variant);
    }

    @Override
    protected void onStop()
    {
        speaking=false;
    }

    @Override
    protected void onSynthesizeText(SynthesisRequest request,SynthesisCallback callback)
    {
        engine_read_lock.lock();
        try
            {
                speaking=true;
                if(engine==null)
                    {
                        Log.e(TAG,"onSynthesizeText has been called, but the service has not been initialized yet");
                        callback.error();
                        return;
                    }
                String language_description="language="+request.getLanguage()+"&&country="+request.getCountry()+"&&variant="+request.getVariant();
                final Candidate best_match=find_best_voice(request.getLanguage(),request.getCountry(),request.getVariant());
                if(best_match.voice==null)
                    {
                        Log.e(TAG,"Unsupported language: "+language_description);
                        callback.error();
                        return;
                    }
                Log.v(TAG,"Requested language: "+language_description+", selected voice: "+best_match.voice.getSource().getName());
                current_voice=best_match.voice;
                Log.v(TAG,"Synthesizing the following text: "+request.getText());
                final SynthesisParameters params=new SynthesisParameters();
                params.setMainVoice(best_match.voice.getSource());
                final Player player=new Player(callback);
                callback.start(16000,AudioFormat.ENCODING_PCM_16BIT,1);
                engine.speak(request.getText(),params,player);
                callback.done();
            }
        catch(RHVoiceException e)
            {
                Log.e(TAG,"Synthesis error",e);
                callback.error();
            }
        finally
            {
                engine_read_lock.unlock();
                speaking=false;
            }
    }
}

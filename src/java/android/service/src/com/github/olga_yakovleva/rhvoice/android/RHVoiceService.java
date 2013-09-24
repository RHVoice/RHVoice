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
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;
import java.util.Set;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioFormat;
import android.preference.PreferenceManager;
import android.speech.tts.SynthesisCallback;
import android.speech.tts.SynthesisRequest;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeechService;
import android.util.Log;

import com.github.olga_yakovleva.rhvoice.*;

public final class RHVoiceService extends TextToSpeechService implements FutureDataResult.Receiver
{
    private static final String TAG="RHVoiceTTSService";
    private static final int[] languageSupportConstants={TextToSpeech.LANG_NOT_SUPPORTED,TextToSpeech.LANG_AVAILABLE,TextToSpeech.LANG_COUNTRY_AVAILABLE,TextToSpeech.LANG_COUNTRY_VAR_AVAILABLE};

    private class OnPackageAddedReceiver extends BroadcastReceiver
    {
        @Override
        public void onReceive(Context context,Intent intent)
        {
            onPackageAdded(intent);
        }
    }

    private static class TtsState
    {
        public TTSEngine engine=null;
        public List<AndroidVoiceInfo> voices=new ArrayList<AndroidVoiceInfo>();
        public AndroidVoiceInfo voice=null;
    }

    private volatile TtsState ttsState;
        private OnPackageAddedReceiver onPackageAddedReceiver;
    private FutureDataResult dataResult;
    private volatile boolean speaking=false;

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

    static private class Candidate
    {
        public final AndroidVoiceInfo voice;
        public final int score;

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

    private Set<String> getPreferredVoices()
    {
        return PreferenceManager.getDefaultSharedPreferences(this).getStringSet("preferred_voices",null);
    }

    private Candidate findBestVoice(TtsState state,String language,String country,String variant,Set<String> preferredVoices)
    {
        Candidate best=new Candidate();
        Candidate bestPreferred=new Candidate();
        for(AndroidVoiceInfo voice: state.voices)
            {
                Candidate candidate=new Candidate(voice,language,country,variant);
                if(candidate.score>best.score)
                    best=candidate;
                if((preferredVoices!=null)&&preferredVoices.contains(candidate.voice.getSource().getName()))
                    {
                        if(candidate.score>bestPreferred.score)
                    bestPreferred=candidate;
                    }
            }
        return bestPreferred.score>=best.score?bestPreferred:best;
    }

    public void onDataFailure()
    {
    }

    public void onDataSuccess(String[] paths)
    {
        if(BuildConfig.DEBUG)
            Log.i(TAG,"Initializing the engine");
        TtsState newState=new TtsState();
        try
            {
                File configDir=getDir("config",0);
                newState.engine=new TTSEngine("",configDir.getAbsolutePath(),paths);
            }
        catch(Exception e)
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Error during engine initialization",e);
                return;
            }
        AndroidVoiceInfo firstVoice=null;
        List<VoiceInfo> engineVoices=newState.engine.getVoices();
        for(VoiceInfo engineVoice: engineVoices)
            {
                AndroidVoiceInfo nextVoice=new AndroidVoiceInfo(engineVoice);
                newState.voices.add(nextVoice);
                if(firstVoice==null)
                    firstVoice=nextVoice;
            }
        Locale locale=Locale.getDefault();
        Candidate bestMatch=findBestVoice(newState,locale.getISO3Language(),locale.getISO3Country(),"",getPreferredVoices());
        if(bestMatch.voice!=null)
            newState.voice=bestMatch.voice;
        else
            newState.voice=firstVoice;
        ttsState=newState;
    }

    @Override
    public void onCreate()
    {
        if(BuildConfig.DEBUG)
            Log.i(TAG,"Starting the service");
        dataResult=DataService.prepareData(this,this);
        IntentFilter filter=new IntentFilter(Intent.ACTION_PACKAGE_ADDED);
        filter.addDataScheme("package");
        onPackageAddedReceiver=this.new OnPackageAddedReceiver();
        registerReceiver(onPackageAddedReceiver,filter);
        super.onCreate();
    }

    @Override
    public void onDestroy()
    {
        if(dataResult!=null)
            dataResult.unregisterReceiver();
        if(onPackageAddedReceiver!=null)
            unregisterReceiver(onPackageAddedReceiver);
        onStop();
        super.onDestroy();
        TtsState state=ttsState;
        if(state.engine!=null)
            state.engine.shutdown();
    }

    private void onPackageAdded(Intent intent)
    {
        int uid=getApplicationInfo().uid;
        if(intent.getIntExtra(Intent.EXTRA_UID,uid)!=uid)
            return;
        String packageName=intent.getData().getSchemeSpecificPart();
        if(BuildConfig.DEBUG)
            Log.i(TAG,"Package "+packageName+" has been installed/updated");
        if(packageName==getPackageName())
            return;
        if(BuildConfig.DEBUG)
            Log.i(TAG,"Trying to reload the data");
        if(dataResult!=null)
            {
                dataResult.unregisterReceiver();
                dataResult=null;
            }
        dataResult=DataService.prepareData(this,this);
    }

    @Override
    protected String[] onGetLanguage()
    {
        String[] result={"rus","RUS",""};
        TtsState state=ttsState;
        if(state==null)
            return result;
        result[0]=state.voice.getLanguage();
        result[1]=state.voice.getCountry();
        result[2]=state.voice.getVariant();
        return result;
    }

    @Override
    protected int onIsLanguageAvailable(String language,String country,String variant)
    {
        TtsState state=ttsState;
        if(state==null)
            return TextToSpeech.LANG_NOT_SUPPORTED;
        Candidate bestMatch=findBestVoice(state,language,country,variant,null);
        return languageSupportConstants[bestMatch.score];
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
        TtsState state=ttsState;
        if(state==null)
            {
                callback.error();
                return;
            }
        try
            {
                speaking=true;
                String langDesc="language="+request.getLanguage()+"&&country="+request.getCountry()+"&&variant="+request.getVariant();
                Set<String> preferredVoices=getPreferredVoices();
                final Candidate bestMatch=findBestVoice(state,request.getLanguage(),request.getCountry(),request.getVariant(),preferredVoices);
                if(bestMatch.voice==null)
                    {
                        if(BuildConfig.DEBUG)
                            Log.e(TAG,"Unsupported language: "+langDesc);
                        callback.error();
                        return;
                    }
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"Requested language: "+langDesc+", selected voice: "+bestMatch.voice.getSource().getName());
                state.voice=bestMatch.voice;
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"Synthesizing the following text: "+request.getText());
                int rate=request.getSpeechRate();
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"rate="+rate);
                int pitch=request.getPitch();
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"pitch="+pitch);
                final SynthesisParameters params=new SynthesisParameters();
                params.setMainVoice(bestMatch.voice.getSource());
                params.setRate(((double)rate)/100.0);
                params.setPitch(((double)pitch)/100.0);
                final Player player=new Player(callback);
                callback.start(16000,AudioFormat.ENCODING_PCM_16BIT,1);
                state.engine.speak(request.getText(),params,player);
                callback.done();
            }
        catch(RHVoiceException e)
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Synthesis error",e);
                callback.error();
            }
        finally
            {
                speaking=false;
            }
    }
}

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

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.media.AudioFormat;
import android.os.Build;
import android.preference.PreferenceManager;
import android.speech.tts.SynthesisCallback;
import android.speech.tts.SynthesisRequest;
import android.speech.tts.TextToSpeech;
import android.speech.tts.TextToSpeechService;
import android.speech.tts.Voice;
import android.support.v4.content.LocalBroadcastManager;
import android.text.TextUtils;
import android.util.Log;
import com.github.olga_yakovleva.rhvoice.*;
import java.io.File;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Set;

public final class RHVoiceService extends TextToSpeechService
{
    private static final String TAG="RHVoiceTTSService";

    private static final int[] languageSupportConstants={TextToSpeech.LANG_NOT_SUPPORTED,TextToSpeech.LANG_AVAILABLE,TextToSpeech.LANG_COUNTRY_AVAILABLE,TextToSpeech.LANG_COUNTRY_VAR_AVAILABLE};

    private final BroadcastReceiver receiver=new BroadcastReceiver()
        {
            @Override
            public void onReceive(Context context,Intent intent)
            {
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"Data state changed");
                initialize();
}
        };

    private static interface SettingValueTranslator
    {
        public Object load(SharedPreferences prefs,String key);
        public String translate(Object value);
}

    private static final SettingValueTranslator prosodySettingValueTranslator=new SettingValueTranslator()
        {
            public Object load(SharedPreferences prefs,String key)
            {
                return prefs.getString(key,"100");
}

            public String translate(Object value)
            {
                try
                    {
                        int n=Integer.parseInt((String)value);
                        float f=n/100.0f;
                        return String.valueOf(f);
}
                catch(NumberFormatException e)
                    {
                        return "1";
}
}
        };

    private static class MappedSetting
    {
        public final String prefKey;
        public Object prefValue;
        public final String nativeKey;
        public final SettingValueTranslator valueTranslator;

        public MappedSetting(String prefKey,String nativeKey,SettingValueTranslator valueTranslator)
        {
            this.prefKey=prefKey;
            this.nativeKey=nativeKey;
            this.valueTranslator=valueTranslator;
}
}

    private static class Tts
    {
        public TTSEngine engine=null;
        public List<AndroidVoiceInfo> voices;
        public Set<String> languages;
        public Map<String,AndroidVoiceInfo> voiceIndex;
        public List<MappedSetting> mappedSettings;

        public Tts()
        {
            voices=new ArrayList<AndroidVoiceInfo>();
            languages=new HashSet<String>();
            voiceIndex=new HashMap<String,AndroidVoiceInfo>();
            mappedSettings=new ArrayList<MappedSetting>();
}

        public Tts(Tts other,boolean passEngine)
        {
            this.voices=other.voices;
            this.languages=other.languages;
            this.voiceIndex=other.voiceIndex;
            this.mappedSettings=other.mappedSettings;
            if(!passEngine)
                return;
            this.engine=other.engine;
            other.engine=null;
}
    }

    private static class TtsManager
        {
            private Tts tts;
            private boolean done;

            public synchronized void reset(Tts newTts)
            {
                if(newTts==null)
                    throw new IllegalArgumentException();
                if(tts!=null&&tts.engine!=null)
                    tts.engine.shutdown();
                tts=newTts;
            }

            public synchronized void destroy()
            {
                done=true;
                if(tts!=null&&tts.engine!=null)
                    tts.engine.shutdown();
                tts=null;
            }

            public synchronized Tts acquireForSynthesis()
            {
                if(done)
                    return null;
                if(tts==null)
                    return null;
                if(tts.engine==null)
                    return null;
                if(tts.voices.isEmpty())
                    return null;
                Tts result=new Tts(tts,true);
                return result;
}

            public synchronized void release(Tts usedTts)
            {
                if(usedTts==null||usedTts.engine==null)
                    throw new IllegalArgumentException();
                if(done||tts.engine!=null)
                    usedTts.engine.shutdown();
                else
                    tts=new Tts(usedTts,true);
}

            public synchronized Tts get()
            {
                if(tts==null)
                    return null;
                return new Tts(tts,false);
}
    }

    private final TtsManager ttsManager=new TtsManager();
    private volatile AndroidVoiceInfo currentVoice;
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
        public AndroidVoiceInfo voice;
        public int score;

        public Candidate()
        {
            voice=null;
            score=0;
        }

        public Candidate(AndroidVoiceInfo voice,String language,String country,String variant)
        {
            this.voice=voice;
            score=voice.getSupportLevel(language,country,variant);
        }
    }

    private static class LanguageSettings
    {
        public AndroidVoiceInfo voice;
        public boolean detect;
    }

    private void logLanguage(String language,String country,String variant)
    {
        Log.v(TAG,"Language: "+language);
        if(TextUtils.isEmpty(country))
            return;
        Log.v(TAG,"Country: "+country);
        if(TextUtils.isEmpty(variant))
            return;
        Log.v(TAG,"Variant: "+variant);
}

    private Map<String,LanguageSettings> getLanguageSettings(Tts tts)
    {
        Map<String,LanguageSettings> result=new HashMap<String,LanguageSettings>();
        SharedPreferences prefs=PreferenceManager.getDefaultSharedPreferences(this);
        for(String language: tts.languages)
            {
                LanguageSettings settings=new LanguageSettings();
                String prefVoice=prefs.getString("language."+language+".voice",null);
                for(AndroidVoiceInfo voice: tts.voices)
                    {
                        if(!voice.getSource().getLanguage().getAlpha3Code().equals(language))
                            continue;
                        String voiceName=voice.getSource().getName();
                        if(settings.voice==null)
                            {
                                settings.voice=voice;
                                if(prefVoice==null)
                                    break;
                            }
                        if(voiceName.equals(prefVoice))
                            {
                                settings.voice=voice;
                                break;
                            }
                    }
                settings.detect=prefs.getBoolean("language."+language+".detect",true);
                result.put(language,settings);
            }
        return result;
    }

    private Candidate findBestVoice(Tts tts,String language,String country,String variant,String voiceName,Map<String,LanguageSettings> languageSettings)
    {
        if(!TextUtils.isEmpty(voiceName))
            {
                AndroidVoiceInfo voice=tts.voiceIndex.get(voiceName);
                if(voice!=null)
                    return new Candidate(voice,language,country,"");
}
        Candidate best=new Candidate();
        for(AndroidVoiceInfo voice: tts.voices)
            {
                Candidate candidate=new Candidate(voice,language,country,variant);
                if(candidate.score>best.score||best.voice==null)
                    best=candidate;
            }
        if(best.score==3)
            return best;
        if(best.voice==null)
            best.voice=tts.voices.get(0);
        LanguageSettings settings=null;
        if(languageSettings!=null)
            settings=languageSettings.get(best.voice.getLanguage());
        if(settings==null)
            return best;
        if(settings.voice!=null)
            best.voice=settings.voice;
        return best;
    }

    private void initialize()
    {
        if(BuildConfig.DEBUG)
            Log.i(TAG,"Initializing the engine");
        DataManager dm=new DataManager(this);
        dm.checkFiles();
        if(!dm.isUpToDate())
            startService(new Intent(this,DataService.class));
        List<String> paths=dm.getUpToDatePaths();
        if(paths.isEmpty())
            {
                Log.w(TAG,"No voice data");
                return;
}
        Tts tts=new Tts();
        try
            {
                File configDir=Config.getDir(this);
                tts.engine=new TTSEngine("",configDir.getAbsolutePath(),paths,CoreLogger.instance);
            }
        catch(Exception e)
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Error during engine initialization",e);
                return;
            }
        List<VoiceInfo> engineVoices=tts.engine.getVoices();
        if(engineVoices.isEmpty())
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"No voices");
                tts.engine.shutdown();
                return;
}
        for(VoiceInfo engineVoice: engineVoices)
            {
                AndroidVoiceInfo nextVoice=new AndroidVoiceInfo(engineVoice);
                if(BuildConfig.DEBUG)
                    Log.i(TAG,"Found voice "+nextVoice.toString());
                tts.voices.add(nextVoice);
                tts.voiceIndex.put(nextVoice.getName(),nextVoice);
                tts.languages.add(engineVoice.getLanguage().getAlpha3Code());
            }
        for(String lang: tts.languages)
            {
                tts.mappedSettings.add(new MappedSetting("language."+lang+".volume","languages."+lang+".default_volume",prosodySettingValueTranslator));
                tts.mappedSettings.add(new MappedSetting("language."+lang+".rate","languages."+lang+".default_rate",prosodySettingValueTranslator));
}
        ttsManager.reset(tts);
    }

    @Override
    public void onCreate()
    {
        if(BuildConfig.DEBUG)
            Log.i(TAG,"Starting the service");
        LocalBroadcastManager.getInstance(this).registerReceiver(receiver,new IntentFilter(DataService.ACTION_DATA_STATE_CHANGED));
        initialize();
        super.onCreate();
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();
        LocalBroadcastManager.getInstance(this).unregisterReceiver(receiver);
        ttsManager.destroy();
    }

    @Override
    protected String[] onGetLanguage()
    {
        String[] result={"rus","RUS",""};
        AndroidVoiceInfo voice=currentVoice;
        if(voice==null)
            {
                Tts tts=ttsManager.get();
                if(tts!=null)
                    {
                        Locale locale=Locale.getDefault();
                        Candidate bestMatch=findBestVoice(tts,locale.getISO3Language(),locale.getISO3Country(),"","",getLanguageSettings(tts));
                        if(bestMatch.voice!=null)
                            voice=bestMatch.voice;
                    }
            }
        if(voice==null)
            return result;
        result[0]=voice.getLanguage();
        result[1]=voice.getCountry();
        return result;
    }

    @Override
    protected int onIsLanguageAvailable(String language,String country,String variant)
    {
        if(BuildConfig.DEBUG)
            {
                Log.v(TAG,"onIsLanguageAvailable called");
                logLanguage(language,country,variant);
}
        Tts tts=ttsManager.get();
        if(tts==null)
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"Not initialized yet");
                return TextToSpeech.LANG_NOT_SUPPORTED;
            }
        Candidate bestMatch=findBestVoice(tts,language,country,variant,"",null);
        int result=languageSupportConstants[bestMatch.score];
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Result: "+result);
        return result;
    }

    @Override
    protected int onLoadLanguage(String language,String country,String variant)
    {
        if(BuildConfig.DEBUG)
            Log.v(TAG,"onLoadLanguage called");
        return onIsLanguageAvailable(language,country,variant);
    }

    @Override
    protected void onStop()
    {
        speaking=false;
    }

    private void applyMappedSettings(Tts tts)
    {
        SharedPreferences prefs=PreferenceManager.getDefaultSharedPreferences(this);
        Object oldPrefValue;
        String nativeValue;
        for(MappedSetting setting: tts.mappedSettings)
            {
                oldPrefValue=setting.prefValue;
                setting.prefValue=setting.valueTranslator.load(prefs,setting.prefKey);
                if(oldPrefValue!=null&&oldPrefValue.equals(setting.prefValue))
                    continue;
                nativeValue=setting.valueTranslator.translate(setting.prefValue);
                tts.engine.configure(setting.nativeKey,nativeValue);
}
}

    @Override
    protected void onSynthesizeText(SynthesisRequest request,SynthesisCallback callback)
    {
        if(BuildConfig.DEBUG)
            {
                Log.v(TAG,"onSynthesize called");
                logLanguage(request.getLanguage(),request.getCountry(),request.getVariant());
            }
        Tts tts=ttsManager.acquireForSynthesis();
        if(tts==null)
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"Not initialized yet");
                callback.error();
                return;
            }
        try
            {
                speaking=true;
                Map<String,LanguageSettings> languageSettings=getLanguageSettings(tts);
                String voiceName="";
                if(Build.VERSION.SDK_INT>=Build.VERSION_CODES.LOLLIPOP)
                    voiceName=request.getVoiceName();
                final Candidate bestMatch=findBestVoice(tts,request.getLanguage(),request.getCountry(),request.getVariant(),voiceName,languageSettings);
                if(bestMatch.voice==null)
                    {
                        if(BuildConfig.DEBUG)
                            Log.e(TAG,"Unsupported language");
                        callback.error();
                        return;
                    }
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"Selected voice: "+bestMatch.voice.getSource().getName());
                currentVoice=bestMatch.voice;
                StringBuilder voiceProfileSpecBuilder=new StringBuilder();
                voiceProfileSpecBuilder.append(bestMatch.voice.getSource().getName());
                for(Map.Entry<String,LanguageSettings> entry: languageSettings.entrySet())
                    {
                        if(entry.getKey().equals(bestMatch.voice.getLanguage()))
                            continue;
                        if(entry.getValue().detect)
                            {
                                String name=entry.getValue().voice.getSource().getName();
                                voiceProfileSpecBuilder.append("+").append(name);
                            }
                    }
                String profileSpec=voiceProfileSpecBuilder.toString();
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"Synthesizing the following text: "+request.getText());
                int rate=request.getSpeechRate();
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"rate="+rate);
                int pitch=request.getPitch();
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"pitch="+pitch);
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"Profile: "+profileSpec);
                applyMappedSettings(tts);
                final SynthesisParameters params=new SynthesisParameters();
                params.setVoiceProfile(profileSpec);
                params.setRate(((double)rate)/100.0);
                params.setPitch(((double)pitch)/100.0);
                final Player player=new Player(callback);
                callback.start(24000,AudioFormat.ENCODING_PCM_16BIT,1);
                tts.engine.speak(request.getText(),params,player);
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
                ttsManager.release(tts);
            }
    }

    @Override
    public String onGetDefaultVoiceNameFor(String language,String country,String variant)
    {
        if(BuildConfig.DEBUG)
            {
                Log.v(TAG,"onGetDefaultVoiceNameFor called");
                    logLanguage(language,country,variant);
                    }
        Tts tts=ttsManager.get();
        if(tts==null)
            return null;
        Candidate bestMatch=findBestVoice(tts,language,country,variant,"",getLanguageSettings(tts));
        if(bestMatch==null||bestMatch.voice==null)
            return null;
        String name=bestMatch.voice.getName();
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Voice name: "+name);
        return name;
}

    @Override
    public List<Voice> onGetVoices()
    {
        List<Voice> result=new ArrayList<Voice>();
        Tts tts=ttsManager.get();
        if(tts==null)
            return result;
        for(AndroidVoiceInfo voice: tts.voices)
            {
                result.add(voice.getAndroidVoice());
            }
        return result;
    }

    @Override
    public int onIsValidVoiceName(String name)
    {
        if(BuildConfig.DEBUG)
            Log.v(TAG,"onIsValidVoiceName called for name "+name);
        Tts tts=ttsManager.get();
        if(tts==null)
            return TextToSpeech.ERROR;
        if(tts.voiceIndex.containsKey(name))
            {
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"Voice found");
                return TextToSpeech.SUCCESS;
            }
        else
            {
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"Voice not found");
                return TextToSpeech.ERROR;
            }
}

    @Override
    public int onLoadVoice(String name)
    {
        return TextToSpeech.SUCCESS;
}
}

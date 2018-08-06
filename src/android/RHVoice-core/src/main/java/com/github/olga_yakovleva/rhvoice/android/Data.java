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
import android.util.Log;
import com.evernote.android.job.JobManager;
import com.evernote.android.job.JobRequest;
import com.github.olga_yakovleva.rhvoice.RHVoiceException;
import com.github.olga_yakovleva.rhvoice.TTSEngine;
import com.github.olga_yakovleva.rhvoice.VoiceInfo;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import com.evernote.android.job.util.support.PersistableBundleCompat;



public final class Data
{
    private static final String TAG="RHVoice.Data";
    private final static List<LanguagePack> languages=new ArrayList<LanguagePack>();
    private static final Map<String,LanguagePack> index=new HashMap<String,LanguagePack>();

    static
    {
        LanguagePack lang=null;
        lang=new LanguagePack("English", "eng", "USA", 2, 4, null, null);
        lang.addVoice(new VoicePack("Alan", lang, 4, 0, null, null));
        lang.addVoice(new VoicePack("BDL", lang, 4, 0, null, null));
        lang.addVoice(new VoicePack("CLB", lang, 4, 0, null, null));
        lang.addVoice(new VoicePack("SLT", lang, 4, 0, null, null));
        addLanguage(lang);
        lang=new LanguagePack("Esperanto", "epo", "", 1, 2, null, null);
        lang.addVoice(new VoicePack("Spomenka", lang, 4, 0, null, null));
        addLanguage(lang);
        lang=new LanguagePack("Georgian", "kat", "GEO", 1, 8, null, null);
        lang.addVoice(new VoicePack("Natia", lang, 4, 0, "http://blindaid.ge/files/RHVoice-voice-Georgian-Natia-v4.0.zip", null));
        addLanguage(lang);
        lang=new LanguagePack("Kyrgyz", "kir", "KGZ", 1, 15, null, null);
        lang.addVoice(new VoicePack("Azamat", lang, 4, 0, null, null));
        lang.addVoice(new VoicePack("Nazgul", lang, 4, 0, null, null));
        addLanguage(lang);
        lang=new LanguagePack("Russian", "rus", "RUS", 2, 3, null, null);
        lang.addVoice(new VoicePack("Aleksandr", lang, 4, 1, null, null));
        lang.addVoice(new VoicePack("Anna", lang, 4, 0, null, null));
        lang.addVoice(new VoicePack("Elena", lang, 4, 1, null, null));
        lang.addVoice(new VoicePack("Irina", lang, 4, 0, null, null));
        addLanguage(lang);
        lang=new LanguagePack("Tatar", "tat", "RUS", 1, 10, null, null);
        lang.addVoice(new VoicePack("Talgat", lang, 4, 0, "http://rsbsrt.ru/Talgat/RHVoice-voice-Tatar-Talgat-v4.0.zip", null));
        addLanguage(lang);
        lang=new LanguagePack("Ukrainian", "ukr", "UKR", 1, 4, null, null);
        lang.addVoice(new VoicePack("Anatol", lang, 4, 1, null, null));
        lang.addVoice(new VoicePack("Natalia", lang, 4, 0, null, null));
        addLanguage(lang);
}

    public static LanguagePack getLanguage(String name)
    {
        return index.get(name);
    }

    public static List<LanguagePack> getLanguages()
    {
        return languages;
}

    private static void addLanguage(LanguagePack language)
    {
        languages.add(language);
        index.put(language.getCode(),language);
}

    public static boolean sync(Context context,IDataSyncCallback callback)
    {
        boolean done=true;
        for(LanguagePack language: languages)
            {
                if(callback.isStopped())
                    break;
                if(!language.sync(context,callback))
                    done=false;
}
        if(callback.isStopped())
            done=true;
        return done;
}

    public static List<String> getPaths(Context context)
    {
        List<String> paths=new ArrayList<String>();
        for(LanguagePack language: languages)
            {
                paths.addAll(language.getPaths(context));
}
        return paths;
}

    public static List<VoiceInfo> getVoices(Context context)
    {
        TTSEngine engine=null;
        try
            {
                engine=new TTSEngine("",Config.getDir(context).getAbsolutePath(),getPaths(context),CoreLogger.instance);
                return engine.getVoices();
            }
        catch(RHVoiceException e)
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Engine initialization failed",e);
                return Collections.<VoiceInfo>emptyList();
            }
        finally
            {
                if(engine!=null)
                    engine.shutdown();
}
}

    public static long getSyncFlags(Context context)
    {
        long flags=0;
        for(LanguagePack language: languages)
            {
                flags|=language.getSyncFlags(context);
}
        if(BuildConfig.DEBUG)
            Log.v(TAG,"SyncFlags="+flags);
        return flags;
}

    public static JobRequest.NetworkType getNetworkTypeSetting(Context context)
    {
        boolean wifiOnly=DataPack.getPrefs(context).getBoolean("wifi_only",true);
        return wifiOnly?JobRequest.NetworkType.UNMETERED:JobRequest.NetworkType.CONNECTED;
}

    public static void scheduleSync(Context context)
    {
        long flags=getSyncFlags(context);
        if(flags==0)
            return;
        JobRequest.NetworkType networkType=JobRequest.NetworkType.ANY;
        if((flags&SyncFlags.NETWORK)!=0)
            networkType=getNetworkTypeSetting(context);
        JobRequest.Builder builder=new JobRequest.Builder(DataSyncJob.JOB_TAG);
        builder.setExecutionWindow(1,10);
        builder.setRequiredNetworkType(networkType);
        if((flags&SyncFlags.NETWORK)!=0&&(flags&SyncFlags.LOCAL)==0)
            builder.setRequirementsEnforced(true);
        PersistableBundleCompat extras=new PersistableBundleCompat();
        extras.putLong("flags",flags);
        builder.setExtras(extras);
        builder.setUpdateCurrent(true);
        JobRequest request=builder.build();
        if(BuildConfig.DEBUG)
            Log.i(TAG,"Scheduling job request");
        request.schedule();
}
}

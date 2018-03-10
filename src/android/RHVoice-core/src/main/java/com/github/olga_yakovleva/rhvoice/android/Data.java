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
import com.github.olga_yakovleva.rhvoice.RHVoiceException;
import com.github.olga_yakovleva.rhvoice.TTSEngine;
import com.github.olga_yakovleva.rhvoice.VoiceInfo;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public final class Data
{
    private static final String TAG="RHVoice.Data";
    private final static List<LanguagePack> languages=new ArrayList<LanguagePack>();
    private static final Map<String,LanguagePack> index=new HashMap<String,LanguagePack>();

    static
    {
        LanguagePack lang=null;
        lang=new LanguagePack("English", "eng", "USA", 2, 3, null);
        lang.addVoice(new VoicePack("Alan", lang, 4, 0, null));
        lang.addVoice(new VoicePack("BDL", lang, 4, 0, null));
        lang.addVoice(new VoicePack("CLB", lang, 4, 0, null));
        lang.addVoice(new VoicePack("SLT", lang, 4, 0, null));
        addLanguage(lang);
        lang=new LanguagePack("Esperanto", "epo", "", 1, 2, null);
        lang.addVoice(new VoicePack("Spomenka", lang, 4, 0, null));
        addLanguage(lang);
        lang=new LanguagePack("Georgian", "kat", "GEO", 1, 6, null);
        lang.addVoice(new VoicePack("Natia", lang, 4, 0, null));
        addLanguage(lang);
        lang=new LanguagePack("Kyrgyz", "kir", "KGZ", 1, 13, null);
        lang.addVoice(new VoicePack("Azamat", lang, 4, 0, null));
        lang.addVoice(new VoicePack("Nazgul", lang, 4, 0, null));
        addLanguage(lang);
        lang=new LanguagePack("Russian", "rus", "RUS", 2, 3, null);
        lang.addVoice(new VoicePack("Aleksandr", lang, 4, 0, null));
        lang.addVoice(new VoicePack("Anna", lang, 4, 0, null));
        lang.addVoice(new VoicePack("Elena", lang, 4, 0, null));
        lang.addVoice(new VoicePack("Irina", lang, 4, 0, null));
        addLanguage(lang);
        lang=new LanguagePack("Tatar", "tat", "RUS", 1, 5, null);
        lang.addVoice(new VoicePack("Talgat", lang, 4, 0, null));
        addLanguage(lang);
        lang=new LanguagePack("Ukrainian", "ukr", "UKR", 1, 3, null);
        lang.addVoice(new VoicePack("Anatol", lang, 4, 0, null));
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
                if(!language.sync(context,callback))
                    done=false;
}
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

    public static boolean isSyncRequired(Context context)
    {
        for(LanguagePack language: languages)
            {
                if(language.isSyncRequired(context))
                    return true;
}
        return false;
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
}

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
        lang=new LanguagePack("English", "eng", "USA", 2, 2, "https://www.dropbox.com/s/1lthlfseb2wdvs8/data.zip?dl=1");
        lang.addVoice(new VoicePack("Alan", lang, 3, 0, "https://www.dropbox.com/s/flnc1kkidc0sgse/data.zip?dl=1"));
        lang.addVoice(new VoicePack("Bdl", lang, 3, 0, "https://www.dropbox.com/s/cqhj7nmgtizm051/data.zip?dl=1"));
        lang.addVoice(new VoicePack("Clb", lang, 3, 0, "https://www.dropbox.com/s/f84xmcr5lc4je5o/data.zip?dl=1"));
        lang.addVoice(new VoicePack("Slt", lang, 3, 0, "https://www.dropbox.com/s/x7idxdtn9amjsid/data.zip?dl=1"));
        addLanguage(lang);
        lang=new LanguagePack("Esperanto", "epo", "", 1, 2, "https://www.dropbox.com/s/dy78nluxxjozdza/data.zip?dl=1");
        lang.addVoice(new VoicePack("Spomenka", lang, 3, 0, "https://www.dropbox.com/s/eghhpy0hiq138lo/data.zip?dl=1"));
        addLanguage(lang);
        lang=new LanguagePack("Georgian", "kat", "GEO", 1, 6, "https://www.dropbox.com/s/ztk7s7kuye61mb4/data.zip?dl=1");
        lang.addVoice(new VoicePack("Natia", lang, 3, 0, "https://www.dropbox.com/s/jyqfza4156n0ccc/data.zip?dl=1"));
        addLanguage(lang);
        lang=new LanguagePack("Kyrgyz", "kir", "KGZ", 1, 12, "https://www.dropbox.com/s/10zxh9h68lm9l9h/data.zip?dl=1");
        lang.addVoice(new VoicePack("Azamat", lang, 3, 1, "https://www.dropbox.com/s/896kmg1j0kdoz6r/data.zip?dl=1"));
        lang.addVoice(new VoicePack("Nazgul", lang, 3, 0, "https://www.dropbox.com/s/x8k4wsxzq3hjedx/data.zip?dl=1"));
        addLanguage(lang);
        lang=new LanguagePack("Russian", "rus", "RUS", 2, 3, "https://www.dropbox.com/s/ejvyzyis2gkswn7/data.zip?dl=1");
        lang.addVoice(new VoicePack("Aleksandr", lang, 3, 0, "https://www.dropbox.com/s/by0ss7m92i5q5cp/data.zip?dl=1"));
        lang.addVoice(new VoicePack("Anna", lang, 3, 0, "https://www.dropbox.com/s/0irrntqes1irx6f/data.zip?dl=1"));
        lang.addVoice(new VoicePack("Elena", lang, 3, 0, "https://www.dropbox.com/s/bmg8mci4jzsxae5/data.zip?dl=1"));
        lang.addVoice(new VoicePack("Irina", lang, 3, 0, "https://www.dropbox.com/s/17cmypgl7qhp8pr/data.zip?dl=1"));
        addLanguage(lang);
        lang=new LanguagePack("Tatar", "tat", "RUS", 1, 5, "https://www.dropbox.com/s/5amabtqrqz5pxad/data.zip?dl=1");
        lang.addVoice(new VoicePack("Talgat", lang, 3, 0, "https://www.dropbox.com/s/ajlwemovrm13cz9/data.zip?dl=1"));
        addLanguage(lang);
        lang=new LanguagePack("Ukrainian", "ukr", "UKR", 1, 2, "https://www.dropbox.com/s/n00skvtatwyy3wr/data.zip?dl=1");
        lang.addVoice(new VoicePack("Anatol", lang, 3, 0, "https://www.dropbox.com/s/8x4wiy255bb6k4d/data.zip?dl=1"));
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

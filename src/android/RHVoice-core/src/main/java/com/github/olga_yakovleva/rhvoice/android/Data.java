/* Copyright (C) 2017, 2018, 2019, 2020  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

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
import android.text.TextUtils;

public final class Data
{
    private static final String TAG="RHVoice.Data";
    public static final String REPO_URL="https://rhvoice.eu-central-1.linodeobjects.com";
    private final static List<LanguagePack> languages=new ArrayList<LanguagePack>();
    private static final Map<String,LanguagePack> index=new HashMap<String,LanguagePack>();
    private static final Map<String,LanguagePack> idIndex=new HashMap<String,LanguagePack>();
    public static final String WORK_PREFIX_STRING="com.github.olga_yakovleva.rhvoice.android";
    public static final String WORK_TAG=WORK_PREFIX_STRING+".data";

    static
    {
        LanguagePack lang=null;
        lang=new LanguagePack("English", "eng", "en", "USA", "US", true, 2, 8, Checksums.LANGUAGE_ENGLISH, null, null);
        lang.addVoice(new VoicePack("Alan", lang, 4, 0, Checksums.VOICE_ALAN, null, null));
        lang.addVoice(new VoicePack("BDL", lang, 4, 1, Checksums.VOICE_BDL, null, null));
        lang.addVoice(new VoicePack("CLB", lang, 4, 0, Checksums.VOICE_CLB, null, null));
        lang.addDefaultVoice(new VoicePack("SLT", lang, 4, 0, Checksums.VOICE_SLT, null, null));
        lang.addVoice(new VoicePack("Evgeniy-Eng", lang, 4, 0, Checksums.VOICE_EVGENIY_ENG, "https://github.com/RHVoice/evgeniy-eng/releases/download/4.0/data.zip", null));
        lang.addVoice(new VoicePack("Lyubov", lang, 4, 0, Checksums.VOICE_LYUBOV, "https://github.com/RHVoice/lyubov-eng/releases/download/4.0/data.zip", null));
        addLanguage(lang);
        lang=new LanguagePack("Esperanto", "epo", "eo", "", "", false, 1, 2, Checksums.LANGUAGE_ESPERANTO, null, null);
        lang.addVoice(new VoicePack("Spomenka", lang, 4, 0, Checksums.VOICE_SPOMENKA, null, null));
        addLanguage(lang);
        lang=new LanguagePack("Georgian", "kat", "ka", "GEO", "GE", false, 1, 12, Checksums.LANGUAGE_GEORGIAN, null, null);
        lang.addVoice(new VoicePack("Natia", lang, 4, 0, Checksums.VOICE_NATIA, "http://blindaid.ge/files/RHVoice-voice-Georgian-Natia-v4.0.zip", null));
        addLanguage(lang);
        lang=new LanguagePack("Kyrgyz", "kir", "ky", "KGZ", "KG", false, 1, 19, Checksums.LANGUAGE_KYRGYZ, null, null);
        lang.addVoice(new VoicePack("Azamat", lang, 4, 0, Checksums.VOICE_AZAMAT, null, null));
        lang.addVoice(new VoicePack("Nazgul", lang, 4, 0, Checksums.VOICE_NAZGUL, null, null));
        addLanguage(lang);
        lang=new LanguagePack("Russian", "rus", "ru", "RUS", "RU", false, 2, 10, Checksums.LANGUAGE_RUSSIAN, null, null);
        lang.addVoice(new VoicePack("Aleksandr", lang, 4, 2, Checksums.VOICE_ALEKSANDR, null, null));
        lang.addVoice(new VoicePack("Aleksandr-hq", lang, 4, 0, Checksums.VOICE_ALEKSANDR_HQ, "https://github.com/RHVoice/aleksandr-hq-rus/releases/download/4.0/data.zip", null));
        lang.addDefaultVoice(new VoicePack("Anna", lang, 4, 1, Checksums.VOICE_ANNA, null, null));
        lang.addVoice(new VoicePack("Elena", lang, 4, 2, Checksums.VOICE_ELENA, null, null));
        lang.addVoice(new VoicePack("Irina", lang, 4, 1, Checksums.VOICE_IRINA, null, null));
        lang.addVoice(new VoicePack("Yuriy", lang, 4, 0, Checksums.VOICE_YURIY, "https://github.com/RHVoice/yuriy-rus/releases/download/4.0/data.zip", null));
        lang.addVoice(new VoicePack("Vitaliy", lang, 4, 0, Checksums.VOICE_VITALIY, "https://github.com/RHVoice/vitaliy-rus/releases/download/4.0/data.zip", null));
        lang.addVoice(new VoicePack("Artemiy", lang, 4, 0, Checksums.VOICE_ARTEMIY, "https://github.com/RHVoice/artemiy-rus/releases/download/4.0/data.zip", null));
        lang.addVoice(new VoicePack("Arina", lang, 4, 0, Checksums.VOICE_ARINA, "https://github.com/RHVoice/arina-rus/releases/download/4.0/data.zip", null));
        lang.addVoice(new VoicePack("Pavel", lang, 4, 0, Checksums.VOICE_PAVEL, "https://github.com/RHVoice/pavel-rus/releases/download/4.0/data.zip", null));
        lang.addVoice(new VoicePack("Victoria", lang, 4, 0, Checksums.VOICE_VICTORIA, "https://github.com/RHVoice/victoria-rus/releases/download/4.0/data.zip", null));
        lang.addVoice(new VoicePack("Tatiana", lang, 4, 0, Checksums.VOICE_TATIANA, "https://github.com/RHVoice/tatiana-rus/releases/download/4.0/data.zip", null));
        lang.addVoice(new VoicePack("Mikhail", lang, 4, 0, Checksums.VOICE_MIKHAIL, "https://github.com/RHVoice/mikhail-rus/releases/download/4.0/data.zip", null));
        lang.addVoice(new VoicePack("Evgeniy-Rus", lang, 4, 0, Checksums.VOICE_EVGENIY_RUS, "https://github.com/RHVoice/evgeniy-rus/releases/download/4.0/data.zip", null));
        addLanguage(lang);
        lang=new LanguagePack("Tatar", "tat", "tt", "RUS", "RU", false, 1, 10, Checksums.LANGUAGE_TATAR, null, null);
        lang.addVoice(new VoicePack("Talgat", lang, 4, 0, Checksums.VOICE_TALGAT, "https://rsbsrt.ru/Talgat/RHVoice-voice-Tatar-Talgat-v4.0.zip", null));
        addLanguage(lang);
        lang=new LanguagePack("Ukrainian", "ukr", "uk", "UKR", "UA", false, 1, 13, Checksums.LANGUAGE_UKRAINIAN, null, null);
        lang.addVoice(new VoicePack("Volodymyr", lang, 4, 0, Checksums.VOICE_VOLODYMYR, "https://github.com/RHVoice/volodymyr-ukr/releases/download/4.0/data.zip", null));
        lang.addVoice(new VoicePack("Marianna", lang, 4, 0, Checksums.VOICE_MARIANNA, "https://github.com/RHVoice/marianna-ukr/releases/download/4.0/data.zip", null));
        lang.addVoice(new VoicePack("Anatol", lang, 4, 1, Checksums.VOICE_ANATOL, null, null));
        lang.addDefaultVoice(new VoicePack("Natalia", lang, 4, 0, Checksums.VOICE_NATALIA, null, null));
        addLanguage(lang);
        lang=new LanguagePack("Brazilian-Portuguese", "por", "pt", "BRA", "BR", true, 1, 19, Checksums.LANGUAGE_BRAZILIAN_PORTUGUESE, REPO_URL+"/RHVoice-F123-Brazilian-Portuguese-language-v1.19.zip", null);
        lang.addVoice(new VoicePack("leticia_f123", "Let\u00Edcia-F123", lang, 4, 6, Checksums.VOICE_LETICIA, REPO_URL+"/Leticia-F123-v4.6.zip", null));
        addLanguage(lang);
        lang=new LanguagePack("Macedonian", "mkd", "mk", "MKD", "MK", false, 1, 21, Checksums.LANGUAGE_MACEDONIAN,REPO_URL+"/RHVoice-language-Macedonian-v1.21.zip" , null);
        lang.addVoice(new VoicePack("mac_m1", "Kiko", lang, 4, 7, Checksums.VOICE_MAC_MALE1, REPO_URL+"/RHVoice-voice-Macedonian-Kiko-v4.7.zip", null));
        addLanguage(lang);
        lang=new LanguagePack("Albanian", "sqi", "sq", "ALB", "AL", false, 1, 19, Checksums.LANGUAGE_ALBANIAN, "https://rhvoice.org/download/RHVoice-LP-language-Albanian-v1.19.zip", null);
        lang.addVoice(new VoicePack("Hana", lang, 4, 4, Checksums.VOICE_HANA, "https://rhvoice.org/download/RHVoice-LP-voice-Hana-v4.4.zip", null));
        addLanguage(lang);
}

    public static LanguagePack getLanguage(String name)
    {
        return index.get(name);
    }

    public static LanguagePack getLanguageById(String id)
    {
        return idIndex.get(id);
}

    public static List<LanguagePack> getLanguages()
    {
        return languages;
}

    private static void addLanguage(LanguagePack language)
    {
        languages.add(language);
        index.put(language.getTag(),language);
        idIndex.put(language.getId(),language);
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

    public static void scheduleSync(Context context,boolean replace)
    {
        for(LanguagePack lang: languages)
            {
                lang.scheduleSync(context,replace);
                for(VoicePack voice: lang.getVoices())
                    voice.scheduleSync(context,replace);
}
}

    public static LanguagePack findMatchingLanguage(String code,String cCode)
    {
        LanguagePack res=null;
        for(LanguagePack lang: languages)
            {
                if(!lang.getCode().equalsIgnoreCase(code))
                    continue;
                if(res==null)
                    {
                        res=lang;
                        if(TextUtils.isEmpty(cCode))
                            break;
}
                if(TextUtils.isEmpty(lang.getCountryCode()))
                    continue;
                if(lang.getCountryCode().equalsIgnoreCase(cCode))
                    {
                        res=lang;
                        break;
}
}
        return res;
}
}

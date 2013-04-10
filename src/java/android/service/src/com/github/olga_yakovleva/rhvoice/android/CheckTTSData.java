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

import java.util.List;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.io.File;
import java.io.IOException;
import android.app.Activity;
import android.os.Bundle;
import android.content.Intent;
import android.util.Log;
import android.speech.tts.TextToSpeech;
import com.github.olga_yakovleva.rhvoice.*;

public final class CheckTTSData extends Activity
{
    private static final String TAG="RHVoice";
    private static final int REQUEST_INSTALLATION=1;

    private List<AndroidVoiceInfo> listVoices() throws RHVoiceException,IOException
    {
        File data_dir=TTSData.getDir(this);
        File config_dir=getDir("config",0);
        TTSEngine engine=new TTSEngine(data_dir.getCanonicalPath(),config_dir.getCanonicalPath());
        List<VoiceInfo> voices=engine.getVoices();
        engine.shutdown();
        List<AndroidVoiceInfo> result=new ArrayList(voices.size());
        for(VoiceInfo voice: voices)
            {
                AndroidVoiceInfo android_voice=new AndroidVoiceInfo(voice);
                Log.d(TAG,"Found voice "+android_voice.toString());
                result.add(android_voice);
            }
        return result;
    }

    private void reply(int result_code,ArrayList<String> available_voices,ArrayList<String> unavailable_voices)
    {
        final Intent intent=new Intent();
        if(available_voices!=null)
            intent.putExtra(TextToSpeech.Engine.EXTRA_AVAILABLE_VOICES,available_voices);
        if(unavailable_voices!=null)
            intent.putExtra(TextToSpeech.Engine.EXTRA_UNAVAILABLE_VOICES,unavailable_voices);
        setResult(result_code,intent);
        finish();
    }

    void doCheck(boolean after_installation)
    {
        if(TTSData.isInstalled(this))
            {
                Log.d(TAG,"Checking which voices are installed");
                List<AndroidVoiceInfo> voices=null;
                try
                    {
                        voices=listVoices();
                    }
                catch(Exception e)
                    {
                        Log.e(TAG,"Unable to list installed voices",e);
                    }
                if(voices==null)
                    reply(TextToSpeech.Engine.CHECK_VOICE_DATA_BAD_DATA,null,null);
                else
                    {
                        ArrayList<String> available_voices=new ArrayList<String>();
                        ArrayList<String> unavailable_voices=new ArrayList<String>();
                        final ArrayList<String> requested_voices=getIntent().getStringArrayListExtra(TextToSpeech.Engine.EXTRA_CHECK_VOICE_DATA_FOR);
                        if((requested_voices==null)||requested_voices.isEmpty())
                            {
                                for(AndroidVoiceInfo voice: voices)
                                    {
                                        available_voices.add(voice.toString());
                                    }
                            }
                        else
                            {
                                LinkedHashSet<String> matching_voices=new LinkedHashSet<String>();
                                for(String name: requested_voices)
                                    {
                                        boolean found=false;
                                        for(AndroidVoiceInfo voice: voices)
                                            {
                                                if(voice.matches(name))
                                                    {
                                                        matching_voices.add(voice.toString());
                                                        found=true;
                                                    }
                                            }
                                        if(!found)
                                            unavailable_voices.add(name);
                                    }
                                available_voices.addAll(matching_voices);
                            }
                        reply(TextToSpeech.Engine.CHECK_VOICE_DATA_PASS,available_voices,unavailable_voices);
                    }
            }
        else
            {
                if(after_installation)
                    {
                        Log.e(TAG,"No voices found after installation");
                        reply(TextToSpeech.Engine.CHECK_VOICE_DATA_MISSING_DATA,null,null);
                    }
                else
                    {
                        Log.i(TAG,"No voices have been installed for this version, attempting installation");
                        final Intent intent=new Intent(this,InstallTTSData.class);
                        startActivityForResult(intent,REQUEST_INSTALLATION);
                    }
            }
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        Log.i(TAG,"Checking if the voices are installed");
        doCheck(false);
    }

    @Override
    protected void onActivityResult(int request_code,int result_code,Intent data)
    {
        if(request_code==REQUEST_INSTALLATION)
            {
                if(result_code==RESULT_OK)
                    doCheck(true);
                else
                    {
                        Log.e(TAG,"Voice installation attempt failed");
                        reply(TextToSpeech.Engine.CHECK_VOICE_DATA_MISSING_DATA,null,null);
                    }
            }
    }
}

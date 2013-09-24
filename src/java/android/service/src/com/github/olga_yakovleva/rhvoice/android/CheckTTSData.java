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

import java.io.IOException;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.speech.tts.TextToSpeech;
import android.util.Log;

import com.github.olga_yakovleva.rhvoice.*;

public final class CheckTTSData extends Activity implements FutureDataResult.Receiver
{
    private static final String TAG="RHVoiceCheckDataActivity";
    private FutureDataResult dataResult;

    private List<AndroidVoiceInfo> listVoices(String[] paths) throws RHVoiceException,IOException
    {
        TTSEngine engine=new TTSEngine("",getDir("config",0).getAbsolutePath(),paths);
        List<VoiceInfo> voices=engine.getVoices();
        engine.shutdown();
        List<AndroidVoiceInfo> result=new ArrayList(voices.size());
        for(VoiceInfo voice: voices)
            {
                AndroidVoiceInfo androidVoice=new AndroidVoiceInfo(voice);
                result.add(androidVoice);
            }
        return result;
    }

    private void reply(int resultCode,ArrayList<String> availableVoices,ArrayList<String> unavailableVoices)
    {
        final Intent intent=new Intent();
        if(availableVoices!=null)
            intent.putExtra(TextToSpeech.Engine.EXTRA_AVAILABLE_VOICES,availableVoices);
        if(unavailableVoices!=null)
            intent.putExtra(TextToSpeech.Engine.EXTRA_UNAVAILABLE_VOICES,unavailableVoices);
        setResult(resultCode,intent);
        finish();
    }

    public void onDataSuccess(String[] paths)
    {
        List<AndroidVoiceInfo> voices=null;
        try
            {
                voices=listVoices(paths);
            }
        catch(Exception e)
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Unable to list installed voices",e);
            }
        if(voices==null)
            reply(TextToSpeech.Engine.CHECK_VOICE_DATA_FAIL,null,null);
        else
            {
                ArrayList<String> availableVoices=new ArrayList<String>();
                ArrayList<String> unavailableVoices=new ArrayList<String>();
                final ArrayList<String> requestedVoices=getIntent().getStringArrayListExtra(TextToSpeech.Engine.EXTRA_CHECK_VOICE_DATA_FOR);
                if((requestedVoices==null)||requestedVoices.isEmpty())
                    {
                        for(AndroidVoiceInfo voice: voices)
                            {
                                availableVoices.add(voice.toString());
                            }
                    }
                else
                    {
                        LinkedHashSet<String> matchingVoices=new LinkedHashSet<String>();
                        for(String name: requestedVoices)
                            {
                                boolean found=false;
                                for(AndroidVoiceInfo voice: voices)
                                    {
                                        if(voice.matches(name))
                                            {
                                                matchingVoices.add(voice.toString());
                                                found=true;
                                            }
                                    }
                                if(!found)
                                    unavailableVoices.add(name);
                            }
                        availableVoices.addAll(matchingVoices);
                    }
                reply(TextToSpeech.Engine.CHECK_VOICE_DATA_PASS,availableVoices,unavailableVoices);
            }
    }

    public void onDataFailure()
    {
        reply(TextToSpeech.Engine.CHECK_VOICE_DATA_FAIL,null,null);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.check);
        dataResult=DataService.checkData(this,this);
    }

    @Override
    protected void onDestroy()
    {
        if(dataResult!=null)
            dataResult.unregisterReceiver();
        super.onDestroy();
    }
}

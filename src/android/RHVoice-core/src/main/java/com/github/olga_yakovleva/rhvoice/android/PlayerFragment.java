/* Copyright (C) 2018  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.media.MediaPlayer;
import android.speech.tts.TextToSpeech;
import android.content.Context;
import java.util.Map;
import java.util.HashMap;
import android.content.res.AssetFileDescriptor;
import java.io.IOException;
import android.util.Log;
import android.os.Handler;
import android.speech.tts.UtteranceProgressListener;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.os.Build;
import android.media.AudioFocusRequest;

public final class PlayerFragment extends Fragment
{
    private static final String TAG="RHVoice.PlayerFragment";

    private final MediaPlayer.OnCompletionListener playerDoneListener=new MediaPlayer.OnCompletionListener()
        {
            @Override
            public void onCompletion(MediaPlayer mp)
            {
                if(BuildConfig.DEBUG)
                    Log.v(TAG,"Playback completed");
                playerState.onStop();
            }
        };

    private final MediaPlayer.OnErrorListener playerErrorListener=new MediaPlayer.OnErrorListener()
        {
            @Override
            public boolean onError(MediaPlayer mp,int what,int extra)
            {
                if(BuildConfig.DEBUG)
                    Log.e(TAG,"Error: "+what+", "+extra);
                playerState.reset();
                return true;
}
        };

    private final TextToSpeech.OnInitListener ttsInitListener=new TextToSpeech.OnInitListener()
        {
            @Override
            public void onInit(int status)
            {
                ttsState.onInit(status);
}
        };

    private final UtteranceProgressListener ttsProgressListener=new UtteranceProgressListener()
        {
            @Override
            public void onStart(String uttId)
            {
}

            @Override
            public void onDone(String uttId)
            {
                ttsHandler.post(new TTSDoneEvent(uttId));
}

            @Override
            public void onError(String uttId)
            {
                if(BuildConfig.DEBUG)
                    Log.w(TAG,"TTS error in utt "+uttId);
                ttsHandler.post(new TTSErrorEvent(uttId));
}

            @Override
            public void onError(String uttId,int code)
            {
                onError(uttId);
}
        };

    private class AudioFocusListener implements AudioManager.OnAudioFocusChangeListener
    {
        @Override
        public void onAudioFocusChange(int change)
        {
            switch(change)
                {
                case AudioManager.AUDIOFOCUS_LOSS:
                    playerState.stop();
                    ttsState.stop();
                    break;
                case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT_CAN_DUCK:
                    playerState.pause();
                    ttsState.pause();
                    break;
                case AudioManager.AUDIOFOCUS_GAIN:
                    playerState.resume();
                    ttsState.resume();
                    break;
                default:
                    break;
}
}
}

    private AudioManager audioManager;
    private AudioAttributes audioAttrs;
    private final Map<String,Map<String,Integer>> resIdCache=new HashMap<String,Map<String,Integer>>();
    private VoicePack playerVoice;
    private MediaPlayer player;
    private PlayerState playerState=new UninitializedPlayerState();
    private VoicePack ttsVoice;
    private TextToSpeech tts;
    private long ttsUttId;
    private TTSState ttsState=new UninitializedTTSState();
    private Handler ttsHandler;
    private AudioFocusListener audioFocusListener;
    private AudioFocusRequest audioFocusRequest;

    private int doGetResId(String name,String type)
    {
        Context context=getActivity();
        if(BuildConfig.DEBUG)
            Log.v(TAG,"Looking for resource: name="+name+", type="+type);
        int id=context.getResources().getIdentifier(name,type,context.getPackageName());
        if(BuildConfig.DEBUG)
            {
                if(id==0)
                    Log.w(TAG,"Resource not found");
}
        return id;
    }

    private int getResId(String name,String type)
    {
        Map<String,Integer> typeCache=resIdCache.get(type);
        if(typeCache!=null)
            {
                Integer id0=typeCache.get(name);
                if(id0!=null)
                    return id0;
            }
        int id=doGetResId(name,type);
        if(typeCache==null)
            {
                typeCache=new HashMap<String,Integer>();
                resIdCache.put(type,typeCache);
            }
        typeCache.put(name,id);
        return id;
    }

    private int getDemoResId(VoicePack v)
    {
        String name="demo_"+v.getId();
        return getResId(name,"raw");
    }

    private int getTestResId(VoicePack v)
    {
        String name="test_"+v.getLanguage().getCode();
        return getResId(name,"string");
    }

    private void abandonAudioFocus()
    {
        if(audioFocusListener==null)
            return;
        if(Build.VERSION.SDK_INT>=Build.VERSION_CODES.O)
            {
                audioManager.abandonAudioFocusRequest(audioFocusRequest);
                audioFocusRequest=null;
            }
        else
            audioManager.abandonAudioFocus(audioFocusListener);
        audioFocusListener=null;
}

    private boolean requestAudioFocus()
    {
        if(audioFocusListener!=null)
            return true;
        final int dur=AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK;
        audioFocusListener=new AudioFocusListener();
        boolean hasFocus=false;
        int res=0;
        if(Build.VERSION.SDK_INT>=Build.VERSION_CODES.O)
            {
                AudioFocusRequest.Builder b=new AudioFocusRequest.Builder(dur);
                b.setAcceptsDelayedFocusGain(false);
                b.setAudioAttributes(audioAttrs);
                b.setOnAudioFocusChangeListener(audioFocusListener);
                b.setWillPauseWhenDucked(true);
                audioFocusRequest=b.build();
                res=audioManager.requestAudioFocus(audioFocusRequest);
}
        else
            {
                res=audioManager.requestAudioFocus(audioFocusListener,AudioManager.STREAM_MUSIC,dur);
}
        hasFocus=(res==AudioManager.AUDIOFOCUS_REQUEST_GRANTED);
        if(!hasFocus)
            {
                audioFocusListener=null;
                if(Build.VERSION.SDK_INT>=Build.VERSION_CODES.O)
                    audioFocusRequest=null;
}
        return hasFocus;
}

    private abstract class PlayerState
    {
        protected void refreshUI()
    {
        AvailableVoicesFragment frag=(AvailableVoicesFragment)(getActivity().getSupportFragmentManager().findFragmentByTag("voices"));
                if(frag!=null)
                    {
                        if(playerVoice==null)
                            throw new IllegalStateException();
                        frag.refresh(playerVoice,VoiceViewChange.PLAYING);
                    }
}

        public void stop()
        {
        }

        public void release()
        {
        }

        public void onStop()
        {
        }

        public boolean isPlaying()
        {
            return false;
        }

        protected void onOpen(VoicePack v)
        {
            playerVoice=v;
            if(!requestAudioFocus())
                {
                    playerState=new StoppedPlayerState();
                    return;
                }
            player.start();
            playerState=new PlayingPlayerState();
            refreshUI();
        }

        public abstract void play(VoicePack v);

        public boolean canPlay(VoicePack v)
        {
            return (getDemoResId(v)!=0);
}

        public boolean isPlaying(VoicePack v)
        {
            return (isPlaying()&&playerVoice==v);
}

        public void reset()
        {
}

        public void pause()
        {
}

        public void resume()
        {
}
    }

    private final class UninitializedPlayerState extends PlayerState
    {
        @Override
        public void play(VoicePack v)
        {
            player=new MediaPlayer();
            if(Build.VERSION.SDK_INT>=Build.VERSION_CODES.LOLLIPOP)
                player.setAudioAttributes(audioAttrs);
            player.setOnErrorListener(playerErrorListener);
            player.setOnCompletionListener(playerDoneListener);
            playerState=new ResetPlayerState();
            playerState.play(v);
        }
    }

    private abstract class CreatedPlayerState extends PlayerState
    {
        @Override
        public void release()
        {
            player.release();
            player=null;
            playerState=new UninitializedPlayerState();
        }

        @Override
        public void reset()
        {
            player.reset();
            playerState=new ResetPlayerState();
}
    }

    private class PlayingPlayerState extends CreatedPlayerState
    {
        @Override
        public void release()
        {
            stop();
            super.release();
        }

        @Override
        public void stop()
        {
            player.stop();
            onStop();
            try
                {
                    player.prepare();
}
            catch(IOException e)
                {
                    if(BuildConfig.DEBUG)
                        Log.e(TAG,"Error",e);
                    playerState.reset();
}
        }

        @Override
        public void onStop()
        {
            abandonAudioFocus();
            playerState=new StoppedPlayerState();
            refreshUI();
        }

        @Override
        public boolean isPlaying()
        {
            return true;
        }

        @Override
        public void play(VoicePack v)
        {
            stop();
            playerState.play(v);
        }

        @Override
        public void reset()
        {
            super.reset();
            refreshUI();
}

        @Override
        public void pause()
        {
            player.pause();
            playerState=new PausedPlayerState();
}
    }

    private final class PausedPlayerState extends PlayingPlayerState
    {
        @Override
        public void resume()
        {
            player.start();
            playerState=new PlayingPlayerState();
}
}

    private final class StoppedPlayerState extends CreatedPlayerState
    {
        @Override
        public void play(VoicePack v)
        {
            if(playerVoice==v)
                {
                    if(BuildConfig.DEBUG)
                        Log.v(TAG,"Request to play the same demo, just restarting");
                    onOpen(v);
                    return;
                }
            reset();
            playerState.play(v);
        }
    }

    private final class ResetPlayerState extends CreatedPlayerState
    {
        @Override
        public void play(VoicePack v)
        {
            int demoId=getDemoResId(v);
            if(demoId==0)
                return;
            AssetFileDescriptor afd=null;
            try
                {
                    afd=getActivity().getResources().openRawResourceFd(demoId);
                    player.setDataSource(afd.getFileDescriptor(),afd.getStartOffset(),afd.getLength());
                    player.prepare();
                    onOpen(v);
}
            catch(IOException e)
                {
                    if(BuildConfig.DEBUG)
                        Log.e(TAG,"Unable to set new data source",e);
                    if(afd!=null)
                        player.reset();
                    return;
}
            finally
                {
                    if(afd!=null)
                        {
                            try
                                {
                                    afd.close();
}
                            catch(IOException e)
                                {
}
}
}
        }
    }

    private abstract class TTSUttEvent implements Runnable
    {
        private final String eventUttId;

        public TTSUttEvent(String id)
        {
            eventUttId=id;
}

        abstract void onEvent();

        @Override
        public void run()
        {
            if(String.valueOf(ttsUttId).equals(eventUttId))
                onEvent();
}
}

    private class TTSDoneEvent extends TTSUttEvent
    {
        public TTSDoneEvent(String id)
        {
            super(id);
}

        @Override
        void onEvent()
        {
            ttsState.onStop();
}
}

    private class TTSErrorEvent extends TTSUttEvent
    {
        public TTSErrorEvent(String id)
        {
            super(id);
}

        @Override
        void onEvent()
        {
            ttsState.onStop();
}
}

    private abstract class TTSState
    {
        protected void refreshUI()
    {
        AvailableVoicesFragment frag=(AvailableVoicesFragment)(getActivity().getSupportFragmentManager().findFragmentByTag("voices"));
                if(frag!=null)
                    {
                        if(ttsVoice==null)
                            throw new IllegalStateException();
                        frag.refresh(ttsVoice,VoiceViewChange.PLAYING);
                    }
}


        public void release()
        {
}

        public void play(VoicePack v)
        {
}

        public void stop()
        {
}

        public void onInit(int status)
        {
}

        public void onStop()
        {
}

        public boolean canPlay(VoicePack v)
        {
            return (getTestResId(v)!=0);
}

        public boolean isPlaying()
        {
            return false;
}

        public boolean isPlaying(VoicePack v)
        {
            return (isPlaying()&&(ttsVoice==v));
}

        public void pause()
        {
}

        public void resume()
        {
}
}

    private final class UninitializedTTSState extends TTSState
    {
        @Override
        public void play(VoicePack v)
        {
            ttsVoice=v;
            ttsState=new InitializingToPlayTTSState();
            tts=new TextToSpeech(getActivity(),ttsInitListener,getActivity().getPackageName());
            refreshUI();
}
}

    private abstract class CreatedTTSState extends TTSState
    {
        @Override
        public void release()
        {
            tts.shutdown();
            tts=null;
            ttsState=new UninitializedTTSState();
}

        boolean doPlay(VoicePack v)
        {
            int resId=getTestResId(v);
            if(resId==0)
                return false;
            if(!requestAudioFocus())
                return false;
            ttsVoice=v;
            String msg=getString(resId);
            HashMap<String,String> params=new HashMap<String,String>();
            params.put(TextToSpeech.Engine.KEY_PARAM_UTTERANCE_ID,String.valueOf(ttsUttId));
            params.put(RHVoiceService.KEY_PARAM_TEST_VOICE,v.getName());
            int res=tts.speak(msg,TextToSpeech.QUEUE_FLUSH,params);
            if(res!=TextToSpeech.SUCCESS)
                {
                    abandonAudioFocus();
                    return false;
                }
            ttsState=new PlayingTTSState();
            return true;
}
}

    private class InitializingTTSState extends CreatedTTSState
    {
        protected void doOnInit()
        {
            tts.setOnUtteranceProgressListener(ttsProgressListener);
            ttsState=new InitializedTTSState();
}

        protected void doOnError()
        {
            release();
}

        @Override
        public void onInit(int status)
        {
            if(status==TextToSpeech.SUCCESS)
                doOnInit();
            else
                doOnError();
}

        @Override
        public void play(VoicePack v)
        {
            ttsVoice=v;
            ttsState=new InitializingToPlayTTSState();
            refreshUI();
}
}

    private final class InitializingToPlayTTSState extends InitializingTTSState
    {
        @Override
        protected void doOnInit()
        {
            super.doOnInit();
            ttsState.play(ttsVoice);
}

        @Override
        protected void doOnError()
        {
            super.doOnError();
            refreshUI();
}

        @Override
        public boolean isPlaying()
        {
            return true;
}

        @Override
        public void stop()
        {
            ttsState=new InitializingTTSState();
            refreshUI();
}

        @Override
        public void play(VoicePack v)
        {
            stop();
            ttsState.play(v);
}

        @Override
        public void release()
        {
            stop();
            super.release();
}
}

    private final class InitializedTTSState extends CreatedTTSState
    {
        @Override
        public void play(VoicePack v)
        {
            if(doPlay(v))
                refreshUI();
        }
}

    private class PlayingTTSState extends CreatedTTSState
    {
        @Override
        public boolean isPlaying()
        {
            return true;
}

        @Override
        public void play(VoicePack v)
        {
            stop();
            ttsState.play(v);
}

        @Override
        public void onStop()
        {
            ++ttsUttId;
            abandonAudioFocus();
            ttsState=new InitializedTTSState();
            refreshUI();
}

        @Override
        public void stop()
        {
            tts.stop();
            onStop();
}

        @Override
        public void release()
        {
            stop();
            super.release();
}

        @Override
        public void pause()
        {
            tts.stop();
            ttsState=new PausedTTSState();
}
}

    private final class PausedTTSState extends PlayingTTSState
    {
        @Override
        public void stop()
        {
            onStop();
}

        @Override
        public void resume()
        {
            ++ttsUttId;
            if(doPlay(ttsVoice))
                return;
            ttsState=new InitializedTTSState();
            refreshUI();
}
}

    @Override
    public void onCreate(Bundle state)
    {
        super.onCreate(state);
        ttsHandler=new Handler();
        audioManager=(AudioManager)(getActivity().getSystemService(Context.AUDIO_SERVICE));
        if(Build.VERSION.SDK_INT>=Build.VERSION_CODES.LOLLIPOP)
            audioAttrs=(new AudioAttributes.Builder()).setUsage(AudioAttributes.USAGE_MEDIA).setContentType(AudioAttributes.CONTENT_TYPE_SPEECH).build();
}

    @Override
    public void onStop()
    {
        super.onStop();
        playerState.release();
        ttsState.release();
    }

    public void play(VoicePack v)
    {
        stopPlayback();
        if(v.getEnabled(getActivity())&&v.isInstalled(getActivity()))
            ttsState.play(v);
        else
            playerState.play(v);
}

    public boolean canPlay(VoicePack v)
    {
        return (playerState.canPlay(v)&&ttsState.canPlay(v));
}

    public boolean isPlaying(VoicePack v)
    {
        return (playerState.isPlaying(v)||ttsState.isPlaying(v));
}

    public void stopPlayback()
    {
        playerState.stop();
        ttsState.stop();
}
}

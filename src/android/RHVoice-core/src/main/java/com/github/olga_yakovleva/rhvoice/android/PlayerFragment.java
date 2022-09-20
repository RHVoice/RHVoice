/* Copyright (C) 2018, 2021  Olga Yakovleva <olga@rhvoice.org> */

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
import android.content.res.AssetFileDescriptor;
import android.media.AudioFocusRequest;
import android.media.AudioManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.speech.tts.TextToSpeech;
import android.speech.tts.UtteranceProgressListener;
import android.util.Log;

import androidx.fragment.app.Fragment;

import com.google.android.exoplayer2.C;
import com.google.android.exoplayer2.MediaItem;
import com.google.android.exoplayer2.PlaybackException;
import com.google.android.exoplayer2.Player;
import com.google.android.exoplayer2.SimpleExoPlayer;
import com.google.android.exoplayer2.audio.AudioAttributes;

import java.io.IOException;
import java.util.HashMap;
import java.util.Map;

public final class PlayerFragment extends Fragment implements Player.Listener {
    private static final String TAG = "RHVoice.PlayerFragment";

    @Override
    public void onPlaybackStateChanged(int state) {
        switch (state) {
            case Player.STATE_IDLE:
                if (BuildConfig.DEBUG)
                    Log.v(TAG, "Audio player state changed to idle");
                playerState.onStop();
                playerState = new IdlePlayerState();
                break;
            case Player.STATE_ENDED:
                if (BuildConfig.DEBUG)
                    Log.v(TAG, "Audio player state changed to ended");
                playerState.onStop();
                break;
            default:
                break;
        }
    }

    @Override
    public void onPlayerError​(PlaybackException error) {
        if (BuildConfig.DEBUG)
            Log.e(TAG, "Audio playback error", error);
    }

    private final TextToSpeech.OnInitListener ttsInitListener = new TextToSpeech.OnInitListener() {
        @Override
        public void onInit(int status) {
            ttsState.onInit(status);
        }
    };

    private final UtteranceProgressListener ttsProgressListener = new UtteranceProgressListener() {
        @Override
        public void onStart(String uttId) {
        }

        @Override
        public void onDone(String uttId) {
            ttsHandler.post(new TTSDoneEvent(uttId));
        }

        @Override
        public void onError(String uttId) {
            if (BuildConfig.DEBUG)
                Log.w(TAG, "TTS error in utt " + uttId);
            ttsHandler.post(new TTSErrorEvent(uttId));
        }

        @Override
        public void onError(String uttId, int code) {
            onError(uttId);
        }
    };

    private class AudioFocusListener implements AudioManager.OnAudioFocusChangeListener {
        @Override
        public void onAudioFocusChange(int change) {
            switch (change) {
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
    private VoicePack playerVoice;
    private SimpleExoPlayer player;
    private PlayerState playerState = new UninitializedPlayerState();
    private VoicePack ttsVoice;
    private TextToSpeech tts;
    private long ttsUttId;
    private TTSState ttsState = new UninitializedTTSState();
    private Handler ttsHandler;
    private AudioFocusListener audioFocusListener;
    private AudioFocusRequest audioFocusRequest;

    private void abandonAudioFocus() {
        if (audioFocusListener == null)
            return;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            audioManager.abandonAudioFocusRequest(audioFocusRequest);
            audioFocusRequest = null;
        } else
            audioManager.abandonAudioFocus(audioFocusListener);
        audioFocusListener = null;
    }

    private boolean requestAudioFocus() {
        if (audioFocusListener != null)
            return true;
        final int dur = AudioManager.AUDIOFOCUS_GAIN_TRANSIENT_MAY_DUCK;
        audioFocusListener = new AudioFocusListener();
        boolean hasFocus = false;
        int res = 0;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            AudioFocusRequest.Builder b = new AudioFocusRequest.Builder(dur);
            b.setAcceptsDelayedFocusGain(false);
            b.setAudioAttributes(audioAttrs.getAudioAttributesV21());
            b.setOnAudioFocusChangeListener(audioFocusListener);
            b.setWillPauseWhenDucked(true);
            audioFocusRequest = b.build();
            res = audioManager.requestAudioFocus(audioFocusRequest);
        } else {
            res = audioManager.requestAudioFocus(audioFocusListener, AudioManager.STREAM_MUSIC, dur);
        }
        hasFocus = (res == AudioManager.AUDIOFOCUS_REQUEST_GRANTED);
        if (!hasFocus) {
            audioFocusListener = null;
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
                audioFocusRequest = null;
        }
        return hasFocus;
    }

    private abstract class PlayerState {
        protected void refreshUI() {
            AvailableVoicesFragment frag = (AvailableVoicesFragment) (getActivity().getSupportFragmentManager().findFragmentByTag("voices"));
            if (frag != null) {
                if (playerVoice == null)
                    throw new IllegalStateException();
                frag.refresh(playerVoice, VoiceViewChange.PLAYING);
            }
        }

        public void stop() {
        }

        public void release() {
        }

        public void onStop() {
        }

        public boolean isPlaying() {
            return false;
        }

        protected void onOpen() {
            if (!requestAudioFocus()) {
                playerState = new StoppedPlayerState();
                return;
            }
            player.play();
            playerState = new PlayingPlayerState();
            refreshUI();
        }

        protected boolean setMediaItem(VoicePack v) {
            String url = v.getDemoUrl();
            if (url.isEmpty())
                return false;
            if (BuildConfig.DEBUG)
                Log.v(TAG, "Setting media item for " + v.getId() + ": " + url);
            MediaItem item = MediaItem.fromUri(url);
            if (item == null)
                return false;
            player.setMediaItem(item);
            player.prepare();
            return true;
        }

        public abstract void play(VoicePack v);

        public boolean canPlay(VoicePack v) {
            return (!v.getDemoUrl().isEmpty());
        }

        public boolean isPlaying(VoicePack v) {
            return (isPlaying() && playerVoice.getId().equals(v.getId()));
        }

        public void pause() {
        }

        public void resume() {
        }
    }

    private final class UninitializedPlayerState extends PlayerState {
        @Override
        public void play(VoicePack v) {
            player = new SimpleExoPlayer.Builder(requireActivity()).setAudioAttributes(audioAttrs, false).build();
            player.addListener(PlayerFragment.this);
            playerState = new IdlePlayerState();
            playerState.play(v);
        }
    }

    private abstract class CreatedPlayerState extends PlayerState {
        @Override
        public void release() {
            player.release();
            player = null;
            playerState = new UninitializedPlayerState();
        }
    }

    private class PlayingPlayerState extends CreatedPlayerState {
        @Override
        public void release() {
            player.stop();
            onStop();
            playerState.release();
        }

        @Override
        public void stop() {
            player.pause();
            onStop();
        }

        @Override
        public void onStop() {
            abandonAudioFocus();
            playerState = new StoppedPlayerState();
            refreshUI();
        }

        @Override
        public boolean isPlaying() {
            return true;
        }

        @Override
        public void play(VoicePack v) {
            stop();
            playerState.play(v);
        }

        @Override
        public void pause() {
            player.pause();
            playerState = new PausedPlayerState();
        }
    }

    private final class PausedPlayerState extends PlayingPlayerState {
        @Override
        public void resume() {
            player.play();
            playerState = new PlayingPlayerState();
        }
    }

    private final class StoppedPlayerState extends CreatedPlayerState {
        @Override
        public void play(VoicePack v) {
            if (playerVoice == v) {
                if (BuildConfig.DEBUG)
                    Log.v(TAG, "Request to play the same demo, just restarting");
                player.seekToDefaultPosition();
                onOpen();
                return;
            }
            if (!setMediaItem(v))
                return;
            playerVoice = v;
            onOpen();
        }
    }

    private final class IdlePlayerState extends CreatedPlayerState {
        @Override
        public void play(VoicePack v) {
            if (!setMediaItem(v))
                return;
            playerVoice = v;
            onOpen();
        }
    }

    private abstract class TTSUttEvent implements Runnable {
        private final String eventUttId;

        public TTSUttEvent(String id) {
            eventUttId = id;
        }

        abstract void onEvent();

        @Override
        public void run() {
            if (String.valueOf(ttsUttId).equals(eventUttId))
                onEvent();
        }
    }

    private class TTSDoneEvent extends TTSUttEvent {
        public TTSDoneEvent(String id) {
            super(id);
        }

        @Override
        void onEvent() {
            ttsState.onStop();
        }
    }

    private class TTSErrorEvent extends TTSUttEvent {
        public TTSErrorEvent(String id) {
            super(id);
        }

        @Override
        void onEvent() {
            ttsState.onStop();
        }
    }

    private abstract class TTSState {
        protected void refreshUI() {
            AvailableVoicesFragment frag = (AvailableVoicesFragment) (getActivity().getSupportFragmentManager().findFragmentByTag("voices"));
            if (frag != null) {
                if (ttsVoice == null)
                    throw new IllegalStateException();
                frag.refresh(ttsVoice, VoiceViewChange.PLAYING);
            }
        }


        public void release() {
        }

        public void play(VoicePack v) {
        }

        public void stop() {
        }

        public void onInit(int status) {
        }

        public void onStop() {
        }

        public boolean canPlay(VoicePack v) {
            return (!v.getTestMessage().isEmpty());
        }

        public boolean isPlaying() {
            return false;
        }

        public boolean isPlaying(VoicePack v) {
            return (isPlaying() && (ttsVoice.getId().equals(v.getId())));
        }

        public void pause() {
        }

        public void resume() {
        }
    }

    private final class UninitializedTTSState extends TTSState {
        @Override
        public void play(VoicePack v) {
            ttsVoice = v;
            ttsState = new InitializingToPlayTTSState();
            tts = new TextToSpeech(getActivity(), ttsInitListener, getActivity().getPackageName());
            refreshUI();
        }
    }

    private abstract class CreatedTTSState extends TTSState {
        @Override
        public void release() {
            tts.shutdown();
            tts = null;
            ttsState = new UninitializedTTSState();
        }

        boolean doPlay(VoicePack v) {
            final String msg = v.getTestMessage();
            if (msg.isEmpty())
                return false;
            if (!requestAudioFocus())
                return false;
            ttsVoice = v;
            HashMap<String, String> params = new HashMap<String, String>();
            params.put(TextToSpeech.Engine.KEY_PARAM_UTTERANCE_ID, String.valueOf(ttsUttId));
            params.put(RHVoiceService.KEY_PARAM_TEST_VOICE, v.getName());
            int res = tts.speak(msg, TextToSpeech.QUEUE_FLUSH, params);
            if (res != TextToSpeech.SUCCESS) {
                abandonAudioFocus();
                return false;
            }
            ttsState = new PlayingTTSState();
            return true;
        }
    }

    private class InitializingTTSState extends CreatedTTSState {
        protected void doOnInit() {
            tts.setOnUtteranceProgressListener(ttsProgressListener);
            ttsState = new InitializedTTSState();
        }

        protected void doOnError() {
            release();
        }

        @Override
        public void onInit(int status) {
            if (status == TextToSpeech.SUCCESS)
                doOnInit();
            else
                doOnError();
        }

        @Override
        public void play(VoicePack v) {
            ttsVoice = v;
            ttsState = new InitializingToPlayTTSState();
            refreshUI();
        }
    }

    private final class InitializingToPlayTTSState extends InitializingTTSState {
        @Override
        protected void doOnInit() {
            super.doOnInit();
            ttsState.play(ttsVoice);
        }

        @Override
        protected void doOnError() {
            super.doOnError();
            refreshUI();
        }

        @Override
        public boolean isPlaying() {
            return true;
        }

        @Override
        public void stop() {
            ttsState = new InitializingTTSState();
            refreshUI();
        }

        @Override
        public void play(VoicePack v) {
            stop();
            ttsState.play(v);
        }

        @Override
        public void release() {
            stop();
            super.release();
        }
    }

    private final class InitializedTTSState extends CreatedTTSState {
        @Override
        public void play(VoicePack v) {
            if (doPlay(v))
                refreshUI();
        }
    }

    private class PlayingTTSState extends CreatedTTSState {
        @Override
        public boolean isPlaying() {
            return true;
        }

        @Override
        public void play(VoicePack v) {
            stop();
            ttsState.play(v);
        }

        @Override
        public void onStop() {
            ++ttsUttId;
            abandonAudioFocus();
            ttsState = new InitializedTTSState();
            refreshUI();
        }

        @Override
        public void stop() {
            tts.stop();
            onStop();
        }

        @Override
        public void release() {
            stop();
            super.release();
        }

        @Override
        public void pause() {
            tts.stop();
            ttsState = new PausedTTSState();
        }
    }

    private final class PausedTTSState extends PlayingTTSState {
        @Override
        public void stop() {
            onStop();
        }

        @Override
        public void resume() {
            ++ttsUttId;
            if (doPlay(ttsVoice))
                return;
            ttsState = new InitializedTTSState();
            refreshUI();
        }
    }

    @Override
    public void onCreate(Bundle state) {
        super.onCreate(state);
        ttsHandler = new Handler();
        audioManager = (AudioManager) (getActivity().getSystemService(Context.AUDIO_SERVICE));
        audioAttrs = (new AudioAttributes.Builder()).setUsage(C.USAGE_MEDIA).setContentType(C.CONTENT_TYPE_SPEECH).build();
    }

    @Override
    public void onStop() {
        super.onStop();
        playerState.release();
        ttsState.release();
    }

    public void play(VoicePack v) {
        stopPlayback();
        if (v.getEnabled(getActivity()) && v.isInstalled(getActivity()))
            ttsState.play(v);
        else
            playerState.play(v);
    }

    public boolean canPlay(VoicePack v) {
        return (playerState.canPlay(v) && ttsState.canPlay(v));
    }

    public boolean isPlaying(VoicePack v) {
        return (playerState.isPlaying(v) || ttsState.isPlaying(v));
    }

    public void stopPlayback() {
        playerState.stop();
        ttsState.stop();
    }
}

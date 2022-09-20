/* Copyright (C) 2013, 2014, 2016, 2017, 2021, 2022  Olga Yakovleva <olga@rhvoice.org> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <https://www.gnu.org/licenses/>. */

package com.github.olga_yakovleva.rhvoice;

import android.text.TextUtils;

import java.util.Arrays;
import java.util.List;

public final class TTSEngine {
    private long data;

    private static native void onClassInit();

    private native void onInit(String data_path, String config_path, String[] resource_paths, String pkgPath, Logger logger) throws RHVoiceException;

    private native void onShutdown();

    private native VoiceInfo[] doGetVoices();

    private native void doSpeak(String text, SynthesisParameters params, TTSClient client) throws RHVoiceException;

    private native boolean doConfigure(String key, String value);

    private native String doGetCachedPackageDir();

    private native String doGetPackageDirFromServer();

    static {
        System.loadLibrary("RHVoice_jni");
        onClassInit();
    }

    public TTSEngine(String data_path, String config_path, String[] resource_paths, String pkgPath, Logger logger) throws RHVoiceException {
        onInit(data_path, config_path, resource_paths, pkgPath, logger);
    }

    public TTSEngine(String data_path, String config_path, List<String> resource_paths, String pkgPath, Logger logger) throws RHVoiceException {
        this(data_path, config_path, resource_paths.toArray(new String[resource_paths.size()]), pkgPath, logger);
    }

    public TTSEngine() throws RHVoiceException {
        this("", "", new String[0], "", null);
    }

    public void shutdown() {
        onShutdown();
    }

    public List<VoiceInfo> getVoices() {
        return Arrays.asList(doGetVoices());
    }

    public void speak(String text, SynthesisParameters params, TTSClient client) throws RHVoiceException {
        if (params.getVoiceProfile() == null)
            throw new RHVoiceException("Voice not set");
        doSpeak(text, params, client);
    }

    public boolean configure(String key, String value) {
        if (TextUtils.isEmpty(key))
            return false;
        if (TextUtils.isEmpty(value))
            return false;
        return doConfigure(key, value);
    }

    public String getCachedPackageDir() {
        return doGetCachedPackageDir();
    }

    public String getPackageDirFromServer() {
        return doGetPackageDirFromServer();
    }
}

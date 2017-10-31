/* Copyright (C) 2013, 2016  Olga Yakovleva <yakovleva.o.v@gmail.com> */

/* This program is free software: you can redistribute it and/or modify */
/* it under the terms of the GNU Lesser General Public License as published by */
/* the Free Software Foundation, either version 2.1 of the License, or */
/* (at your option) any later version. */

/* This program is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the */
/* GNU Lesser General Public License for more details. */

/* You should have received a copy of the GNU Lesser General Public License */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */

package com.github.olga_yakovleva.rhvoice.android;

import android.app.IntentService;
import android.content.Intent;
import android.speech.tts.TextToSpeech;
import android.support.v4.content.LocalBroadcastManager;

public final class DataService extends IntentService
{
    public static final String ACTION_DATA_STATE_CHANGED="com.github.olga_yakovleva.rhvoice.android.action.data_state_changed";
    public static final String ACTION_DATA_SYNC_FINISHED="com.github.olga_yakovleva.rhvoice.android.action.data_sync_finished";

    public DataService()
    {
        super("DataService");
    }

    @Override
    protected void onHandleIntent(final Intent intent)
    {
        DataSyncAction a=new DataSyncAction(this);
        a.run();
    }
}

/* Copyright (C) 2016  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.support.v4.content.LocalBroadcastManager;

public final class DataInstallationFragment extends UIFragment
{
    private final BroadcastReceiver receiver=new BroadcastReceiver()
        {
            @Override
            public void onReceive(Context context,Intent intent)
            {
                SyncCompletionListener listener=(SyncCompletionListener)getActivity();
                listener.onSyncFinished();
}
        };

    public interface SyncCompletionListener
    {
        public void onSyncFinished();
}

    @Override
    protected int getLayout()
    {
        return R.layout.progress;
}

    @Override
    public void onResume()
    {
        super.onResume();
        Activity activity=getActivity();
        LocalBroadcastManager.getInstance(activity).registerReceiver(receiver,new IntentFilter(DataService.ACTION_DATA_SYNC_FINISHED));
        activity.startService(new Intent(activity,DataService.class));
}

    @Override
    public void onPause()
    {
        super.onPause();
        LocalBroadcastManager.getInstance(getActivity()).unregisterReceiver(receiver);
}
}

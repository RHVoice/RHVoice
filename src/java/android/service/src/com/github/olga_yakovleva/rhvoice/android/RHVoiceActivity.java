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


import android.app.Activity;
import android.os.Bundle;
import android.content.Intent;

public final class RHVoiceActivity extends Activity implements FutureDataResult.Receiver
{
    static private final String TAG="RHVoiceInstallDataActivity";
    private FutureDataResult dataResult;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        dataResult=DataService.prepareData(this,this);
    }

    @Override
    protected void onDestroy()
    {
        if(dataResult!=null)
            dataResult.unregisterReceiver();
        super.onDestroy();
    }

    public void onDataSuccess(String[] paths)
    {
        final Intent intent=new Intent(this,SettingsActivity.class);
        intent.putExtra("paths",paths);
        startActivity(intent);
        setResult(RESULT_OK);
        finish();
    }

    public void onDataFailure()
    {
        setResult(RESULT_CANCELED);
        finish();
    }
}

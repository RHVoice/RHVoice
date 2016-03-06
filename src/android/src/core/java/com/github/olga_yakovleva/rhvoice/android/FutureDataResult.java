package com.github.olga_yakovleva.rhvoice.android;


import android.os.Bundle;
import android.os.Handler;
import android.os.ResultReceiver;

public final class FutureDataResult extends ResultReceiver 
{
    public interface Receiver
    {
        public void onDataSuccess(String[] paths);
        public void onDataFailure();
    }

    private Receiver receiver;

    public FutureDataResult()
    {
        super(new Handler());
    }

    public void setReceiver(Receiver receiver)
    {
        this.receiver=receiver;
    }

    public void unregisterReceiver()
    {
        receiver=null;
    }

    @Override
    protected void onReceiveResult(int code,Bundle data)
    {
        if(receiver!=null)
            {
                if(code==0)
                    receiver.onDataSuccess(data.getStringArray("paths"));
                else
                    receiver.onDataFailure();
            }
    }
}

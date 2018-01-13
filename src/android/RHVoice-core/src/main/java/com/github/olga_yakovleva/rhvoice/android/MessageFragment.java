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

import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

public final class MessageFragment extends UIFragment
{
    public static final String ARG_MESSAGE_ID="message_id";

    @Override
    protected int getLayout()
    {
        return R.layout.message;
    }

    @Override
    protected void setViewContent(View view)
    {
        Bundle args=getArguments();
        int messageId=args.getInt(ARG_MESSAGE_ID);
        String messageString=getString(messageId);
        TextView textView=(TextView)(view.findViewById(R.id.text));
        textView.setText(messageString);
}

    public static MessageFragment newInstance(int messageId)
    {
        Bundle args=new Bundle();
        args.putInt(ARG_MESSAGE_ID,messageId);
        MessageFragment frag=new MessageFragment();
        frag.setArguments(args);
        return frag;
}
}

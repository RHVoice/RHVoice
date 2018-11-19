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

import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.TextView;

public final class VoiceViewHolder extends RecyclerView.ViewHolder
{
    public final TextView nameView;
    public final ProgressBar progressBar;
    public final ImageButton actionButton;
    public final ImageButton playButton;

    public VoiceViewHolder(View v)
    {
        super(v);
        nameView=v.findViewById(R.id.voice);
        progressBar=v.findViewById(R.id.progress);
        actionButton=v.findViewById(R.id.action);
        playButton=v.findViewById(R.id.play);
}
}

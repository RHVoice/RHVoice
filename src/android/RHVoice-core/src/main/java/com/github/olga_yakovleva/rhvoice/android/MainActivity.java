/* Copyright (C) 2017, 2018, 2019  Olga Yakovleva <yakovleva.o.v@gmail.com> */

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

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;

public final class MainActivity extends AppCompatActivity implements AvailableLanguagesFragment.Listener, AvailableVoicesFragment.Listener, ConfirmVoiceRemovalDialogFragment.Listener {
    private DataManager dm;

    @Override
    protected void onCreate(Bundle state) {
        super.onCreate(state);
        dm = new DataManager();
        setContentView(R.layout.frame);
        Repository.get().getPackageDirectoryLiveData().observe(this, this::onPackageDirectory);
        if (state == null)
            getSupportFragmentManager().beginTransaction().replace(R.id.frame, new AvailableLanguagesFragment(), "languages").add(new PlayerFragment(), "player").commit();
    }

    private void onPackageDirectory(PackageDirectory dir) {
        dm.setPackageDirectory(dir);
        dm.scheduleSync(this, false);
    }

    public void onAccentSelected(VoiceAccent accent) {
        Bundle args = new Bundle();
        args.putString(AvailableVoicesFragment.ARG_LANGUAGE, accent.getLanguage().getId());
        args.putString(AvailableVoicesFragment.ARG_COUNTRY, accent.getTag().country3);
        args.putString(AvailableVoicesFragment.ARG_VARIANT, accent.getTag().variant);
        AvailableVoicesFragment frag = new AvailableVoicesFragment();
        frag.setArguments(args);
        getSupportFragmentManager().beginTransaction().replace(R.id.frame, frag, "voices").addToBackStack(null).commit();
    }

    public void onVoiceSelected(VoicePack voice, boolean state) {
        if (state || !voice.isInstalled(this)) {
            voice.setEnabled(this, state);
            AvailableVoicesFragment frag = (AvailableVoicesFragment) (getSupportFragmentManager().findFragmentByTag("voices"));
            if (frag != null)
                frag.refresh(voice, VoiceViewChange.INSTALLED);
        } else {
            ConfirmVoiceRemovalDialogFragment.show(this, voice);
        }
    }

    public void onConfirmVoiceRemovalResponse(VoicePack voice, boolean response) {
        if (response) {
            voice.setEnabled(this, false);
            AvailableVoicesFragment frag = (AvailableVoicesFragment) (getSupportFragmentManager().findFragmentByTag("voices"));
            if (frag != null)
                frag.refresh(voice, VoiceViewChange.INSTALLED);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main, menu);
        return super.onCreateOptionsMenu(menu);
    }

    private void showSettings() {
        Intent intent = new Intent(this, SettingsActivity.class);
        startActivity(intent);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.settings:
                showSettings();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    @Override
    public void onStart() {
        super.onStart();
        Repository.get().refresh();
    }
}

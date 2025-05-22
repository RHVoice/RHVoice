package com.github.olga_yakovleva.rhvoice.android;

import android.os.Bundle;
import android.view.View;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

public class SettingsFragment extends ToolbarFragment {
    public static final String NAME = "settings";

    @Override
    public void onViewReady(@NonNull View view, @Nullable Bundle savedInstanceState) {
        toolbar.getMenu().clear();
        if (savedInstanceState == null)
            getChildFragmentManager().beginTransaction().replace(R.id.toolbar_frame, new SettingsListFragment(), SettingsListFragment.NAME).commit();
    }
}

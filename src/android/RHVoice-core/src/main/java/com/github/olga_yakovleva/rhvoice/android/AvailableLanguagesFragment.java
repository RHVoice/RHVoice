/* Copyright (C) 2017, 2018, 2021  Olga Yakovleva <olga@rhvoice.org> */

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

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import android.os.Bundle;

import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.view.View;

import com.google.android.material.divider.MaterialDividerItemDecoration;

public final class AvailableLanguagesFragment extends ToolbarFragment {
    private LanguageListAdapter adapter;
    private DataManager dm;

    public interface Listener {
        public void onAccentSelected(VoiceAccent accent);
    }

    @Override
    public void onViewReady(@NonNull View view, @Nullable Bundle savedInstanceState) {
        RecyclerView recyclerView = (RecyclerView) replaceFrame(view, R.layout.list);
        if (recyclerView == null) {
            return;
        }

        recyclerView.setHasFixedSize(true);
        recyclerView.setLayoutManager(new LinearLayoutManager(requireActivity()));
        recyclerView.addItemDecoration(new MaterialDividerItemDecoration(requireContext(), MaterialDividerItemDecoration.VERTICAL));
        adapter = new LanguageListAdapter((MainActivity) requireActivity());
        recyclerView.setAdapter(adapter);
        InsetUtil.setInsets(recyclerView, (left, top, right, bottom) -> recyclerView.setPadding(left, 0, right, bottom));

        toolbar.setSubtitle(R.string.languages);
    }

    @Override
    public void onActivityCreated(Bundle state) {
        super.onActivityCreated(state);
        dm = new DataManager();
        Repository.get().getPackageDirectoryLiveData().observe(getViewLifecycleOwner(), this::onPackageDirectory);
    }

    private void onPackageDirectory(PackageDirectory dir) {
        dm.setPackageDirectory(dir);
        adapter.setItems(dm.getAccents());
    }
}

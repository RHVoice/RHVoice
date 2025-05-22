package com.github.olga_yakovleva.rhvoice.android;

import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.LayoutRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.google.android.material.appbar.MaterialToolbar;

public abstract class ToolbarFragment extends Fragment {
    protected MaterialToolbar toolbar;

    @Nullable
    @Override
    public final View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.toolbar_fragment, container, false);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        toolbar = view.findViewById(R.id.toolbar);
        toolbar.setTitle(requireActivity().getTitle());
        toolbar.inflateMenu(R.menu.main);
        toolbar.setOnMenuItemClickListener(item -> {
            if (item.getItemId() == R.id.settings) {
                SettingsActivity.show(requireContext());
                return true;
            }
            return false;
        });
        InsetUtil.setInsets(toolbar, (left, top, right, bottom) -> toolbar.setPadding(left, 0, right, 0));
        onViewReady(view, savedInstanceState);
    }

    public abstract void onViewReady(@NonNull View view, @Nullable Bundle savedInstanceState);

    /**
     * Replaces the {@link android.widget.FrameLayout} with a new layout.
     * @param view Parent view.
     * @param layoutId New layout to inflate.
     * @return The newly inflated {@link View} or null if the frame could not be found.
     */
    @Nullable
    public final View replaceFrame(@NonNull View view, @LayoutRes int layoutId) {
        View frame = view.findViewById(R.id.toolbar_frame);
        if (frame == null)
            return null;

        ViewGroup parent = (ViewGroup) frame.getParent();
        int index = parent.indexOfChild(frame);
        parent.removeViewAt(index);
        View newView = LayoutInflater.from(view.getContext()).inflate(layoutId, parent, false);
        parent.addView(newView, index);
        return newView;
    }
}

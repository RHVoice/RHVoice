package com.github.olga_yakovleva.rhvoice.android;

import android.view.View;

import androidx.annotation.NonNull;
import androidx.core.graphics.Insets;
import androidx.core.view.OnApplyWindowInsetsListener;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

public class InsetUtil {
    public interface InsetsListener {
        void apply(int left, int top, int right, int bottom);
    }

    /**
     * Abstraction on {@link ViewCompat#setOnApplyWindowInsetsListener(View, OnApplyWindowInsetsListener)}
     * that takes the max inset value from the system and display cutout insets and provides their
     * values through the {@link InsetsListener#apply(int, int, int, int)} interface method.
     * @param view The view that you would like to add an inset listener to.
     * @param listener The listener that applies insets to the view.
     */
    public static void setInsets(View view, InsetsListener listener) {
        ViewCompat.setOnApplyWindowInsetsListener(view, new OnApplyWindowInsetsListener() {
            @NonNull
            @Override
            public WindowInsetsCompat onApplyWindowInsets(@NonNull View v, @NonNull WindowInsetsCompat insets) {
                Insets systemBarInsets = insets.getInsets(WindowInsetsCompat.Type.systemBars());
                Insets displayCutoutInsets = insets.getInsets(WindowInsetsCompat.Type.displayCutout());

                int left = Integer.max(systemBarInsets.left, displayCutoutInsets.left);
                int top = Integer.max(systemBarInsets.top, displayCutoutInsets.top);
                int right = Integer.max(systemBarInsets.right, displayCutoutInsets.right);
                int bottom = Integer.max(systemBarInsets.bottom, displayCutoutInsets.bottom);

                listener.apply(left, top, right, bottom);

                return insets;
            }
        });
    }
}

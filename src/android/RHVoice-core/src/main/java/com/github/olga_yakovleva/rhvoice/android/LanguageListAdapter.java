package com.github.olga_yakovleva.rhvoice.android;

import android.annotation.SuppressLint;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.google.common.collect.Ordering;

import java.util.ArrayList;
import java.util.List;

public class LanguageListAdapter extends RecyclerView.Adapter<LanguageListAdapter.LanguageViewHolder> {
    private final MainActivity activity;

    private final List<VoiceAccent> items = new ArrayList<>();

    public LanguageListAdapter(MainActivity activity) {
        this.activity = activity;
    }

    @NonNull
    @Override
    public LanguageViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        return new LanguageViewHolder(LayoutInflater.from(activity).inflate(R.layout.text_list_item, parent, false));
    }

    @Override
    public void onBindViewHolder(@NonNull LanguageViewHolder holder, int position) {
        VoiceAccent item = items.get(position);
        holder.textView.setText(item.getDisplayName());
        holder.itemView.setOnClickListener(v -> activity.onAccentSelected(item));
    }

    @Override
    public int getItemCount() {
        return items.size();
    }

    @SuppressLint("NotifyDataSetChanged")
    public void setItems(List<VoiceAccent> items) {
        this.items.clear();
        this.items.addAll(items);
        this.items.sort(Ordering.usingToString());
        notifyDataSetChanged();
    }

    public static class LanguageViewHolder extends RecyclerView.ViewHolder {
        public final TextView textView;
        public LanguageViewHolder(@NonNull View itemView) {
            super(itemView);
            textView = itemView.findViewById(R.id.text);
        }
    }
}

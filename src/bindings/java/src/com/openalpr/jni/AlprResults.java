package com.openalpr.jni;

import com.openalpr.jni.json.JSONArray;
import com.openalpr.jni.json.JSONException;
import com.openalpr.jni.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

public class AlprResults {
    private final long epoch_time;
    private final int img_width;
    private final int img_height;
    private final float total_processing_time_ms;

    private List<AlprPlateResult> plates;

    private List<AlprRegionOfInterest> regionsOfInterest;

    AlprResults(String json) throws JSONException
    {
        JSONObject jobj = new JSONObject(json);

        epoch_time = jobj.getLong("epoch_time");
        img_width = jobj.getInt("img_width");
        img_height = jobj.getInt("img_height");
        total_processing_time_ms = (float) jobj.getDouble("processing_time_ms");

        JSONArray resultsArray = jobj.getJSONArray("results");
        plates = new ArrayList<AlprPlateResult>(resultsArray.length());
        for (int i = 0; i < resultsArray.length(); i++)
        {
            JSONObject plateObj = (JSONObject) resultsArray.get(i);
            AlprPlateResult result = new AlprPlateResult(plateObj);
            plates.add(result);
        }

        JSONArray roisArray = jobj.getJSONArray("regions_of_interest");
        regionsOfInterest = new ArrayList<AlprRegionOfInterest>(roisArray.length());
        for (int i = 0; i < roisArray.length(); i++)
        {
            JSONObject roiObj = (JSONObject) roisArray.get(i);
            AlprRegionOfInterest roi = new AlprRegionOfInterest(roiObj);
            regionsOfInterest.add(roi);
        }
    }

    public long getEpochTime() {
        return epoch_time;
    }

    public int getImgWidth() {
        return img_width;
    }

    public int getImgHeight() {
        return img_height;
    }

    public float getTotalProcessingTimeMs() {
        return total_processing_time_ms;
    }

    public List<AlprPlateResult> getPlates() {
        return plates;
    }

    public List<AlprRegionOfInterest> getRegionsOfInterest() {
        return regionsOfInterest;
    }
}

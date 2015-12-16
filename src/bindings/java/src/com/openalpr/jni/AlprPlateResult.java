package com.openalpr.jni;


import com.openalpr.jni.json.JSONArray;
import com.openalpr.jni.json.JSONException;
import com.openalpr.jni.json.JSONObject;

import java.util.ArrayList;
import java.util.List;

public class AlprPlateResult {
    // The number requested is always >= the topNPlates count
    private final int requested_topn;

    // the best plate is the topNPlate with the highest confidence
    private final AlprPlate bestPlate;

    // A list of possible plate number permutations
    private List<AlprPlate> topNPlates;

    // The processing time for this plate
    private final float processing_time_ms;

    // the X/Y coordinates of the corners of the plate (clock-wise from top-left)
    private List<AlprCoordinate> plate_points;

    // The index of the plate if there were multiple plates returned
    private final int plate_index;

    // When region detection is enabled, this returns the region.  Region detection is experimental
    private final int regionConfidence;
    private final String region;

    AlprPlateResult(JSONObject plateResult) throws JSONException
    {
        requested_topn = plateResult.getInt("requested_topn");

        JSONArray candidatesArray = plateResult.getJSONArray("candidates");

        if (candidatesArray.length() > 0)
            bestPlate = new AlprPlate((JSONObject) candidatesArray.get(0));
        else
            bestPlate = null;

        topNPlates = new ArrayList<AlprPlate>(candidatesArray.length());
        for (int i = 0; i < candidatesArray.length(); i++)
        {
            JSONObject candidateObj = (JSONObject) candidatesArray.get(i);
            AlprPlate newPlate = new AlprPlate(candidateObj);
            topNPlates.add(newPlate);
        }

        JSONArray coordinatesArray = plateResult.getJSONArray("coordinates");
        plate_points = new ArrayList<AlprCoordinate>(coordinatesArray.length());
        for (int i = 0; i < coordinatesArray.length(); i++)
        {
            JSONObject coordinateObj = (JSONObject) coordinatesArray.get(i);
            AlprCoordinate coordinate = new AlprCoordinate(coordinateObj);
            plate_points.add(coordinate);
        }

        processing_time_ms = (float) plateResult.getDouble("processing_time_ms");
        plate_index = plateResult.getInt("plate_index");

        regionConfidence = plateResult.getInt("region_confidence");
        region = plateResult.getString("region");

    }

    public int getRequestedTopn() {
        return requested_topn;
    }

    public AlprPlate getBestPlate() {
        return bestPlate;
    }

    public List<AlprPlate> getTopNPlates() {
        return topNPlates;
    }

    public float getProcessingTimeMs() {
        return processing_time_ms;
    }

    public List<AlprCoordinate> getPlatePoints() {
        return plate_points;
    }

    public int getPlateIndex() {
        return plate_index;
    }

    public int getRegionConfidence() {
        return regionConfidence;
    }

    public String getRegion() {
        return region;
    }
}

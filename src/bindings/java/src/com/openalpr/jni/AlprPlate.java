package com.openalpr.jni;


import com.openalpr.jni.json.JSONException;
import com.openalpr.jni.json.JSONObject;

public class AlprPlate {
    private final String characters;
    private final float overall_confidence;
    private final boolean matches_template;

    AlprPlate(JSONObject plateObj) throws JSONException
    {
        characters = plateObj.getString("plate");
        overall_confidence = (float) plateObj.getDouble("confidence");
        matches_template = plateObj.getInt("matches_template") != 0;
    }

    public String getCharacters() {
        return characters;
    }

    public float getOverallConfidence() {
        return overall_confidence;
    }

    public boolean isMatchesTemplate() {
        return matches_template;
    }
}

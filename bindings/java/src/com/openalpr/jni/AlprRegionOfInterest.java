package com.openalpr.jni;

import com.openalpr.jni.json.JSONObject;


public class AlprRegionOfInterest {
    private final int x;
    private final int y;
    private final int width;
    private final int height;

    AlprRegionOfInterest(JSONObject roiObj)
    {
        x = roiObj.getInt("x");
        y = roiObj.getInt("y");
        width = roiObj.getInt("width");
        height = roiObj.getInt("height");
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }
}

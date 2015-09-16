/*
 * Copyright (c) 2015 OpenALPR Technology, Inc.
 * Open source Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenALPR.
 *
 * OpenALPR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License
 * version 3 as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "result_aggregator.h"

using namespace std;
using namespace cv;

namespace alpr
{

  ResultAggregator::ResultAggregator()
  {

  }

  ResultAggregator::~ResultAggregator() {

  }


  void ResultAggregator::addResults(AlprFullDetails full_results)
  {
    all_results.push_back(full_results);
  }

  AlprFullDetails ResultAggregator::getAggregateResults()
  {
    assert(all_results.size() > 0);

    if (all_results.size() == 1)
      return all_results[0];


    AlprFullDetails response;

    // Plate regions are needed for benchmarking
    // Copy all detected boxes across all results
    for (unsigned int i = 0; i < all_results.size(); i++)
    {
      for (unsigned int k = 0; k < all_results[i].plateRegions.size(); k++)
        response.plateRegions.push_back(all_results[i].plateRegions[k]);
    }


    response.results.epoch_time = all_results[0].results.epoch_time;
    response.results.img_height = all_results[0].results.img_height;
    response.results.img_width = all_results[0].results.img_width;
    response.results.total_processing_time_ms = all_results[0].results.total_processing_time_ms;
    response.results.regionsOfInterest = all_results[0].results.regionsOfInterest;


    vector<vector<AlprPlateResult> > clusters = findClusters();

    // Assume we have multiple results, one cluster for each unique train data (e.g., eu, eu2)

    // Now for each cluster of plates, pick the best one
    for (unsigned int i = 0; i < clusters.size(); i++)
    {
      float best_confidence = 0;
      int best_index = 0;
      for (unsigned int k = 0; k < clusters[i].size(); k++)
      {
        if (clusters[i][k].bestPlate.overall_confidence > best_confidence)
        {
          best_confidence = clusters[i][k].bestPlate.overall_confidence;
          best_index = k;
        }
      }

      response.results.plates.push_back(clusters[i][best_index]);
    }

    return response;
  }

  // Searches all_plates to find overlapping plates
  // Returns an array containing "clusters" (overlapping plates)
  std::vector<std::vector<AlprPlateResult> > ResultAggregator::findClusters()
  {
    std::vector<std::vector<AlprPlateResult> > clusters;

    for (unsigned int i = 0; i < all_results.size(); i++)
    {
      for (unsigned int plate_id = 0; plate_id < all_results[i].results.plates.size(); plate_id++)
      {
        AlprPlateResult plate = all_results[i].results.plates[plate_id];

        int cluster_index = overlaps(plate, clusters);
        if (cluster_index < 0)
        {
          vector<AlprPlateResult> new_cluster;
          new_cluster.push_back(plate);
          clusters.push_back(new_cluster);
        }
        else
        {
          clusters[cluster_index].push_back(plate);
        }
      }
    }

    return clusters;
  }

  PlateShapeInfo ResultAggregator::getShapeInfo(AlprPlateResult plate)
  {
    int NUM_POINTS = 4;
    Moments mu;

    PlateShapeInfo response;

    vector<Point> points;
    for (int i = 0; i < NUM_POINTS; i++ )
    {
      cv::Point p(plate.plate_points[i].x, plate.plate_points[i].y);
      points.push_back(p);
    }

    mu = moments( points, false );
    response.center = cv::Point2f( mu.m10/mu.m00 , mu.m01/mu.m00 );
    response.area = mu.m00;

    Rect r = cv::boundingRect(points);
    response.max_width = r.width;
    response.max_height = r.height;

    return response;
  }

  // Returns the cluster ID if the plate overlaps.  Otherwise returns -1
  int ResultAggregator::overlaps(AlprPlateResult plate,
                                 std::vector<std::vector<AlprPlateResult> > clusters)
  {
    // Check the center positions to see how close they are to each other
    // Also compare the size.  If it's much much larger/smaller, treat it as a separate cluster

    PlateShapeInfo psi = getShapeInfo(plate);

    for (unsigned int i = 0; i < clusters.size(); i++)
    {
      for (unsigned int k = 0; k < clusters[i].size(); k++)
      {
        PlateShapeInfo cluster_shapeinfo = getShapeInfo(clusters[i][k]);

        int diffx = abs(psi.center.x - cluster_shapeinfo.center.x);
        int diffy = abs(psi.center.y - cluster_shapeinfo.center.y);

        // divide the larger plate area by the smaller plate area to determine a match
        float area_diff;
        if (psi.area > cluster_shapeinfo.area)
          area_diff = psi.area / cluster_shapeinfo.area;
        else
          area_diff = cluster_shapeinfo.area / psi.area;

        int max_x_diff = (psi.max_width + cluster_shapeinfo.max_width) / 2;
        int max_y_diff = (psi.max_height + cluster_shapeinfo.max_height) / 2;

        float max_area_diff = 4.0;
        // Consider it a match if center diffx/diffy are less than the average height
        // the area is not more than 4x different

        if (diffx <= max_x_diff && diffy <= max_y_diff && area_diff <= max_area_diff)
        {
          return i;
        }
      }

    }


    return -1;
  }
}
/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Part of the code is referenced from https://github.com/mcximing/sort-cpp. The orignal code are published under the BSD license.
 */
#include "sort.h"
#include "hungarian.h"
#include "my_tracker.h"
#include <opencv2/opencv.hpp>

#include <set>
#include <vector>
#include <chrono>


double Sort::GetIOU(cv::Rect_<float> bb_dr, cv::Rect_<float> bb_gt){
    float in = (bb_dr & bb_gt).area();
    float un = bb_dr.area() + bb_gt.area() - in;

    if(un < DBL_EPSILON)
        return 0;

    double iou = in / un;

    return iou;
}


std::vector<Sort::TrackingBox> Sort::Sortx(std::vector<BoxInfo> bbox, int fi){
    int max_age = 30;//max time object disappear
    int min_hits = 3; //min time target appear
    double iouThreshold = 0.70;//matching IOU

    std::vector<Sort::TrackingBox> detData;
    std::vector<cv::Rect_<float>> predictedBoxes;
    std::vector<std::vector<double>> iouMatrix;
    std::vector<int> assignment;

    std::set<int> unmatchedDetections;
    std::set<int> unmatchedTrajectories;
    std::set<int> allItems;
    std::set<int> matchedItems;
    std::vector<cv::Point> matchedPairs;
    std::vector<Sort::TrackingBox> frameTrackingResult;

    unsigned int trkNum = 0;
    unsigned int detNum = 0;
    auto start_time = std::chrono::high_resolution_clock::now();

    // bounding boxes in a frame store in detFrameData
    for (int i = 0; i < bbox.size() ; i++){
        Sort::TrackingBox tb;
        tb.frame = fi + 1;
        tb.box = Rect_<float>(cv::Point_<float>(bbox[i].x1, bbox[i].y1), cv::Point_<float>(bbox[i].x2, bbox[i].y2));
        detData.push_back(tb);
    }

    detFrameData.push_back(detData);

    if(trackers.size() == 0){
        std::vector<Sort::TrackingBox> first_frame;

        for (unsigned int i = 0; i < detFrameData[fi].size(); i++){

            MyTracker trk = MyTracker(detFrameData[fi][i].box);

            trackers.push_back(trk);

        }
        return first_frame;
    }

    /*
    3.1. get predicted locations from existing trackers
    */
    for (auto it = trackers.begin(); it != trackers.end();)
    {
        cv::Rect_<float> pBox = (*it).predict();
        if (pBox.x >= 0 && pBox.y >= 0)
        {
            predictedBoxes.push_back(pBox);
            it++;
        }
        else
        {
            it = trackers.erase(it);
            //cerr << "Box invalid at frame: " << frame_count << endl;
        }
    }

    /*
    3.2. associate detections to tracked object (both represented as bounding boxes)
    */
    trkNum = predictedBoxes.size();
    detNum = detFrameData[fi].size();
    iouMatrix.resize(trkNum, std::vector<double>(detNum, 0));


    for (unsigned int i = 0; i < trkNum; i++) // compute iou matrix as a distance matrix
    {
        for (unsigned int j = 0; j < detNum; j++)
        {
            // use 1-iou because the hungarian algorithm computes a minimum-cost assignment.
            iouMatrix[i][j] = 1 - GetIOU(predictedBoxes[i], detFrameData[fi][j].box);
        }
    }

    // solve the assignment problem using hungarian algorithm.
    // the resulting assignment is [track(prediction) : detection], with len=preNum
    HungarianAlgorithm HungAlgo;
    HungAlgo.Solve(iouMatrix, assignment);

    // find matches, unmatched_detections and unmatched_predictions
    if (detNum > trkNum) //	there are unmatched detections
    {
        for (unsigned int n = 0; n < detNum; n++)
            allItems.insert(n);

        for (unsigned int i = 0; i < trkNum; ++i)
            matchedItems.insert(assignment[i]);

        // calculate the difference between allItems and matchedItems, return to unmatchedDetections
        std::set_difference(allItems.begin(), allItems.end(),
            matchedItems.begin(), matchedItems.end(),
            insert_iterator<set<int>>(unmatchedDetections, unmatchedDetections.begin()));
    }
    else if (detNum < trkNum) // there are unmatched trajectory/predictions
    {
        for (unsigned int i = 0; i < trkNum; ++i)
            if (assignment[i] == -1) // unassigned label will be set as -1 in the assignment algorithm
                unmatchedTrajectories.insert(i);
    }
    else
        ;

    // filter out matched with low IOU
    // output matchedPairs
    for (unsigned int i = 0; i < trkNum; ++i)
    {
        if (assignment[i] == -1) // pass over invalid values
            continue;
        if (1 - iouMatrix[i][assignment[i]] < iouThreshold)
        {
            unmatchedTrajectories.insert(i);
            unmatchedDetections.insert(assignment[i]);
        }
        else
            matchedPairs.push_back(cv::Point(i, assignment[i]));
    }

    /*
    3.3. updating trackers
    update matched trackers with assigned detections.
    each prediction is corresponding to a tracker
    */
    int detIdx, trkIdx;
    for (unsigned int i = 0; i < matchedPairs.size(); i++)
    {
        trkIdx = matchedPairs[i].x;
        detIdx = matchedPairs[i].y;
        trackers[trkIdx].update(detFrameData[fi][detIdx].box);
    }

    // create and initialize new trackers for unmatched detections
    for (auto umd : unmatchedDetections)
    {
        MyTracker tracker = MyTracker(detFrameData[fi][umd].box);
        trackers.push_back(tracker);
    }

    vector<MyTracker>::iterator it = trackers.begin();
    for (; it != trackers.end();)
    {
        if (it != trackers.end() && (*it).m_time_since_update > max_age)
        {
            it = trackers.erase(it);
            continue;
        }

        if (((*it).m_time_since_update < 1) &&
            ((*it).m_hit_streak >= min_hits ))
        {
            Sort::TrackingBox res;
            res.box = (*it).get_state();
            res.id = (*it).m_id + 1;            
            res.frame = fi;
            frameTrackingResult.push_back(res);
            it++;
        }
        else
        {
            it++;
        }

    }
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    return frameTrackingResult;

}

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
 */

#include "pose_action.h"


std::vector<action_helper> action_recs(3);

void PoseAction::clear_action_count() {
  action_recs[0].action_count = 0;
  action_recs[1].action_count = 0;
  action_recs[2].action_count = 0;
  return;
}



int PoseAction::get_action_count(int recid) {
  return action_recs[recid].action_count;
}

float PoseAction::get_xyratio(std::vector<KKeyPoint> &kpts_sframe, int index_x, int index_y) {
  float xdiff = std::abs(kpts_sframe[index_x].p.x - kpts_sframe[index_y].p.x);
  float ydiff = std::abs(kpts_sframe[index_x].p.y - kpts_sframe[index_y].p.y);
  return xdiff/ydiff;
}

bool PoseAction::get_xyhigher(std::vector<KKeyPoint> &kpts_sframe, int index_x, int index_y) {
  float ydiff = kpts_sframe[index_x].p.y - kpts_sframe[index_y].p.y;
  return ydiff > 0;
}

int PoseAction::check_lateral_raise(std::vector<KKeyPoint> &kpts_sframe, int recid,float thres_conf) {
  std::vector<int> kpts = {4, 8, 5, 9};
  float down_thres=0.5;
  float up_thres=3.;
  if(kpts_sframe.empty()) {
    return action_recs[recid].action_count;
  }
  float xy_ratio;
  if (kpts_sframe[kpts[0]].prob > thres_conf && kpts_sframe[kpts[1]].prob > thres_conf) {
    xy_ratio = get_xyratio(kpts_sframe, kpts[0], kpts[1]);
  }
  else if (kpts_sframe[kpts[2]].prob > thres_conf && kpts_sframe[kpts[3]].prob > thres_conf) {
    xy_ratio = get_xyratio(kpts_sframe, kpts[2], kpts[3]);
  }
  else {
    return action_recs[recid].action_count;
  }
  if (xy_ratio < down_thres && action_recs[recid].mark) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 4) {
      action_recs[recid].mark = false;
      action_recs[recid].latency = 0;
    }
  }
  else if(xy_ratio > up_thres && !(action_recs[recid].mark)) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 4) {
      action_recs[recid].action_count += 1;
      action_recs[recid].mark = true;
      action_recs[recid].latency = 0;
    }
  }
  return action_recs[recid].action_count;
}

int PoseAction::check_stand_press(std::vector<KKeyPoint> &kpts_sframe, int recid,float thres_conf) {
  std::vector<int> kpts = {6, 4, 7, 5};
  if(kpts_sframe.empty()) {
    return action_recs[recid].action_count;
  }
  bool xy_ratio;
  if (kpts_sframe[kpts[0]].prob > thres_conf && kpts_sframe[kpts[1]].prob > thres_conf) {
    xy_ratio = get_xyhigher(kpts_sframe, kpts[0], kpts[1]);
  }
  else if(kpts_sframe[kpts[2]].prob > thres_conf && kpts_sframe[kpts[3]].prob > thres_conf) {
    xy_ratio = get_xyhigher(kpts_sframe, kpts[2], kpts[3]);
  }
  else {
    return action_recs[recid].action_count;
  }

  if (xy_ratio && action_recs[recid].mark) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 4) {
      action_recs[recid].mark = false;
      action_recs[recid].latency = 0;
    }
  }
  else if(!xy_ratio && !(action_recs[recid].mark)) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 4) {
      action_recs[recid].action_count += 1;
      action_recs[recid].mark = true;
      action_recs[recid].latency = 0;
    }
  }
  return action_recs[recid].action_count;
}

int PoseAction::check_deep_down(std::vector<KKeyPoint> &kpts_sframe, int recid,float thres_conf) {
  //std::vector<int> kpts = {8, 12, 9, 13};
  std::vector<int> kpts = {11, 13, 12, 14};
  if(kpts_sframe.empty()) {
    return action_recs[recid].action_count;
  }
  bool xy_ratio;
  if (kpts_sframe[kpts[0]].prob > thres_conf && kpts_sframe[kpts[1]].prob > thres_conf) {
    xy_ratio = get_xyhigher(kpts_sframe, kpts[0], kpts[1]);
  }
  else if(kpts_sframe[kpts[2]].prob > thres_conf && kpts_sframe[kpts[3]].prob > thres_conf) {
    xy_ratio = get_xyhigher(kpts_sframe, kpts[2], kpts[3]);
  }
  else {
    return action_recs[recid].action_count;
  }

  if (xy_ratio && action_recs[recid].mark) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 4) {
      action_recs[recid].mark = false;
      action_recs[recid].latency = 0;
    }
  }
  else if(!xy_ratio && !(action_recs[recid].mark)) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 4) {
      action_recs[recid].action_count += 1;
      action_recs[recid].mark = true;
      action_recs[recid].latency = 0;
    }
  }
  return action_recs[recid].action_count;
}

int PoseAction::check_deep_down2(std::vector<KKeyPoint> &kpts_sframe, float h, int recid,float thres_conf) {
//  std::vector<int> kpts = {9, 13, 10, 14};
  std::vector<int> kpts = {10, 12, 11, 13};
  if(kpts_sframe.empty()) {
    return action_recs[recid].action_count;
  }
  float down_thres=0.12;
  float up_thres=0.16;
  float xy_ratio;
  if (kpts_sframe[kpts[0]].prob > thres_conf && kpts_sframe[kpts[1]].prob > thres_conf) {
    int ydiff = std::max(-kpts_sframe[kpts[0]].p.y + kpts_sframe[kpts[1]].p.y, 0.f);
    xy_ratio = ydiff / h;
  }
  else if(kpts_sframe[kpts[2]].prob > thres_conf && kpts_sframe[kpts[3]].prob > thres_conf) {
    int ydiff = std::max(-kpts_sframe[kpts[2]].p.y + kpts_sframe[kpts[3]].p.y, 0.f);
    xy_ratio = ydiff / h;
  }
  else {
    return action_recs[recid].action_count;
  }

  if (xy_ratio<down_thres && action_recs[recid].mark) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 4) {
      action_recs[recid].mark = false;
      action_recs[recid].latency = 0;
    }
  }
  else if(xy_ratio>up_thres && !(action_recs[recid].mark)) {
    action_recs[recid].latency += 1;
    if (action_recs[recid].latency == 4) {
      action_recs[recid].action_count += 1;
      action_recs[recid].mark = true;
      action_recs[recid].latency = 0;
    }
  }
  return action_recs[recid].action_count;
}

int PoseAction::single_action_check(std::vector<KKeyPoint> &results_kpts, float h, int actionid,float thres_conf, int recid) {
  if (results_kpts.size()==0 || actionid>3) {
    return action_recs[recid].action_count;
  }
  if (actionid == 1) {
    return check_lateral_raise(results_kpts, recid,thres_conf);
  }
  else if (actionid == 2) {
    return check_stand_press(results_kpts, recid,thres_conf);
  }
  else {
    return check_deep_down(results_kpts, recid,thres_conf);
  }
}















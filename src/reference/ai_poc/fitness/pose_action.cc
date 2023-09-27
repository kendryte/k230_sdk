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
#include "utils.h"

std::vector<action_helper> action_recs(3);

float get_leg_ratio(std::vector<KKeyPoint> &kpts_sframe,std::vector<int> kpts ) {
  
  int aver_hip = ( kpts_sframe[kpts[2*0]].p.y + kpts_sframe[kpts[2*0+1]].p.y ) / 2.0;
  int aver_knee = ( kpts_sframe[kpts[2*1]].p.y + kpts_sframe[kpts[2*1+1]].p.y ) / 2.0;
  int aver_ankle = ( kpts_sframe[kpts[2*2]].p.y + kpts_sframe[kpts[2*2+1]].p.y ) / 2.0;

  float ratio = ( aver_hip - aver_knee ) * 1.0 / (aver_knee - aver_ankle);

  return ratio;

}

int PoseAction::check_deep_down(std::vector<KKeyPoint> &kpts_sframe, int recid,float thres_conf) 
{

    float down_thres=0.5;
    float up_thres=0.9;

    if(kpts_sframe.empty()) {
        return action_recs[recid].action_count;
    }

    float ratio;

    std::vector<int> kpts = {11,12,13,14,15,16};

    if (kpts_sframe[kpts[0]].prob > thres_conf && kpts_sframe[kpts[1]].prob > thres_conf &&
     kpts_sframe[kpts[2]].prob > thres_conf &&  kpts_sframe[kpts[3]].prob > thres_conf && 
      kpts_sframe[kpts[4]].prob > thres_conf &&  kpts_sframe[kpts[5]].prob > thres_conf) {
        ratio = get_leg_ratio(kpts_sframe,kpts);
    }
    else {
        return action_recs[recid].action_count;
    }

    if (ratio > up_thres && action_recs[recid].mark){
        action_recs[recid].latency += 1;
        if (action_recs[recid].latency == 2) {
            action_recs[recid].mark = false;
            action_recs[recid].latency = 0;
        }
    }
    else if( ratio < down_thres && !(action_recs[recid].mark)) {
        action_recs[recid].latency += 1;
        if (action_recs[recid].latency == 2) {
            action_recs[recid].action_count += 1;
            action_recs[recid].mark = true;
            action_recs[recid].latency = 0;
        }
    }
    return action_recs[recid].action_count;
}


int PoseAction::single_action_check(std::vector<KKeyPoint> &results_kpts, float thres_conf, int actionid, int recid) {


  if (results_kpts.size()==0 || actionid>3) {
    return action_recs[recid].action_count;
  }
  else {
    return check_deep_down(results_kpts,recid,thres_conf);
  }
}















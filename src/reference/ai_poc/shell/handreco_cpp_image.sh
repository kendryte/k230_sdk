#!/bin/sh

set -x
./sq_handreco.elf hand_det.kmodel input_hd.jpg 0.15 0.4 hand_reco.kmodel 0

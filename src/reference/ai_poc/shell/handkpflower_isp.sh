#!/bin/sh

set -x
./sq_handkp_flower.elf hand_det.kmodel None 0.15 0.4 handkp_det.kmodel flower_rec.kmodel 0

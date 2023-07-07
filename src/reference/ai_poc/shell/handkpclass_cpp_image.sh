#!/bin/sh

set -x
./sq_handkp_class.elf hand_det.kmodel input_hd.jpg 0.15 0.4  handkp_det.kmodel 0

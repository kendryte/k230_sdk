#!/bin/sh

set -x
./sq_handkp_ocr.elf hand_det.kmodel input_ocr.jpg 0.15 0.4 handkp_det.kmodel ocr_det.kmodel 0.15 0.4 ocr_rec.kmodel 0

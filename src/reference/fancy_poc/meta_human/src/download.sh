#!/bin/bash 

for file in blender_tools_v1.0.zip central_control_v1.0.zip run_sh.zip Tftpd64.zip ;  
do  
wget https://ai.b-bug.org/k230/downloads/fancy_poc/meta_human/$file || wget https://kendryte-download.canaan-creative.com/k230/downloads/fancy_poc/meta_human/$file;  
done  

for file in onboard_v2.2.zip
do
wget https://ai.b-bug.org/k230/downloads/fancy_poc/meta_human/k230_board/$file || wget https://kendryte-download.canaan-creative.com/k230/downloads/fancy_poc/meta_human/k230_board/$file;  
done


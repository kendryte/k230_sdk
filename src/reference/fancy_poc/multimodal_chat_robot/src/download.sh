#!/bin/bash 
wget https://ai.b-bug.org/k230/downloads/fancy_poc/multimodal_chat_robot/model_sd_v1.0.zip || wget https://kendryte-download.canaan-creative.com/k230/downloads/fancy_poc/multimodal_chat_robot/model_sd_v1.0.zip;  

for file in onboard_v2.3.zip
do
wget https://ai.b-bug.org/k230/downloads/fancy_poc/multimodal_chat_robot/k230_board/$file || wget https://kendryte-download.canaan-creative.com/k230/downloads/fancy_poc/multimodal_chat_robot/k230_board/$file;  
done



#######download.sh########
for file in AIScale.zip;  
do  
wget https://ai.b-bug.org/k230/downloads/fancy_poc/ai_scale/$file || wget https://kendryte-download.canaan-creative.com/k230/downloads/fancy_poc/ai_scale/$file;  
done  

for file in onboard_v2.5.zip
do
wget https://ai.b-bug.org/k230/downloads/fancy_poc/ai_scale/k230_board/$file || wget https://kendryte-download.canaan-creative.com/k230/downloads/fancy_poc/ai_scale/k230_board/$file;  
done

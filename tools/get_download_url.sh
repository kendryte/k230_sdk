#!/bin/bash
if curl --output /dev/null --silent --head --fail "https://ai.b-bug.org/k230/"; then 
    echo "URL is accessible"; 
	export DOWNLOAD_URL="https://ai.b-bug.org/k230/"; 
else
	echo "URL is not accessible";
	export DOWNLOAD_URL="https://kendryte-download.canaan-creative.com/k230";
fi
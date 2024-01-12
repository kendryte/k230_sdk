@echo off
move .\test_uvc.exe- test_uvc.exe 2>nul
::test_uvc.exe -m 1 -s 1  ::snap once every second
::test_uvc.exe -m 2 -i "H1280W720_ref.bin" -o "/sharefs/H1280W720_ref.bin"
::test_uvc.exe -m 2 -i "H1280W720_conf.bin" -o "/sharefs/H1280W720_conf.bin"
::test_uvc.exe -m 2 -i "H1280W720_conf.zip" -o "/sharefs/H1280W720_conf.zip"
::test_uvc.exe -m 2 -i "H1280W720_ref.zip" -o "/sharefs/H1280W720_ref.zip"

test_uvc.exe -m 0 -s 0
pause

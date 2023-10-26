# 1.简介

拼图游戏：其中手掌检测采用了yolov5网络结构，backbone选取了1.0-mobilenetV2，手掌关键点检测采用了resnet50网络结构。使用该应用，可得到图像或视频中的每个手掌的21个骨骼关键点位置。并且可以实现拼图游戏的功能，游戏方式分为两种，一种为图片复原拼图，另一种为数字华容道拼图。

# 2.应用使用说明

## 2.1 使用帮助

```
"Usage: " << puzzle_game.elf << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <bin_file> <level> <debug_mode>"

各参数释义如下：
kmodel_det      手掌检测 kmodel路径
obj_thresh      手掌检测阈值
nms_thresh      手掌检测非极大值抑制阈值
kmodel_kp       手掌关键点检测 kmodel路径
bin_file        拼图文件 (文件名 或者 None(表示排序数字拼图模式))
level           拼图游戏难度
debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试
 
 #数字华容道视频流推理：（puzzle_game_isp.sh）
./puzzle_game.elf hand_det.kmodel 0.15 0.4 handkp_det.kmodel None 3 0

 #图片复原视频流推理：（puzzle_game_bin_isp.sh）
./puzzle_game.elf hand_det.kmodel 0.15 0.4 handkp_det.kmodel pintu.bin 3 0

操作说明：
屏幕界面显示左右（上下）两块拼图，左（上）部分的拼图为游戏区域，右（下）部分为原始图像 以供参考。使用食指进入游戏，中指与食指指尖相距较远时，食指指尖用来确定当前指向哪一块拼图分区。当食指与中指指尖逼近时，可以交换当前区域和灰色移动块区域（注：只能交换灰色移动块区域的上下左右四个区域）。
```
## 2.2 效果展示
<img src="https://kendryte-download.canaan-creative.com/k230/downloads/doc_images/ai_demo/puzzle_game/puzzle_game.gif" alt="video.gif" width="50%" height="50%" />



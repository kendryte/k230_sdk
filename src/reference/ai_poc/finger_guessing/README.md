# 1.简介

猜拳游戏中：手掌检测部分采用了yolov5网络结构，backbone选取了1.0-mobilenetV2，手掌关键点检测部分采用了resnet50网络结构。使用该应用，可得到视频中的每个手掌的21个骨骼关键点位置,并根据关键点的位置二维约束判断获得静态手势。此代码实现猜拳游戏玩家稳赢，玩家必输，和多局胜出制。

# 2.应用使用说明

## 2.1 使用帮助

```
"Usage: " << finger_guessing.elf << "<kmodel_det> <obj_thresh> <nms_thresh> <kmodel_kp> <guess_mode> <debug_mode>"

各参数释义如下：
kmodel_det      手掌检测 kmodel路径
obj_thresh      手掌检测阈值
nms_thresh      手掌检测非极大值抑制阈值
kmodel_kp       手掌关键点检测 kmodel路径
guess_mode      石头剪刀布的游戏模式 0(玩家稳赢) 1(玩家必输) 奇数n(n局定输赢)
debug_mode      是否需要调试，0、1、2分别表示不调试、简单调试、详细调试

 #视频流推理：
（finger_guessing_0_isp.sh）
./finger_guessing.elf hand_det.kmodel 0.15 0.4 handkp_det.kmodel 0 0
（finger_guessing_1_isp.sh）
./finger_guessing.elf hand_det.kmodel 0.15 0.4 handkp_det.kmodel 1 0
（finger_guessing_n_isp.sh）
./finger_guessing.elf hand_det.kmodel 0.15 0.4 handkp_det.kmodel 3 0
```




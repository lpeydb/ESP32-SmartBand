# ESP32-SmartBand
> 这是大学三年级的实训项目。
>
## PCB效果

![](Hardware/PCB.jpg)
## 连线

![](Hardware/Connection.png)

## 演示视频

[B站视频](https://www.bilibili.com/video/BV1it4y1s7AY?spm_id_from=333.999.0.0)

## 关于取模

[汉字取模](https://www.23bei.com/tool/965.html)

[图片取模](https://javl.github.io/image2cpp/)

[GIF取模](https://www.arduino.cn/thread-104936-1-1.html)

## 一些遗憾

+ 全局变量使用过多。

  当时还没学指针，不懂得可以通过指针传递地址给函数进行操作。

+ 几乎所有代码都塞在`main.cpp`文件里，代码可读性差。
# test
Used for store small programs, can not use gist at beijing now. 
# TODO
decision a good hash function 

XVMC是什么？
为了加速mpeg2的解码，尤其为了实现1920*1080的高清全帧解码，很多显卡提供了mpeg硬件解码单元。而XVMC就是使用这个单元的一套规则。
在用户空间存在的就是类似libviaXvMC.so，libviaXvMCPro.so这样的库，它们负责和内核的DRM模块打交道，从而能使用硬件提供的mpeg解码加速功能。而每块视频卡对应的用户空间的xvmc库都不同，所以XvMC提供了一个包装盒，libXvMCW.so，它会向ddx询问应该使用哪个共享库，如果ddx没有回答，它就会使用/etc/X11/XvMCConfig文件来确定。

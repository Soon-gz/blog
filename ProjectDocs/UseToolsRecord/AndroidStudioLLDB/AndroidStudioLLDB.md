# AndroidStudio下载安装

https://developer.android.com/studio?hl=zh-cn

一键安装，没有废话。

# AS LLDB断点调试条件

## 1、包体debuggable必须true

如果包体不可调试，那么使用apktool反编译，然后修改清单文件，添加debuggable属性。

```
解析apk
java -jar apktool_2.6.1.jar d  .\test.apk -o out
回编apk
java -jar apktool_2.6.1.jar b out -o test_kill.apk
添加以下属性
android:debuggable="true"
```

## 2、拥有和包名一样的Android工程

比如需要断点的包名是 com.example.android16k ，那么需要新建一个com.example.android16k的同名空工程，来欺骗AS IDE，否则无法正常调试附加。

# AS 设置断点符号

比如想在linker的dlopen断点，对某个so的加载流程进行分析。那么可以通过AS的符号设置断点，需要断点的so名称是libandroid16k.so

![1](imges\1.png)

```
(char*)strstr((char*)$x0, "android16k") != (char*)0
```

![1](imges\2.png)

# 断点调试进入init

除了hook dlopen ext之外，等进入断点后，再添加一个do_dlopen的断点在linker64。

```
__dl__Z9do_dlopenPKciPK17android_dlextinfoPKv

linker64
```

![1](imges\3.png)

```
adb shell am start -D -n com.example.android16k/.MainActivity
```

调试启动应用app，先关掉do_dlopen的拦截，不然所有so加载都会被断住，毫无意义，并附加调试进程。

![1](imges\4.png)

同时注意，需要将调试修改为java+native模式，附加后进入断点。

![1](imges\5.png)

然后开启do_dlopen的断点，在do_dlopen中找到__dl__ZN6soinfo17call_constructorsEv，这个时候的x0就是soinfo首地址了。

![1](imges\6.png)

可以查阅linker源码https://cs.android.com/ ，找到soinfo的结构体，这里可以强制做结构体转换，先简单手动分析，

![1](imges\7.png)

可以根据base定位到so的基址。

![1](imges\8.png)

之后可以配合IDA查看到的符号地址，加上这个基址进行断点分析。例如

```
b 0x70F8ED1470
breakpoint set -a 0x70F8ED1470
```

lldb具体的调试命令可以去查 https://lldb.llvm.org/use/map.html
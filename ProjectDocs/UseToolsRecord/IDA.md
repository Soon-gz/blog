IDA是逆向常用工具之一，非常强大。这里记录一下IDA的食用方式。

#### IDA启动调试root真机

**注意：需要使用root手机，应用 debuggable=true，并关闭AndroidStudio开发工具。**

首先将IDA安装目录的x64\dbgsrv\下的android_server推送到手机目录。

```
adb push \IDAPro7.6SP1\x64\dbgsrv\android_server64  /data/local/tmp/a64_server
```

然后进入手机的adb shell模式，对文件进行赋权。

```
adb shell
cd /data/local/tmp/
chmod +x a64_server
```

再启动server进行监听，默认修改好端口，避免被检测软件通过端口反调试检测。

```
./a64_server -p 11689
```

这边就启动好了手机端的server。

windows shell 新开一个窗口，将端口进行转发。

```
adb forward tcp:11689 tcp:11689
```

通过adb am的调试命令启动应用。

```
adb shell am start -D -n com.example.myapplication/.MainActivity
adb shell ps|findstr myapplication
1978
```

查看对应的进程号，然后使用jdwp转端口。

```
adb forward tcp:23947 jdwp:1978
```

在启动ida64，通过debugger选择remote arm linux /Android debugger。然后填入127.0.0.1，11689端口号进行附加。

![](images\POPO20230409-094414.png)

然后勾选Debugger -> Debugger  Options -> Suspend on library load/unload。点F9继续运行。等待jdb连接。

```
 jdb -connect com.sun.jdi.SocketAttach:hostname=localhost,port=23947
```

执行了jdb命令后，就成功的调试上了。

#### IDA 附加调试

先让应用运行起来，然后通过IDA直接附加。不是通过adb am启动应用。

流程和上面是一样的，先将IDA server运行起来。

```
./a64_server -p 11689
```

新开一个windows shell，端口转发。

```
adb forward tcp:11689 tcp:11689
```

启动IDA，然后通过debugger选择remote arm linux /Android debugger。然后填入127.0.0.1，11689端口号进行附加就可以进行调试了，方便很多。



#### IDA调试常见异常

若是调试过程中一直弹窗提示异常，可尝试SIGPWR和SIGXCPU都Pass to Application。

![](images\POPO20230409-094428.png)

点击 Debugger -> Debugger  Options -> Edit exceptions。找到SIGPWR和SIGXCPU，修改为Pass to Application试试。

#### 一键调试脚本

```
//启动调试

adb forward tcp:23946 tcp:23946 

adb shell am start -D  -n package/Launcher

adb shell ps |findstr package

adb forward tcp:23947 jdwp:IDA atach可以看该进程的id
//上面内容可做成脚本，下面的内容要在IDA设置好调试后，shell窗口执行
jdb -connect com.sun.jdi.SocketAttach:hostname=localhost,port=23947
连接时需要关闭AS，避免8700端口被占用
```


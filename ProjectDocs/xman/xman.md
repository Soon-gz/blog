## ctf-xman

xman是一道ctf题，记录一下逆向过程。

案例地址：https://github.com/Soon-gz/blog/tree/main/ProjectDocs/xman

安装后启动可以正常运行，点自由正义分享之后，进入注册页面。

![1](images\1.png)

输入 123，点注册后退出应用程序，再次打开和之前的逻辑一样。

## 源码分析

直接用jadx打开apk。

![](images\2.png)

这里点击注册后，调用了MyApp的saveSN函数，这是一个native函数。用IDA打开lib/armeabi/libmyjni.so文件。

在导出符号表搜索JNI_OnLoad函数，按F5并导入jni.h，将变量切换为JavaVM 和 JNIEnv

![](images\3.png)

根据RegisterNatives函数定义，动态注册的函数地址在off_5004位置。

![](images\4.png)

可以看到 saveSN的符号地址。n2就是saveSN函数了。

![](images\5.png)

经过简单处理后，得到了save SN的伪代码。将用户输入的字符串经过一个算法之后保存到/sdcard/reg.dat这个目录。这个算法我们后面在来逆推一下。当保存成功后，用户打开应用，在MyApp的oncreate会调用initSN函数。

![](images\6.png)

根据我们在so定位到的函数地址，initSN的对应函数为n1，简单F5后。

![](images\7.png)

可以看到，从/sdcard/reg.dat读取数据，如果是EoPAoY62@ElRD，那么就调用setValue将myApp的m字段设置为1，表示已注册，在MainActivity会调用myApp的work函数，进行内容提示。

![](images\8.png)

getValue就是获取myApp的m，判断是否是1，来进行提示。这里我们可以看到输入的内容就是flag了。所以需要去逆推save SN的算法。这里我们知道了算法最后的输出是EoPAoY62@ElRD，所以可以用python z3进行逆推。

```
from z3 import *

def main():
    s = Solver()

    result = "EoPAoY62@ElRD"
    key    = "W3_arE_whO_we_ARE"
    index_key = 2016

    x = [BitVec("x%s" % i, 8) for i in range(len(result))]

    for i in range(len(result)):
        v11 = None
        if (i % 3 == 1):
            index_key = (index_key + 5) % 16
            v11 = key[index_key + 1]
        elif (i % 3 == 2):
            index_key = (index_key + 7) % 15
            v11 = key[index_key + 2]
        else:
            index_key = (index_key + 3) % 13
            v11 = key[index_key + 3]        
        s.add((x[i]) == ord(v11) ^ ord(result[i]))
        
    if (s.check() == sat):
        model = (s.model())
        flag = ""
        for i in range(len(result)):
            flag += chr(model[x[i]].as_long().real)
        print('"' + flag + '"')

if __name__ == "__main__":
    main()
```

最后得出的flag 是 xman{201608Am!2333}
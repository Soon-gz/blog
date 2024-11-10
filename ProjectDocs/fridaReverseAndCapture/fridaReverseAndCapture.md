肉丝大佬出的frida入门，记录一些学习笔记。

#### objection hook类参数，返回值和调用栈

```
android hooking watch class_method <method> --dump-args --dump-backtrace --dump-return
```

非常有用的命令，可以根据需求，--dump-args  打印指定类的入参，--dump-backtrace 调用栈以及  --dump-return返回值，分析函数的明文和密文，都有可能出现在参数和返回值。
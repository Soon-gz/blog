1、字段名和方法名相同时，设置字段变量时需要加下划线 _，来区分字段名和方法。

2、hook动态加载dex，需要使用Java的enumerateClassLoader来遍历loader，如果找到动态dex的类，那么将Java的classFactory的loader切换到当前loader即可操作。

3、启动apk hook使用--no-pause -f命令

4、adb shell ，输入text命令 input text "codenameduchess"

5、单独打包一个class文件，
```
jar -cvf dex.jar com/dexample/main.class
```
再将jar转为dex  
```
dx --dex --output=ddex.dex .\ddex.jar 
```
 使用frida  Java.openClassfile加载dex，之后加载的dex需要调用load函数，完成加载。

6、hook registerNatives 可以通过libart的enumateSymbals遍历所有符号，找到art 
	JNI的registerNatives，并作hook，在获取方法名和参数时注意Process.pointerSize的偏移以及读取字符串时的readPointer和readCString。

7、frida 保存文件可以通过NativeFunction创建libc的fopen，fputs和fclose实现，字符串申明时需要使用Memory.alloc，另一种使用js提供的File API创建文件并使用write写入。

8、IDA trace需要先运行apk，IDA attach调试，F9，在脚本中需要将hook的so名字，方法起止地址，根据运行平台修改返回寄存器arm thumb为LR，arm64为X30，点击IDA file，scrept file 加载脚本，
	成功之后会在相应地址下了断点，之后触发算法，在python中输入starthook(),并挂起其他线程suspend_other_thread()，之后点击Debugger --> Tracing --> options，不勾选Trace over debugger segments，
	并设置Trace file路径，勾选 Debugger --> Tracing --> instruction Tracing ，F9即可。如果trace到某条指令无法trace，断点该指令，并点击instruction Tracing

9、搭建VS CODE frida自动提示，google搜索frida-gum npm，找到github的@types/frida-gum,在需要的目录下执行npm install --save @types/frida-gum即可。

10.hook dlopen可以通过Module的findExportByName(null,android_dlopen_ext)进行hook。


11、修改系统库返回值，比如修改libc.so的gettimeofday方法，可以使用CModule，之后新建一个NativeFunction，通过Interpreter replace原方法地址，使用NativeCallback作为回调处理。代码如下。

11 遇到md5的值，可以先猜测md5原始值，上google浏览器搜索

12 frida打印字符串参数
	frida在js中打印jstring
```
	aes_value = Java.vm.getEnv().getStringUtfChars(args[1], null).readCString()

	C字符串转成js string

	ptr(result).readCString()
```

13、frida 读取参数地址对应内存数据
```
	var addr = args[1].readPointer();
    console.log("[AssetManager::getZipFileLocked]->",ptr(addr).readCString());
```
14、javascript API分开打印十进制的 byte数组 
```
		function bytes2hexstr_2(arrBytes)
	{
	   var str_hex = JSON.stringify(arrBytes);
	   return str_hex;
	}
```

15、用Android API打印十六进制的 byte数组 
```
	function bytes2hexstr_3(bytes_ary)
	{
	   var ByteString = Java.use("com.android.okhttp.okio.ByteString");
	   var str_hex = ByteString.of(bytes_ary).hex();
	   return str_hex;
	}
```
16、修改系统库返回值，比如修改libc.so的gettimeofday方法，可以使用CModule，之后新建一个NativeFunction，通过Interpreter replace原方法地址，使用NativeCallback作为回调处理。代码如下。

   ```
   var gettimeofday_func = Module.findExportByName(null,"gettimeofday");
    var gettimeofday = new NativeFunction(gettimeofday_func, "int", ["pointer", "pointer"]);
    if(gettimeofday) {
        var source = [
            'struct timeval {',
            '    long tv_sec;',
            '    long tv_usec;',
            '};',
            'void modify_timests(struct timeval* tv, long tv_sec, long tv_usec) {',
            '  tv->tv_sec = tv_sec;',
            '  tv->tv_usec = tv_usec;',
            '}',
        ].join('\n');
        var cm = new CModule(source);
        var modify_timests = new NativeFunction(cm.modify_timests,"void",["pointer","long","long"]);
        Interceptor.replace(gettimeofday_func,new NativeCallback(function(ptr_tz, ptr_tzp){
            var result = gettimeofday(ptr_tz, ptr_tzp);
            if(result == 0) {
                console.log("hook gettimeofday ",ptr_tz,ptr_tzp,result);
                var dat  = 0xaaaa;
                var dat2  = 0xbbbb;
                modify_timests(ptr_tz,dat,dat2);
            }
            return result;
        },"int",["pointer","pointer"]));
    }
    ```


17、通过jstring获取字符串打印 (ptr(Java.vm.tryGetEnv().getByteArrayElements(args[i])).readCString(parseInt(args[1])))



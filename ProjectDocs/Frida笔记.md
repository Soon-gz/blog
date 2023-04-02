1. 字段名和方法名相同时，设置字段变量时需要加下划线 _，来区分字段名和方法。

   ```
   public class FridaActivity3 extends BaseFridaActivity {
       private static boolean static_bool_var = false;
       private boolean bool_var = false;
       private boolean same_name_bool_var = false;
   
       @Override // com.github.lastingyang.androiddemo.Activity.BaseFridaActivity
       public String getNextCheckTitle() {
           return "当前第3关";
       }
   
       private void same_name_bool_var() {
           Log.d("Frida", static_bool_var + " " + this.bool_var + " " + this.same_name_bool_var);
       }
   
       @Override // com.github.lastingyang.androiddemo.Activity.BaseFridaActivity
       public void onCheck() {
           if (static_bool_var && this.bool_var && this.same_name_bool_var) {
               CheckSuccess();
               startActivity(new Intent(this, FridaActivity4.class));
               finishActivity(0);
               return;
           }
           super.CheckFailed();
       }
   }
   ```

   ```
   Java.choose("com.github.lastingyang.androiddemo.Activity.FridaActivity3", {
         onMatch : function(instance) {
           instance._same_name_bool_var.value = true;
         }, onComplete : function() {
         }
       })
   ```

2. hook动态加载dex，需要使用Java的enumerateClassLoader来遍历loader，如果找到动态dex的类，那么将Java的classFactory的loader切换到当前loader即可操作。

   ```
   function hook_dyn_dex() {
     Java.perform(function() {
       Java.enumerateClassLoaders({
         onMatch : function(loader) {
           try {
             if (loader.findClass("com.example.androiddemo.Dynamic.DynamicCheck")) {
               Java.classFactory.loader = loader;
               console.log(loader);
             }
           } catch (error) {
             
           }
         }, onComplete : function() {
         }
       });
       var DynamicCheck = Java.use("com.example.androiddemo.Dynamic.DynamicCheck");
       DynamicCheck.check.implementation = function() {
         var result = this.check();
         console.log("DynamicCheck.check:", result);
         return true;
       }
     })
   }
   ```

3. 动态加载dex

   ```
   //动态加载dex
   function load_dex() {
       var DecodeUtilsDex = Java.openClassFile("/data/local/tmp/DecodeUtils.dex");
       console.log("DecodeUtilsDex:", DecodeUtilsDex);
       Java.perform(function() {
           DecodeUtilsDex.load();
           var DecodeUtils = Java.use("com.example.androiddemo.DecodeUtils");
           console.log(DecodeUtils);
           var FridaActivity8 = Java.use("com.github.lastingyang.androiddemo.Activity.FridaActivity8");
           Java.scheduleOnMainThread(function() {
               console.log(DecodeUtils.$new().decode(FridaActivity8.$new().password.value));
           })
       });
   }
   ```

4. 启动apk hook使用--no-pause -f命令

   ```
   frida -U -f com.example.test -l xxx.js --no-pause 
   ```

5. adb shell ，输入text命令 input text "codenameduchess"

6. 单独打包一个class文件，

   ```
   jar -cvf dex.jar com/dexample/main.class
   ```

   再将jar转为dex  

   ```
   dx --dex --output=ddex.dex .\ddex.jar 
   ```

   使用frida  Java.openClassfile加载dex，之后加载的dex需要调用load函数，完成加载。

7. hook registerNatives 可以通过libart的enumateSymbals遍历所有符号，找到art  JNI的registerNatives，并作hook，在获取方法名和参数时注意Process.pointerSize的偏移以及读取字符串时的readPointer和readCString。

   ```
   function hook_registerNatives () {
     
     var module_libart = Process.findModuleByName("libart.so");
     console.log(module_libart);
     var addr_RegisterNatives = null;
     //枚举模块的符号
     var symbols = module_libart.enumerateSymbols();
     for (var i = 0; i < symbols.length; i++) {
         var name = symbols[i].name;
         if (name.indexOf("CheckJNI") == -1 && name.indexOf("JNI") > 0) {
             if (name.indexOf("RegisterNatives") > 0) {
                 console.log(name);
                 addr_RegisterNatives = symbols[i].address;
             }
   
         }
     }
   
     if (addr_RegisterNatives) {
       Interceptor.attach(addr_RegisterNatives, {
           onEnter: function (args) {
               var java_class = Java.vm.tryGetEnv().getClassName(args[1]);
               var methods = args[2];
               var method_count = parseInt(args[3]);
               console.log("addr_RegisterNatives java_class:", java_class, "method_count:", method_count);
               for (var i = 0; i < method_count; i++) {
                   console.log(methods.add(i * Process.pointerSize * 3).readPointer().readCString());
                   console.log(methods.add(i * Process.pointerSize * 3 + Process.pointerSize).readPointer().readCString());
                   var fnPtr = methods.add(i * Process.pointerSize * 3 + Process.pointerSize * 2).readPointer();
                   console.log(fnPtr);
   
               }
           }, onLeave: function (retval) {
   
           }
       })
   }
   }
   ```

8. frida 保存文件可以通过NativeFunction创建libc的fopen，fputs和fclose实现，字符串申明时需要使用Memory.alloc，另一种使用js提供的File API创建文件并使用write写入。

   ```
   function frida_file() {
       var file = new File("/sdcard/reg.dat", "r+");
       file.write("EoPAoY62@ElRD");
       file.flush();
       file.close();
   }
   
   //用frida调用c函数
   function c_read_file() {
       //fopen
       //fseek
       //ftell
       //fread
       //fclose
       var fopen = new NativeFunction(Module.findExportByName("libc.so", "fopen"), "pointer", ["pointer", "pointer"]);
       var fseek = new NativeFunction(Module.findExportByName("libc.so", "fseek"), "int", ["pointer", "int", "int"]);
       var ftell = new NativeFunction(Module.findExportByName("libc.so", "ftell"), "long", ["pointer"]);
       var fread = new NativeFunction(Module.findExportByName("libc.so", "fread"), "int", ["pointer", "int", "int", "pointer"]);
       var fclose = new NativeFunction(Module.findExportByName("libc.so", "fclose"), "int", ["pointer"]);
   
       var file = fopen(Memory.allocUtf8String("/sdcard/reg.dat"), Memory.allocUtf8String("r+"));
       fseek(file, 0, 2);
       var size = ftell(file);
       var buffer = Memory.alloc(size + 1);
       fseek(file, 0, 0);
       fread(buffer, size, 1, file);
       console.log("buffer:", buffer, buffer.readCString());
       fclose(file);
   }
   ```

9. IDA trace需要先运行apk，IDA attach调试，F9，在脚本中需要将hook的so名字，方法起止地址，根据运行平台修改返回寄存器arm thumb为LR，arm64为X30，点击IDA file，scrept file 加载脚本，
   	成功之后会在相应地址下了断点，之后触发算法，在python中输入starthook(),并挂起其他线程suspend_other_thread()，之后点击Debugger --> Tracing --> options，不勾选Trace over debugger segments，
   	并设置Trace file路径，勾选 Debugger --> Tracing --> instruction Tracing ，F9即可。如果trace到某条指令无法trace，断点该指令，并点击instruction Tracing

10. 搭建VS CODE frida自动提示，google搜索frida-gum npm，找到github的@types/frida-gum,在需要的目录下执行npm install --save @types/frida-gum即可。

    ```
    npm install --save @types/frida-gum
    ```

11. hook dlopen可以通过Module的findExportByName(null,android_dlopen_ext)进行hook。

    ```
    function hook_android_dlopen_ext() {
        Interceptor.attach(Module.findExportByName(null, "android_dlopen_ext"), {
            onEnter: function (args) {
                this.name = args[0].readCString();
                console.log("android_dlopen_ext:", this.name);
            }, onLeave: function (retval) {
                if (this.name.indexOf("libmyjni.so") > 0) {
                    hook_native();
                }
            }
        })
    }
    ```

12. 遇到md5的值，可以先猜测md5原始值，上google浏览器搜索

    

13. frida打印字符串参数
    frida在js中打印jstring

    ```
    	aes_value = Java.vm.getEnv().getStringUtfChars(args[1], null).readCString()
    
    	C字符串转成js string
    
    	ptr(result).readCString()
    ```

14. frida 读取参数地址对应内存数据

    ```
    	var addr = args[1].readPointer();
        console.log("[AssetManager::getZipFileLocked]->",ptr(addr).readCString());
    ```

15. javascript API分开打印十进制的 byte数组 

    ```
    	function bytes2hexstr_2(arrBytes)
    	{
    	   var str_hex = JSON.stringify(arrBytes);
    	   return str_hex;
    	}
    ```

16. 用Android API打印十六进制的 byte数组 

    ```
    	function bytes2hexstr_3(bytes_ary)
    	{
    	   var ByteString = Java.use("com.android.okhttp.okio.ByteString");
    	   var str_hex = ByteString.of(bytes_ary).hex();
    	   return str_hex;
    	}
    ```

17. 修改系统库返回值，比如修改libc.so的gettimeofday方法，可以使用CModule，之后新建一个NativeFunction，通过Interpreter replace原方法地址，使用NativeCallback作为回调处理。代码如下。

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

18. 通过jstring获取字符串打印

    ```
    (ptr(Java.vm.tryGetEnv().getByteArrayElements(args[i])).readCString(parseInt(args[1])))
    ```

19. 打印C的调用栈

    ```
    console.log('setValue called from:\n' +
                    Thread.backtrace(this.context, Backtracer.ACCURATE)
                        .map(DebugSymbol.fromAddress).join('\n') + '\n');
    ```

20. 打印java调用堆栈

    ```
    //打印调用栈
    function printStackTrace() {
        Java.perform(function() {
            var Exception = Java.use("java.lang.Exception");
            var exception = Exception.$new();
            var stackTrace = exception.getStackTrace().toString();
            console.log("==========================\r\n" + stackTrace.replaceAll(",", "\r\n") 
                + "\r\n==========================");
            exception.$dispose();
        });
    }
    ```

21. hook 某条指令，并打印对应的寄存器

    ```
        //inline hook
        Interceptor.attach(base_hello_jni.add(0x731C), {
            onEnter : function(args) {
                console.log("0x731C", this.context.x13, this.context.x14);
            }, onLeave : function(retval) {
               
            }
        });
    ```

22. 主动调用 so中某个函数，需要创建NativeFunction，并传入参数，参数需要通过alloc或者memory等API申明内存。

    ```
    function call_1CFF0() {
        var base_hello_jni = Module.findBaseAddress("libhello-jni.so");
        var sub_1CFF0 = new NativeFunction(base_hello_jni.add(0x1CFF0), "int", ["pointer", "int", "pointer"]);
        var input_str = "0123456789abcdef";
    
        var arg0 = Memory.allocUtf8String(input_str);
        var arg1 = input_str.length;
        var arg2 = Memory.alloc(arg1);
        sub_1CFF0(arg0, arg1, arg2);
        console.log(hexdump(arg2, {length : arg1}));
    }
    ```

23. 在frida中运行自己的代码，需要使用CModule加载自己的代码，并创建NativeFunction函数来实现调用。

    ```
    function makeid(length) {
        var result           = '';
        var characters       = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
        var charactersLength = characters.length;
        for ( var i = 0; i < length; i++ ) {
           result += characters.charAt(Math.floor(Math.random() * charactersLength));
        }
        return result;
     }
    
    function test_enc() {
        const cm = new CModule(`
        void enc_function(const char* input_str, int input_len, char* result) {
            const char* table_key1 = "9d9107e02f0f07984956767ab1ac87e5";
            const unsigned char table_key2[] = {0x37, 0x92, 0x44, 0x68, 0xA5, 0x3D, 0xCC, 0x7F, 0xBB, 0xF, 0xD9, 0x88, 0xEE, 0x9A, 0xE9, 0x5A};
            for (int i = 0; i < input_len; ++i) {
                unsigned char X2 = input_str[i];
                unsigned char key2 = table_key2[(i & 0xF) & 0xFFFFFFFF];
                unsigned char W8 = 0xDA;
                unsigned char W30 = 0x25;
                unsigned char W2 = X2;
                unsigned char W7 = W8 & (~W2);
                W2 = W2 & 0x25;
                W2 = W7 | W2;
                unsigned char W3 = key2;
                W7 = W8 & (~W3);
                W3 = W3 & W30;
                W3 = W7 | W3;
                W2 = W2 ^ W3;
                W3 = W2;
        
                unsigned char key1 = table_key1[(i ^ 0xFFFFFFF8) & i ];
                W2 = key1;
                W7 = key2;
                W30 = key2;
                unsigned char W1 = W2 & (~W3);
                W3 = W3 & (~W2);
                unsigned char W5 = W30 & (~W2);
                W2 = W2 & (~W30);
                W1 = W1 | W3;
                W2 = W5 | W2;
                W1 = (unsigned char)((unsigned char)W1 + (unsigned char)W7);
                W3 = W1 & (~W2);
                W1 = W2 & (~W1);
                W1 = W3 | W1;
                result[i] = W1;
            }
        }
    
        int test_eq(const char* buf1, const char* buf2, int buf_len) {
            for (int i = 0; i < buf_len; ++i) {
                if (buf1[i] != buf2[i]) {
                    return 0;
                }
            }
            return 1;
        }
    `);
        // console.log(JSON.stringify(cm));
        // console.log(cm.enc_function);
        var test_eq = new NativeFunction(cm.test_eq, "int", ["pointer", "pointer", "int"]);
    
        var enc_function = new NativeFunction(cm.enc_function, "void", ["pointer", "int", "pointer"]);
        
        for (var index = 1; index < 0x1000; index++) {        
            var input_str = makeid(index);
            var arg0 = Memory.allocUtf8String(input_str);
            var arg1 = input_str.length;
            var arg2 = Memory.alloc(arg1);
            enc_function(arg0, arg1, arg2);
            // console.log(hexdump(arg2, {length : arg1}));
            var test_ret = test_eq(call_1CFF0(input_str), arg2, arg1);
            if (test_ret == 0) {
                console.log(input_str);
            }
        }
        console.log("test_enc end");
    }
    ```

24. frida提供了Stalker 可进行Trace执行指令，方便进行指令执行流的分析。

    ```
    function StalkerTrace() {
        var base_hello_jni = Module.findBaseAddress("libhello-jni.so");
        var sub_1CFF0 = base_hello_jni.add(0x1CFF0);
        console.log(sub_1CFF0);
        var module_hello_jni = Process.findModuleByName("libhello-jni.so");
    
        var module_start = module_hello_jni.base;
        var module_end = module_hello_jni.base + module_hello_jni.size;
        var pre_regs = {}
    
        Interceptor.attach(sub_1CFF0, {
            onEnter: function (args) {
                this.arg0 = args[0];
                this.arg1 = args[1];
                this.arg2 = args[2];
                this.tid = Process.getCurrentThreadId();
    
                Stalker.follow(this.tid, {
                    events: {
                        call: false, // CALL instructions: yes please
    
                        // Other events:
                        ret: false, // RET instructions
                        exec: true, // all instructions: not recommended as it's
                        //                   a lot of data
                        block: false, // block executed: coarse execution trace
                        compile: false // block compiled: useful for coverage
                    },
                    // onCallSummary(summary) {
                    //     console.log("onCallSummary:", Object.entries(summary));
                    // },
                    // onReceive(events) {
                    //     // console.log(Stalker.parse(events))
                    //     for (const [index, value] of Object.entries(Stalker.parse(events))) {
                    //         if (value.indexOf("exec") >= 0) {
                    //             var address = value.toString().split(",")
                    //             var addr = address[1];
                    //             var module = Process.findModuleByAddress(addr);
                    //             if (module) {
                    //                 console.log("onReceive:", module.name + "!" + ptr(addr).sub(module.base), Instruction.parse(ptr(addr)));
                    //             }
                    //         }
    
                    //     }
                    // },
                    transform(iterator) {
                        let instruction = iterator.next();
                        do {
                            const startAddress = instruction.address;
                            const is_module_code = startAddress.compare(module_start) >= 0 &&
                                startAddress.compare(module_end) === -1;
                            if (is_module_code) {
                                // console.log(startAddress, instruction);
                                iterator.putCallout(function (context) {
                                    var pc = context.pc;
                                    var module = Process.findModuleByAddress(pc);
                                    if (module) {
                                        var diff_regs = get_diff_regs(context, pre_regs);
    
                                        console.log(module.name + "!" + ptr(pc).sub(module.base), 
                                            Instruction.parse(ptr(pc)),
                                            JSON.stringify(diff_regs));
                                    }
                                    
                                });
                            }
    
                            iterator.keep();
                        } while ((instruction = iterator.next()) !== null);
                    }
    
    
                });
    
            }, onLeave: function (retval) {
    
                Stalker.unfollow(this.tid);
    
                console.log("sub_1CFF0:", hexdump(this.arg0, { length: parseInt(this.arg1) }),
                    "\r\n", hexdump(this.arg2, { length: parseInt(this.arg1) }))
            }
        })
    }
    ```

25. frida hook windows 的dll，和hook android so动态库类似。

    ```
    // Find base address of current imported jvm.dll by main process fledge.exe
        const baseAddr = Module.findBaseAddress('Jvm.dll');
        console.log('Jvm.dll baseAddr: ' + baseAddr);
    
        const setAesDecrypt0 = resolveAddress('0x1FF44870'); // Here we use the function address as seen in our disassembler
    
        Interceptor.attach(setAesDecrypt0, { // Intercept calls to our SetAesDecrypt function
    
            // When function is called, print out its parameters
            onEnter(args) {
                console.log('');
                console.log('[+] Called SetAesDeCrypt0' + setAesDecrypt0);
                console.log('[+] Ctx: ' + args[0]);
                console.log('[+] Input: ' + args[1]); // Plaintext
                console.log('[+] Output: ' + args[2]); // This pointer will store the de/encrypted data
                console.log('[+] Len: ' + args[3]); // Length of data to en/decrypt
                dumpAddr('Input', args[1], args[3].toInt32());
                this.outptr = args[2]; // Store arg2 and arg3 in order to see when we leave the function
                this.outsize = args[3].toInt32();
            },
    
            // When function is finished
            onLeave(retval) {
                dumpAddr('Output', this.outptr, this.outsize); // Print out data array, which will contain de/encrypted data as output
                console.log('[+] Returned from setAesDecrypt0: ' + retval);
            }
        });
    
        function dumpAddr(info, addr, size) {
            if (addr.isNull())
                return;
    
            console.log('Data dump ' + info + ' :');
            const buf = addr.readByteArray(size);
    
            // If you want color magic, set ansi to true
            console.log(hexdump(buf, { offset: 0, length: size, header: true, ansi: false }));
        }
    
        function resolveAddress(addr) {
            const idaBase = ptr('0x1FEE0000'); // Enter the base address of jvm.dll as seen in your favorite disassembler (here IDA)
            const offset = ptr(addr).sub(idaBase); // Calculate offset in memory from base address in IDA database
            const result = baseAddr.add(offset); // Add current memory base address to offset of function to monitor
            console.log('[+] New addr=' + result); // Write location of function in memory to console
            return result;
        }
    ```

26. 

27. 

28. 

29. 

30. 

31. 

32. 

    




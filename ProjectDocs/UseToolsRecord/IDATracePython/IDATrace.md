IDA 的功能非常强大，可以通过trace记录指令执行后寄存器值的变化，如果固定了输入，以及知道结果，可以通过分析log日志来得到算法，这里记录一下操作过程。

下面是看雪大佬Yang写的tace脚本，拿来主义，先用再说。这个脚本只适合在python 2上运行，也就是IDA 7.0之前，在git仓库的/ProjectDocs\UseToolsRecord\IDATracePython\trace_xman_70.py文件。

#### 一把梭

IDA trace需要先运行apk，IDA attach调试，F9，在脚本中需要将hook的so名字，方法起止地址，根据运行平台修改返回寄存器arm thumb为LR，arm64为X30，点击IDA file，scrept file 加载脚本， 成功之后会在相应地址下了断点，之后触发算法，在python中输入starthook(),并挂起其他线程suspend_other_thread()，之后点击Debugger --> Tracing --> options，不勾选Trace over debugger segments， 并设置Trace file路径，勾选 Debugger --> Tracing --> instruction Tracing ，F9即可。如果trace到某条指令无法trace，断点该指令，并点击instruction Tracing。

#### Trace脚本修改

##### 起始地址

下面是trace脚本入口，xman分析的地址在 https://github.com/Soon-gz/blog/blob/main/ProjectDocs/xman/xman.md，主要算法在n2函数，那么用IDA静态打开可以看到导出符号，定位后可以通过快捷键ALT + P 查看函数的起始地址，0x11F8 - 0x1302；

```
def main():
    global debughook
    unhook()
    skip_functions = []
    modules_info = []
    start_ea = 0
    end_ea = []
    so_modules = ["libmyjni.so"]
    for module in idc._get_modules():
        module_name = os.path.basename(module.name)
        for so_module in so_modules:
            if re.search(so_module, module_name, re.IGNORECASE):
                print("modules_info append %08X %s %08X" % (module.base, module.name, module.size))
                if module_name == "libmyjni.so":
                    modules_info.append({"base": module.base, "size": module.size, "name": module.name})
                    start_ea = (module.base + 0x11F8)      #UUIDCheckSum
                    end_ea = [((module.base + 0x1302))]   
                    break

    if start_ea:
        set_breakpoint(start_ea)
    if end_ea:
        for ea in end_ea:
            set_breakpoint(ea)

    if skip_functions:
        print("skip_functions")
        for skip_function in skip_functions:
            print ("%08X" % skip_function)
    
    debughook = MyDbgHook(modules_info, skip_functions, end_ea)
    
    pass
```

##### 根据平台设置返回寄存器

根据运行平台修改返回寄存器arm thumb为LR，arm64为X30。

```
    def dbg_trace(self, tid, ea):
        #print("Trace tid=%d ea=0x%x" % (tid, ea))
        # return values:
        #   1  - do not log this trace event;
        #   0  - log it
        if self.line_trace:
            in_mine_so = False
            for module_info in self.modules_info:
                # print (module_info)
                so_base = module_info["base"]
                so_size = module_info["size"]
                if so_base <= ea <= (so_base + so_size):
                    in_mine_so = True
                    break

            self.trace_size += 1
            if (not in_mine_so) or (ea in self.skip_functions):
                if (self.trace_lr != 0) and (self.trace_step_into_count < self.trace_step_into_size):
                    self.trace_step_into_count += 1
                    return 0

                if (self.trace_lr != 0) and (self.trace_step_into_count == self.trace_step_into_size):
                    ida_dbg.enable_insn_trace(False)
                    ida_dbg.enable_step_trace(False)
                    ida_dbg.suspend_process()
                    if self.trace_size > self.trace_total_size:
                        self.trace_size = 0
                        ida_dbg.request_clear_trace()
                        ida_dbg.run_requests()

                    ida_dbg.request_run_to(self.trace_lr)
                    ida_dbg.run_requests()
                    self.trace_lr = 0
                    self.trace_step_into_count = 0
                    return 0

                if self.trace_lr == 0:
                    self.trace_lr = my_get_reg_value("LR")  #arm thumb LR, arm64 X30
            return 0
```

##### 需要trace的so名称

需要trace的名称是libmyjni.so

```
def main():
    global debughook
    unhook()
    skip_functions = []
    modules_info = []
    start_ea = 0
    end_ea = []
    so_modules = ["libmyjni.so"]
    for module in idc._get_modules():
        module_name = os.path.basename(module.name)
        for so_module in so_modules:
            if re.search(so_module, module_name, re.IGNORECASE):
                print("modules_info append %08X %s %08X" % (module.base, module.name, module.size))
                if module_name == "libmyjni.so":
                    modules_info.append({"base": module.base, "size": module.size, "name": module.name})
                    start_ea = (module.base + 0x11F8)      #UUIDCheckSum
                    end_ea = [((module.base + 0x1302))]   
                    break

    if start_ea:
        set_breakpoint(start_ea)
    if end_ea:
        for ea in end_ea:
            set_breakpoint(ea)

    if skip_functions:
        print("skip_functions")
        for skip_function in skip_functions:
            print ("%08X" % skip_function)
    
    debughook = MyDbgHook(modules_info, skip_functions, end_ea)
    
    pass

```



#### IDA附加应用

设备用的root真机，先执行IDA server

```
./data/local/tmp/android_server_70
```

转发端口

```
 adb forward tcp:23946 tcp:23946
```

#### IDA 加载脚本

IDA 附加后，可以通过菜单栏 File -> script file 加载脚本，或者alt + f7 快捷键加载。可以加载idc或者py脚本，这里加载py脚本。脚本成功加载后可以看到n2的起始和结束下好了断点，并且output 窗口会打印init日志。

```
================================================================================
-----------------------------------------------------------------------------------------
Python 2.7.13 (v2.7.13:a06454b1afa1, Dec 17 2016, 20:53:40) [MSC v.1500 64 bit (AMD64)] 
IDAPython v1.7.0 final (serial 0) (c) The IDAPython Team <idapython@googlegroups.com>
-----------------------------------------------------------------------------------------
The initial autoanalysis has been finished.
modules_info append BD442000 /data/app/~~Pjlr0l_3GCmvON7WegtO7Q==/com.gdufs.xman-2RHeB70T4wkejWQICMJLyw==/lib/arm/libmyjni.so 00006000
__init__
Caching 'Modules'... ok
Caching 'Modules'... ok
```

点击注册按钮，触发n2函数进入断点状态，然后在python窗口输入starthook(),并挂起其他线程suspend_other_thread()。

```
Python>starthook()
start_hook
Python>suspend_other_thread()
```

设置IDA trace的配置，点击Debugger --> Tracing --> options

![1](.\image\1.png)

不勾选Trace over debugger segments，勾选 Debugger --> Tracing --> instruction Tracing 

![2](.\image\2.png)

F9即可开始trace指令。如果trace到某条指令无法trace，断点后续的指令，并点击instruction Tracing。在trace xman 过程会遇到调用bl调用函数后无法继续trace，需要断点函数后的指令，然后点击instruction Tracing 继续trace。

#### 日志分析

```
00002961	                        	                                	R0=E5F00380 R1=FF7F391C R2=FF7F3920 R3=00000000 R4=E558808C R5=00000030 R6=BD871B82 R7=00000003 R8=00000000 R9=E26C5410 R10=FF7F39D8 R11=FF7F399C R12=BD59C1F9 SP=FF7F3910 LR=BD6EA0E7 PSR=20840030 D0= D1= D2= D3= D4= D5= D6= D7= D8= D9= D10= D11= D12= D13= D14= D15= D16= D17= D18= D19= D20= D21= D22= D23= D24= D25= D26= D27= D28= D29= D30= 	
00002961	libmyjni.so:n2          	PUSH.W          {R4-R10,LR}     	SP=FF7F38F0                             	
00002961	libmyjni.so:n2+4        	MOV             R6, R0          	R6=E5F00380                             	
00002961	libmyjni.so:n2+6        	LDR             R4, =(off_BD59FF80 - 0xBD59C20A)	R4=00003D76                             	
00002961	libmyjni.so:n2+8        	SUB             SP, SP, #0x18   	SP=FF7F38D8                             	
00002961	libmyjni.so:n2+A        	LDR             R0, =(aSdcardRegDat - 0xBD59C210)	R0=00001B98                             	
00002961	libmyjni.so:n2+C        	MOV             R9, R2          	R9=FF7F3920                             	
00002961	libmyjni.so:n2+E        	ADD             R4, PC; off_BD59FF80	R4=BD59FF80                             	
00002961	libmyjni.so:n2+10       	LDR             R4, [R4]; __stack_chk_guard	R4=E8AD3124                             	
00002961	libmyjni.so:n2+12       	LDR             R1, =(unk_BD59DDB8 - 0xBD59C214)	R1=00001BA4                             	
00002961	libmyjni.so:n2+14       	ADD             R0, PC; \"/sdcard/reg.dat\"	R0=BD59DDA8                             	
00002961	libmyjni.so:n2+16       	LDR             R3, [R4]        	R3=8F855664                             	
00002961	libmyjni.so:n2+18       	ADD             R1, PC; unk_BD59DDB8	R1=BD59DDB8                             	
00002961	libmyjni.so:n2+1A       	MOV             R8, R4          	R8=E8AD3124                             	
00002961	libmyjni.so:n2+1C       	STR             R3, [SP,#0x14]  	                                        	
00002961	libmyjni.so:n2+1E       	BLX             unk_BD59C0F0    	LR=BD59C21B T=0                         	
00002961	libmyjni.so:unk_BD59C0F0	ADRL            R12, 0xBD59F0F8 	R12=BD59F0F8                            	
00002961	libmyjni.so:BD59C0F8    	LDR             PC, [R12,#0xEC4]!	R12=BD59FFBC T=1                        	
00002961	libc.so:fopen           	PUSH            {R4-R7,LR}      	SP=FF7F38C4                             	
00002961	                        	                                	R0=E5D4001C R1=8F855664 R2=E8ABEEB5 R3=E8ABF591 R4=E8AD3124 R5=00000030 R6=E5F00380 R7=00000003 R8=E8AD3124 R9=FF7F3920 R10=FF7F39D8 R11=FF7F399C R12=E8ABEF0D SP=FF7F38D8 LR=E8A7FD2D PSR=60840030 D0= D1= D2= D3= D4= D5= D6= D7= D8= D9= D10= D11= D12= D13= D14= D15= D16= D17= D18= D19= D20= D21= D22= D23= D24= D25= D26= D27= D28= D29= D30= 	
00002961	libmyjni.so:n2+22       	MOV             R7, R0          	R7=E5D4001C                             	
00002961	libmyjni.so:n2+24       	CBNZ            R0, loc_BD59C23A	                                        	
00002961	libmyjni.so:loc_BD59C23A	LDR             R3, =(aW3AreWhoWeAre - 0xBD59C242)	R3=00001B9B                             	
00002961	libmyjni.so:n2+44       	MOV             R4, SP          	R4=FF7F38D8                             	
00002961	libmyjni.so:n2+46       	ADD             R3, PC; \"W3_arE_whO_we_ARE\"	R3=BD59DDDD                             	
00002961	libmyjni.so:n2+48       	ADD.W           R2, R3, #0x10   	R2=BD59DDED                             	
00002961	libmyjni.so:loc_BD59C244	LDR             R0, [R3]; \"W3_arE_whO_we_ARE\"	R0=615F3357                             	
00002961	libmyjni.so:n2+4E       	ADDS            R3, #8          	R3=BD59DDE5 PSR=80840030 C=0 Z=0 N=1    	
00002961	libmyjni.so:n2+50       	LDR.W           R1, [R3,#-4]    	R1=775F4572                             	
00002961	libmyjni.so:n2+54       	CMP             R3, R2          	                                        	
00002961	libmyjni.so:n2+56       	MOV             R5, R4          	R5=FF7F38D8                             	
00002961	libmyjni.so:n2+58       	STMIA           R5!, {R0,R1}    	R5=FF7F38E0                             	
00002961	libmyjni.so:n2+5A       	MOV             R4, R5          	R4=FF7F38E0                             	
00002961	libmyjni.so:n2+5C       	BNE             loc_BD59C244    	                                        	
00002961	libmyjni.so:loc_BD59C244	LDR             R0, [R3]; \"W3_arE_whO_we_ARE\"	R0=775F4F68                             	
00002961	libmyjni.so:n2+4E       	ADDS            R3, #8          	R3=BD59DDED                             	
00002961	libmyjni.so:n2+50       	LDR.W           R1, [R3,#-4]    	R1=52415F65                             	
00002961	libmyjni.so:n2+54       	CMP             R3, R2          	PSR=60840030 C=1 Z=1 N=0                	
00002961	libmyjni.so:n2+56       	MOV             R5, R4          	                                        	
00002961	libmyjni.so:n2+58       	STMIA           R5!, {R0,R1}    	R5=FF7F38E8                             	
00002961	libmyjni.so:n2+5A       	MOV             R4, R5          	R4=FF7F38E8                             	
00002961	libmyjni.so:n2+5C       	BNE             loc_BD59C244    	                                        	
00002961	libmyjni.so:n2+5E       	LDRH            R3, [R3]        	R3=00000045                             	
00002961	libmyjni.so:n2+60       	MOV             R1, R9          	R1=FF7F3920                             	
00002961	libmyjni.so:n2+62       	MOV             R0, R6          	R0=E5F00380                             	
00002961	libmyjni.so:n2+64       	MOVS            R2, #0          	R2=00000000                             	
00002961	libmyjni.so:n2+66       	MOV.W           R4, #0x7E0      	R4=000007E0                             	
00002961	libmyjni.so:n2+6A       	STRH            R3, [R5]        	                                        	
00002961	libmyjni.so:n2+6C       	LDR             R3, [R6]        	R3=E5CE214C                             	
00002961	libmyjni.so:n2+6E       	MOVS            R6, #0          	R6=00000000                             	
00002961	libmyjni.so:n2+70       	LDR.W           R3, [R3,#0x2A4] 	R3=E5A5E6E5                             	
00002961	libmyjni.so:n2+74       	BLX             R3              	LR=BD59C26F                             	
00002961	libart.so:_ZN3art12_GLOBAL__N_18CheckJNI17GetStringUTFCharsEP7_JNIEnvP8_jstringPh	PUSH            {R7,LR}         	SP=FF7F38D0                             	
00002961	                        	                                	R0=EB437830 R1=8F855664 R2=00430000 R3=E5880969 R4=000007E0 R5=FF7F38E8 R6=00000000 R7=E5D4001C R8=E8AD3124 R9=FF7F3920 R10=FF7F39D8 R11=FF7F399C R12=E8BF8FB4 SP=FF7F38D8 LR=E5A6DAF7 PSR=60840030 D0= D1= D2= D3= D4= D5= D6= D7= D8= D9= D10= D11= D12= D13= D14= D15= D16= D17= D18= D19= D20= D21= D22= D23= D24= D25= D26= D27= D28= D29= D30= 	
00002961	libmyjni.so:n2+76       	MOV             R9, R0          	R9=EB437830                             	
00002961	libmyjni.so:n2+78       	BLX             unk_BD59C0FC    	LR=BD59C275 T=0                         	
00002961	libmyjni.so:unk_BD59C0FC	ADRL            R12, 0xBD59F104 	R12=BD59F104                            	
00002961	libmyjni.so:BD59C104    	LDR             PC, [R12,#0xEBC]!	R12=BD59FFC0 T=1                        	
00002961	libc.so:strlen_a9       	PLD.W           [R0]            	                                        	
00002961	                        	                                	R0=00000006 R1=EB437838 R2=00000000 R3=FF006362 R4=000007E0 R5=FF7F38E8 R6=00000000 R7=E5D4001C R8=E8AD3124 R9=EB437830 R10=FF7F39D8 R11=FF7F399C R12=00800000 SP=FF7F38D8 LR=BD59C275 PSR=00840030 D0= D1= D2= D3= D4= D5= D6= D7= D8= D9= D10= D11= D12= D13= D14= D15= D16= D17= D18= D19= D20= D21= D22= D23= D24= D25= D26= D27= D28= D29= D30= 	
00002961	libmyjni.so:n2+7C       	MOV             R5, R9          	R5=EB437830                             	
00002961	libmyjni.so:n2+7E       	MOV             R10, R0         	R10=00000006                            	
00002961	libmyjni.so:loc_BD59C278	CMP             R6, R10         	PSR=80840030 N=1                        	
00002961	libmyjni.so:n2+82       	BGE             loc_BD59C2E0    	                                        	
00002961	libmyjni.so:n2+84       	MOV             R0, R6          	R0=00000000                             	
00002961	libmyjni.so:n2+86       	MOVS            R1, #3          	R1=00000003 PSR=00840030 N=0            	
00002961	libmyjni.so:n2+88       	BLX             unk_BD59C6C8    	LR=BD59C285 T=0                         	
00002961	libmyjni.so:unk_BD59C6C8	CMP             R1, #0          	C=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+130	BEQ             unk_BD59C6B8    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+134	STMFD           SP!, {R0,R1,LR} 	SP=FF7F38CC                             	
00002961	libmyjni.so:JNI_OnUnLoad+138	BL              unk_BD59C5F4    	LR=BD59C6D8                             	
00002961	libmyjni.so:unk_BD59C5F4	EOR             R12, R0, R1     	R12=00000003                            	
00002961	libmyjni.so:JNI_OnUnLoad+5C	RSBMI           R1, R1, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+60	SUBS            R2, R1, #1      	R2=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+64	BEQ             unk_BD59C684    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+68	MOVS            R3, R0          	R3=00000000 Z=1                         	
00002961	libmyjni.so:JNI_OnUnLoad+6C	RSBMI           R3, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+70	CMP             R3, R1          	PSR=80840010 C=0 Z=0 N=1                	
00002961	libmyjni.so:JNI_OnUnLoad+74	BLS             unk_BD59C690    	                                        	
00002961	libmyjni.so:unk_BD59C690	MOVCC           R0, #0          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+F8	MOVEQ           R0, R12,ASR#31  	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+FC	ORREQ           R0, R0, #1      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+100	BX              LR              	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+13C	LDMFD           SP!, {R1,R2,LR} 	R1=00000000 R2=00000003 SP=FF7F38D8 LR=BD59C285 	
00002961	libmyjni.so:JNI_OnUnLoad+140	MUL             R3, R2, R0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+144	SUB             R1, R1, R3      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+148	BX              LR              	T=1                                     	
00002961	libmyjni.so:n2+8C       	CMP             R1, #1          	                                        	
00002961	libmyjni.so:n2+8E       	BNE             loc_BD59C2A8    	                                        	
00002961	libmyjni.so:loc_BD59C2A8	CMP             R1, #2          	                                        	
00002961	libmyjni.so:n2+B2       	BNE             loc_BD59C2C0    	                                        	
00002961	libmyjni.so:loc_BD59C2C0	ADDS            R0, R4, #3      	R0=000007E3 PSR=00840030 N=0            	
00002961	libmyjni.so:n2+CA       	MOVS            R1, #0xD        	R1=0000000D                             	
00002961	libmyjni.so:n2+CC       	BLX             unk_BD59C6C8    	LR=BD59C2C9 T=0                         	
00002961	libmyjni.so:unk_BD59C6C8	CMP             R1, #0          	C=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+130	BEQ             unk_BD59C6B8    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+134	STMFD           SP!, {R0,R1,LR} 	SP=FF7F38CC                             	
00002961	libmyjni.so:JNI_OnUnLoad+138	BL              unk_BD59C5F4    	LR=BD59C6D8                             	
00002961	libmyjni.so:unk_BD59C5F4	EOR             R12, R0, R1     	R12=000007EE                            	
00002961	libmyjni.so:JNI_OnUnLoad+5C	RSBMI           R1, R1, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+60	SUBS            R2, R1, #1      	R2=0000000C                             	
00002961	libmyjni.so:JNI_OnUnLoad+64	BEQ             unk_BD59C684    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+68	MOVS            R3, R0          	R3=000007E3                             	
00002961	libmyjni.so:JNI_OnUnLoad+6C	RSBMI           R3, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+70	CMP             R3, R1          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+74	BLS             unk_BD59C690    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+78	TST             R1, R2          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+7C	BEQ             unk_BD59C6A0    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+80	CLZ             R2, R1          	R2=0000001C                             	
00002961	libmyjni.so:JNI_OnUnLoad+84	CLZ             R0, R3          	R0=00000015                             	
00002961	libmyjni.so:JNI_OnUnLoad+88	SUB             R0, R2, R0      	R0=00000007                             	
00002961	libmyjni.so:JNI_OnUnLoad+8C	MOV             R2, #1          	R2=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+90	MOV             R1, R1,LSL R0   	R1=00000680                             	
00002961	libmyjni.so:JNI_OnUnLoad+94	MOV             R2, R2,LSL R0   	R2=00000080                             	
00002961	libmyjni.so:JNI_OnUnLoad+98	MOV             R0, #0          	R0=00000000                             	
00002961	libmyjni.so:JNI_OnUnLoad+9C	CMP             R3, R1          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+A0	SUBCS           R3, R3, R1      	R3=00000163                             	
00002961	libmyjni.so:JNI_OnUnLoad+A4	ORRCS           R0, R0, R2      	R0=00000080                             	
00002961	libmyjni.so:JNI_OnUnLoad+A8	CMP             R3, R1,LSR#1    	PSR=80840010 C=0 N=1                    	
00002961	libmyjni.so:JNI_OnUnLoad+AC	SUBCS           R3, R3, R1,LSR#1	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+B0	ORRCS           R0, R0, R2,LSR#1	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+B4	CMP             R3, R1,LSR#2    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+B8	SUBCS           R3, R3, R1,LSR#2	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+BC	ORRCS           R0, R0, R2,LSR#2	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C0	CMP             R3, R1,LSR#3    	PSR=20840010 C=1 N=0                    	
00002961	libmyjni.so:JNI_OnUnLoad+C4	SUBCS           R3, R3, R1,LSR#3	R3=00000093                             	
00002961	libmyjni.so:JNI_OnUnLoad+C8	ORRCS           R0, R0, R2,LSR#3	R0=00000090                             	
00002961	libmyjni.so:JNI_OnUnLoad+CC	CMP             R3, #0          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+D0	MOVNES          R2, R2,LSR#4    	R2=00000008 C=0                         	
00002961	libmyjni.so:JNI_OnUnLoad+D4	MOVNE           R1, R1,LSR#4    	R1=00000068                             	
00002961	libmyjni.so:JNI_OnUnLoad+D8	BNE             unk_BD59C638    	                                        	
00002961	libmyjni.so:unk_BD59C638	CMP             R3, R1          	C=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+A0	SUBCS           R3, R3, R1      	R3=0000002B                             	
00002961	libmyjni.so:JNI_OnUnLoad+A4	ORRCS           R0, R0, R2      	R0=00000098                             	
00002961	libmyjni.so:JNI_OnUnLoad+A8	CMP             R3, R1,LSR#1    	PSR=80840010 C=0 N=1                    	
00002961	libmyjni.so:JNI_OnUnLoad+AC	SUBCS           R3, R3, R1,LSR#1	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+B0	ORRCS           R0, R0, R2,LSR#1	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+B4	CMP             R3, R1,LSR#2    	PSR=20840010 C=1 N=0                    	
00002961	libmyjni.so:JNI_OnUnLoad+B8	SUBCS           R3, R3, R1,LSR#2	R3=00000011                             	
00002961	libmyjni.so:JNI_OnUnLoad+BC	ORRCS           R0, R0, R2,LSR#2	R0=0000009A                             	
00002961	libmyjni.so:JNI_OnUnLoad+C0	CMP             R3, R1,LSR#3    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C4	SUBCS           R3, R3, R1,LSR#3	R3=00000004                             	
00002961	libmyjni.so:JNI_OnUnLoad+C8	ORRCS           R0, R0, R2,LSR#3	R0=0000009B                             	
00002961	libmyjni.so:JNI_OnUnLoad+CC	CMP             R3, #0          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+D0	MOVNES          R2, R2,LSR#4    	R2=00000000 Z=1                         	
00002961	libmyjni.so:JNI_OnUnLoad+D4	MOVNE           R1, R1,LSR#4    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+D8	BNE             unk_BD59C638    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+DC	CMP             R12, #0         	Z=0                                     	
00002961	libmyjni.so:JNI_OnUnLoad+E0	RSBMI           R0, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+E4	BX              LR              	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+13C	LDMFD           SP!, {R1,R2,LR} 	R1=000007E3 R2=0000000D SP=FF7F38D8 LR=BD59C2C9 	
00002961	libmyjni.so:JNI_OnUnLoad+140	MUL             R3, R2, R0      	R3=000007DF                             	
00002961	libmyjni.so:JNI_OnUnLoad+144	SUB             R1, R1, R3      	R1=00000004                             	
00002961	libmyjni.so:JNI_OnUnLoad+148	BX              LR              	T=1                                     	
00002961	libmyjni.so:n2+D0       	ADD             R3, SP, #0x18   	R3=FF7F38F0                             	
00002961	libmyjni.so:n2+D2       	ADD             R3, R1          	R3=FF7F38F4                             	
00002961	libmyjni.so:n2+D4       	MOV             R4, R1          	R4=00000004                             	
00002961	libmyjni.so:n2+D6       	LDRB.W          R2, [R3,#-0x15] 	R2=00000077                             	
00002961	libmyjni.so:loc_BD59C2D2	LDRB            R3, [R5]        	R3=00000031           ;debug input[0]                  	
00002961	libmyjni.so:n2+DC       	ADDS            R6, #1          	R6=00000001 C=0                         	
00002961	libmyjni.so:n2+DE       	ADDS            R5, #1          	R5=EB437831 PSR=80840030 N=1            	
00002961	libmyjni.so:n2+E0       	EORS            R3, R2          	R3=00000046 PSR=00840030 N=0   ;debug result[0]         	
00002961	libmyjni.so:n2+E2       	STRB.W          R3, [R5,#-1]    	                                        	
00002961	libmyjni.so:n2+E6       	B               loc_BD59C278    	                                        	
00002961	libmyjni.so:loc_BD59C278	CMP             R6, R10         	PSR=80840030 N=1                        	
00002961	libmyjni.so:n2+82       	BGE             loc_BD59C2E0    	                                        	
00002961	libmyjni.so:n2+84       	MOV             R0, R6          	R0=00000001                             	
00002961	libmyjni.so:n2+86       	MOVS            R1, #3          	R1=00000003 PSR=00840030 N=0            	
00002961	libmyjni.so:n2+88       	BLX             unk_BD59C6C8    	LR=BD59C285 T=0                         	
00002961	libmyjni.so:unk_BD59C6C8	CMP             R1, #0          	C=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+130	BEQ             unk_BD59C6B8    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+134	STMFD           SP!, {R0,R1,LR} 	SP=FF7F38CC                             	
00002961	libmyjni.so:JNI_OnUnLoad+138	BL              unk_BD59C5F4    	LR=BD59C6D8                             	
00002961	libmyjni.so:unk_BD59C5F4	EOR             R12, R0, R1     	R12=00000002                            	
00002961	libmyjni.so:JNI_OnUnLoad+5C	RSBMI           R1, R1, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+60	SUBS            R2, R1, #1      	R2=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+64	BEQ             unk_BD59C684    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+68	MOVS            R3, R0          	R3=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+6C	RSBMI           R3, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+70	CMP             R3, R1          	PSR=80840010 C=0 N=1                    	
00002961	libmyjni.so:JNI_OnUnLoad+74	BLS             unk_BD59C690    	                                        	
00002961	libmyjni.so:unk_BD59C690	MOVCC           R0, #0          	R0=00000000                             	
00002961	libmyjni.so:JNI_OnUnLoad+F8	MOVEQ           R0, R12,ASR#31  	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+FC	ORREQ           R0, R0, #1      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+100	BX              LR              	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+13C	LDMFD           SP!, {R1,R2,LR} 	R1=00000001 R2=00000003 SP=FF7F38D8 LR=BD59C285 	
00002961	libmyjni.so:JNI_OnUnLoad+140	MUL             R3, R2, R0      	R3=00000000                             	
00002961	libmyjni.so:JNI_OnUnLoad+144	SUB             R1, R1, R3      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+148	BX              LR              	T=1                                     	
00002961	libmyjni.so:n2+8C       	CMP             R1, #1          	PSR=60840030 C=1 Z=1 N=0                	
00002961	libmyjni.so:n2+8E       	BNE             loc_BD59C2A8    	                                        	
00002961	libmyjni.so:n2+90       	LDR             R3, =0x8000000F 	R3=8000000F                             	
00002961	libmyjni.so:n2+92       	ADDS            R4, #5          	R4=00000009 C=0 Z=0                     	
00002961	libmyjni.so:n2+94       	ANDS            R3, R4          	R3=00000009                             	
00002961	libmyjni.so:n2+96       	CMP             R3, #0          	C=1                                     	
00002961	libmyjni.so:n2+98       	ITTT LT                         	                                        	
00002961	libmyjni.so:n2+A4       	MOV             R4, R3          	                                        	
00002961	libmyjni.so:n2+A6       	ADD             R3, SP, #0x18   	R3=FF7F38F0                             	
00002961	libmyjni.so:n2+A8       	ADD             R3, R4          	R3=FF7F38F9                             	
00002961	libmyjni.so:n2+AA       	LDRB.W          R2, [R3,#-0x17] 	R2=0000005F                             	
00002961	libmyjni.so:n2+AE       	B               loc_BD59C2D2    	                                        	
00002961	libmyjni.so:loc_BD59C2D2	LDRB            R3, [R5]        	R3=00000032           ;debug input[1]                  	
00002961	libmyjni.so:n2+DC       	ADDS            R6, #1          	R6=00000002 C=0                         	
00002961	libmyjni.so:n2+DE       	ADDS            R5, #1          	R5=EB437832 PSR=80840030 N=1            	
00002961	libmyjni.so:n2+E0       	EORS            R3, R2          	R3=0000006D PSR=00840030 N=0   ;debug result[1]         	
00002961	libmyjni.so:n2+E2       	STRB.W          R3, [R5,#-1]    	                                        	
00002961	libmyjni.so:n2+E6       	B               loc_BD59C278    	                                        	
00002961	libmyjni.so:loc_BD59C278	CMP             R6, R10         	PSR=80840030 N=1                        	
00002961	libmyjni.so:n2+82       	BGE             loc_BD59C2E0    	                                        	
00002961	libmyjni.so:n2+84       	MOV             R0, R6          	R0=00000002                             	
00002961	libmyjni.so:n2+86       	MOVS            R1, #3          	R1=00000003 PSR=00840030 N=0            	
00002961	libmyjni.so:n2+88       	BLX             unk_BD59C6C8    	T=0                                     	
00002961	libmyjni.so:unk_BD59C6C8	CMP             R1, #0          	C=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+130	BEQ             unk_BD59C6B8    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+134	STMFD           SP!, {R0,R1,LR} 	SP=FF7F38CC                             	
00002961	libmyjni.so:JNI_OnUnLoad+138	BL              unk_BD59C5F4    	LR=BD59C6D8                             	
00002961	libmyjni.so:unk_BD59C5F4	EOR             R12, R0, R1     	R12=00000001                            	
00002961	libmyjni.so:JNI_OnUnLoad+5C	RSBMI           R1, R1, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+60	SUBS            R2, R1, #1      	R2=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+64	BEQ             unk_BD59C684    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+68	MOVS            R3, R0          	R3=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+6C	RSBMI           R3, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+70	CMP             R3, R1          	PSR=80840010 C=0 N=1                    	
00002961	libmyjni.so:JNI_OnUnLoad+74	BLS             unk_BD59C690    	                                        	
00002961	libmyjni.so:unk_BD59C690	MOVCC           R0, #0          	R0=00000000                             	
00002961	libmyjni.so:JNI_OnUnLoad+F8	MOVEQ           R0, R12,ASR#31  	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+FC	ORREQ           R0, R0, #1      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+100	BX              LR              	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+13C	LDMFD           SP!, {R1,R2,LR} 	R1=00000002 R2=00000003 SP=FF7F38D8 LR=BD59C285 	
00002961	libmyjni.so:JNI_OnUnLoad+140	MUL             R3, R2, R0      	R3=00000000                             	
00002961	libmyjni.so:JNI_OnUnLoad+144	SUB             R1, R1, R3      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+148	BX              LR              	T=1                                     	
00002961	libmyjni.so:n2+8C       	CMP             R1, #1          	PSR=20840030 C=1 N=0                    	
00002961	libmyjni.so:n2+8E       	BNE             loc_BD59C2A8    	                                        	
00002961	libmyjni.so:loc_BD59C2A8	CMP             R1, #2          	Z=1                                     	
00002961	libmyjni.so:n2+B2       	BNE             loc_BD59C2C0    	                                        	
00002961	libmyjni.so:n2+B4       	ADDS            R0, R4, #7      	R0=00000010 C=0 Z=0                     	
00002961	libmyjni.so:n2+B6       	MOVS            R1, #0xF        	R1=0000000F                             	
00002961	libmyjni.so:n2+B8       	BLX             unk_BD59C6C8    	LR=BD59C2B5 T=0                         	
00002961	libmyjni.so:unk_BD59C6C8	CMP             R1, #0          	C=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+130	BEQ             unk_BD59C6B8    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+134	STMFD           SP!, {R0,R1,LR} 	SP=FF7F38CC                             	
00002961	libmyjni.so:JNI_OnUnLoad+138	BL              unk_BD59C5F4    	LR=BD59C6D8                             	
00002961	libmyjni.so:unk_BD59C5F4	EOR             R12, R0, R1     	R12=0000001F                            	
00002961	libmyjni.so:JNI_OnUnLoad+5C	RSBMI           R1, R1, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+60	SUBS            R2, R1, #1      	R2=0000000E                             	
00002961	libmyjni.so:JNI_OnUnLoad+64	BEQ             unk_BD59C684    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+68	MOVS            R3, R0          	R3=00000010                             	
00002961	libmyjni.so:JNI_OnUnLoad+6C	RSBMI           R3, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+70	CMP             R3, R1          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+74	BLS             unk_BD59C690    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+78	TST             R1, R2          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+7C	BEQ             unk_BD59C6A0    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+80	CLZ             R2, R1          	R2=0000001C                             	
00002961	libmyjni.so:JNI_OnUnLoad+84	CLZ             R0, R3          	R0=0000001B                             	
00002961	libmyjni.so:JNI_OnUnLoad+88	SUB             R0, R2, R0      	R0=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+8C	MOV             R2, #1          	R2=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+90	MOV             R1, R1,LSL R0   	R1=0000001E                             	
00002961	libmyjni.so:JNI_OnUnLoad+94	MOV             R2, R2,LSL R0   	R2=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+98	MOV             R0, #0          	R0=00000000                             	
00002961	libmyjni.so:unk_BD59C638	CMP             R3, R1          	PSR=80840010 C=0 N=1                    	
00002961	libmyjni.so:JNI_OnUnLoad+A0	SUBCS           R3, R3, R1      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+A4	ORRCS           R0, R0, R2      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+A8	CMP             R3, R1,LSR#1    	PSR=20840010 C=1 N=0                    	
00002961	libmyjni.so:JNI_OnUnLoad+AC	SUBCS           R3, R3, R1,LSR#1	R3=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+B0	ORRCS           R0, R0, R2,LSR#1	R0=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+B4	CMP             R3, R1,LSR#2    	PSR=80840010 C=0 N=1                    	
00002961	libmyjni.so:JNI_OnUnLoad+B8	SUBCS           R3, R3, R1,LSR#2	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+BC	ORRCS           R0, R0, R2,LSR#2	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C0	CMP             R3, R1,LSR#3    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C4	SUBCS           R3, R3, R1,LSR#3	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C8	ORRCS           R0, R0, R2,LSR#3	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+CC	CMP             R3, #0          	PSR=20840010 C=1 N=0                    	
00002961	libmyjni.so:JNI_OnUnLoad+D0	MOVNES          R2, R2,LSR#4    	R2=00000000 C=0 Z=1                     	
00002961	libmyjni.so:JNI_OnUnLoad+D4	MOVNE           R1, R1,LSR#4    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+D8	BNE             unk_BD59C638    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+DC	CMP             R12, #0         	C=1 Z=0                                 	
00002961	libmyjni.so:JNI_OnUnLoad+E0	RSBMI           R0, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+E4	BX              LR              	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+13C	LDMFD           SP!, {R1,R2,LR} 	R1=00000010 R2=0000000F SP=FF7F38D8 LR=BD59C2B5 	
00002961	libmyjni.so:JNI_OnUnLoad+140	MUL             R3, R2, R0      	R3=0000000F                             	
00002961	libmyjni.so:JNI_OnUnLoad+144	SUB             R1, R1, R3      	R1=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+148	BX              LR              	T=1                                     	
00002961	libmyjni.so:n2+BC       	ADD             R3, SP, #0x18   	R3=FF7F38F0                             	
00002961	libmyjni.so:n2+BE       	ADD             R3, R1          	R3=FF7F38F1                             	
00002961	libmyjni.so:n2+C0       	MOV             R4, R1          	R4=00000001                             	
00002961	libmyjni.so:n2+C2       	LDRB.W          R2, [R3,#-0x16] 	R2=00000061                             	
00002961	libmyjni.so:n2+C6       	B               loc_BD59C2D2    	                                        	
00002961	libmyjni.so:loc_BD59C2D2	LDRB            R3, [R5]        	R3=00000033      ;debug input[2]                       	
00002961	libmyjni.so:n2+DC       	ADDS            R6, #1          	R6=00000003 C=0                         	
00002961	libmyjni.so:n2+DE       	ADDS            R5, #1          	R5=EB437833 PSR=80840030 N=1            	
00002961	libmyjni.so:n2+E0       	EORS            R3, R2          	R3=00000052 PSR=00840030 N=0    ;debug result[2]        	
00002961	libmyjni.so:n2+E2       	STRB.W          R3, [R5,#-1]    	                                        	
00002961	libmyjni.so:n2+E6       	B               loc_BD59C278    	                                        	
00002961	libmyjni.so:loc_BD59C278	CMP             R6, R10         	PSR=80840030 N=1                        	
00002961	libmyjni.so:n2+82       	BGE             loc_BD59C2E0    	                                        	
00002961	libmyjni.so:n2+84       	MOV             R0, R6          	R0=00000003                             	
00002961	libmyjni.so:n2+86       	MOVS            R1, #3          	R1=00000003 PSR=00840030 N=0            	
00002961	libmyjni.so:n2+88       	BLX             unk_BD59C6C8    	LR=BD59C285 T=0                         	
00002961	libmyjni.so:unk_BD59C6C8	CMP             R1, #0          	C=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+130	BEQ             unk_BD59C6B8    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+134	STMFD           SP!, {R0,R1,LR} 	SP=FF7F38CC                             	
00002961	libmyjni.so:JNI_OnUnLoad+138	BL              unk_BD59C5F4    	LR=BD59C6D8                             	
00002961	libmyjni.so:unk_BD59C5F4	EOR             R12, R0, R1     	R12=00000000                            	
00002961	libmyjni.so:JNI_OnUnLoad+5C	RSBMI           R1, R1, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+60	SUBS            R2, R1, #1      	R2=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+64	BEQ             unk_BD59C684    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+68	MOVS            R3, R0          	R3=00000003                             	
00002961	libmyjni.so:JNI_OnUnLoad+6C	RSBMI           R3, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+70	CMP             R3, R1          	Z=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+74	BLS             unk_BD59C690    	                                        	
00002961	libmyjni.so:unk_BD59C690	MOVCC           R0, #0          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+F8	MOVEQ           R0, R12,ASR#31  	R0=00000000                             	
00002961	libmyjni.so:JNI_OnUnLoad+FC	ORREQ           R0, R0, #1      	R0=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+100	BX              LR              	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+13C	LDMFD           SP!, {R1,R2,LR} 	R2=00000003 SP=FF7F38D8 LR=BD59C285     	
00002961	libmyjni.so:JNI_OnUnLoad+140	MUL             R3, R2, R0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+144	SUB             R1, R1, R3      	R1=00000000                             	
00002961	libmyjni.so:JNI_OnUnLoad+148	BX              LR              	T=1                                     	
00002961	libmyjni.so:n2+8C       	CMP             R1, #1          	PSR=80840030 C=0 Z=0 N=1                	
00002961	libmyjni.so:n2+8E       	BNE             loc_BD59C2A8    	                                        	
00002961	libmyjni.so:loc_BD59C2A8	CMP             R1, #2          	                                        	
00002961	libmyjni.so:n2+B2       	BNE             loc_BD59C2C0    	                                        	
00002961	libmyjni.so:loc_BD59C2C0	ADDS            R0, R4, #3      	R0=00000004 PSR=00840030 N=0            	
00002961	libmyjni.so:n2+CA       	MOVS            R1, #0xD        	R1=0000000D                             	
00002961	libmyjni.so:n2+CC       	BLX             unk_BD59C6C8    	LR=BD59C2C9 T=0                         	
00002961	libmyjni.so:unk_BD59C6C8	CMP             R1, #0          	C=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+130	BEQ             unk_BD59C6B8    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+134	STMFD           SP!, {R0,R1,LR} 	SP=FF7F38CC                             	
00002961	libmyjni.so:JNI_OnUnLoad+138	BL              unk_BD59C5F4    	LR=BD59C6D8                             	
00002961	libmyjni.so:unk_BD59C5F4	EOR             R12, R0, R1     	R12=00000009                            	
00002961	libmyjni.so:JNI_OnUnLoad+5C	RSBMI           R1, R1, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+60	SUBS            R2, R1, #1      	R2=0000000C                             	
00002961	libmyjni.so:JNI_OnUnLoad+64	BEQ             unk_BD59C684    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+68	MOVS            R3, R0          	R3=00000004                             	
00002961	libmyjni.so:JNI_OnUnLoad+6C	RSBMI           R3, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+70	CMP             R3, R1          	PSR=80840010 C=0 N=1                    	
00002961	libmyjni.so:JNI_OnUnLoad+74	BLS             unk_BD59C690    	                                        	
00002961	libmyjni.so:unk_BD59C690	MOVCC           R0, #0          	R0=00000000                             	
00002961	libmyjni.so:JNI_OnUnLoad+F8	MOVEQ           R0, R12,ASR#31  	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+FC	ORREQ           R0, R0, #1      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+100	BX              LR              	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+13C	LDMFD           SP!, {R1,R2,LR} 	R1=00000004 R2=0000000D SP=FF7F38D8 LR=BD59C2C9 	
00002961	libmyjni.so:JNI_OnUnLoad+140	MUL             R3, R2, R0      	R3=00000000                             	
00002961	libmyjni.so:JNI_OnUnLoad+144	SUB             R1, R1, R3      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+148	BX              LR              	T=1                                     	
00002961	libmyjni.so:n2+D0       	ADD             R3, SP, #0x18   	R3=FF7F38F0                             	
00002961	libmyjni.so:n2+D2       	ADD             R3, R1          	R3=FF7F38F4                             	
00002961	libmyjni.so:n2+D4       	MOV             R4, R1          	R4=00000004                             	
00002961	libmyjni.so:n2+D6       	LDRB.W          R2, [R3,#-0x15] 	R2=00000077                             	
00002961	libmyjni.so:loc_BD59C2D2	LDRB            R3, [R5]        	R3=00000061             ;debug input[3]                	
00002961	libmyjni.so:n2+DC       	ADDS            R6, #1          	R6=00000004 PSR=00840030 N=0            	
00002961	libmyjni.so:n2+DE       	ADDS            R5, #1          	R5=EB437834 PSR=80840030 N=1            	
00002961	libmyjni.so:n2+E0       	EORS            R3, R2          	R3=00000016 PSR=00840030 N=0         ;debug result[3]    	
00002961	libmyjni.so:n2+E2       	STRB.W          R3, [R5,#-1]    	                                        	
00002961	libmyjni.so:n2+E6       	B               loc_BD59C278    	                                        	
00002961	libmyjni.so:loc_BD59C278	CMP             R6, R10         	PSR=80840030 N=1                        	
00002961	libmyjni.so:n2+82       	BGE             loc_BD59C2E0    	                                        	
00002961	libmyjni.so:n2+84       	MOV             R0, R6          	R0=00000004                             	
00002961	libmyjni.so:n2+86       	MOVS            R1, #3          	R1=00000003 PSR=00840030 N=0            	
00002961	libmyjni.so:n2+88       	BLX             unk_BD59C6C8    	LR=BD59C285 T=0                         	
00002961	libmyjni.so:unk_BD59C6C8	CMP             R1, #0          	C=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+130	BEQ             unk_BD59C6B8    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+134	STMFD           SP!, {R0,R1,LR} 	SP=FF7F38CC                             	
00002961	libmyjni.so:JNI_OnUnLoad+138	BL              unk_BD59C5F4    	LR=BD59C6D8                             	
00002961	libmyjni.so:unk_BD59C5F4	EOR             R12, R0, R1     	R12=00000007                            	
00002961	libmyjni.so:JNI_OnUnLoad+5C	RSBMI           R1, R1, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+60	SUBS            R2, R1, #1      	R2=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+64	BEQ             unk_BD59C684    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+68	MOVS            R3, R0          	R3=00000004                             	
00002961	libmyjni.so:JNI_OnUnLoad+6C	RSBMI           R3, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+70	CMP             R3, R1          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+74	BLS             unk_BD59C690    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+78	TST             R1, R2          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+7C	BEQ             unk_BD59C6A0    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+80	CLZ             R2, R1          	R2=0000001E                             	
00002961	libmyjni.so:JNI_OnUnLoad+84	CLZ             R0, R3          	R0=0000001D                             	
00002961	libmyjni.so:JNI_OnUnLoad+88	SUB             R0, R2, R0      	R0=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+8C	MOV             R2, #1          	R2=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+90	MOV             R1, R1,LSL R0   	R1=00000006                             	
00002961	libmyjni.so:JNI_OnUnLoad+94	MOV             R2, R2,LSL R0   	R2=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+98	MOV             R0, #0          	R0=00000000                             	
00002961	libmyjni.so:unk_BD59C638	CMP             R3, R1          	PSR=80840010 C=0 N=1                    	
00002961	libmyjni.so:JNI_OnUnLoad+A0	SUBCS           R3, R3, R1      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+A4	ORRCS           R0, R0, R2      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+A8	CMP             R3, R1,LSR#1    	PSR=20840010 C=1 N=0                    	
00002961	libmyjni.so:JNI_OnUnLoad+AC	SUBCS           R3, R3, R1,LSR#1	R3=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+B0	ORRCS           R0, R0, R2,LSR#1	R0=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+B4	CMP             R3, R1,LSR#2    	Z=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+B8	SUBCS           R3, R3, R1,LSR#2	R3=00000000                             	
00002961	libmyjni.so:JNI_OnUnLoad+BC	ORRCS           R0, R0, R2,LSR#2	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C0	CMP             R3, R1,LSR#3    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C4	SUBCS           R3, R3, R1,LSR#3	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C8	ORRCS           R0, R0, R2,LSR#3	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+CC	CMP             R3, #0          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+D0	MOVNES          R2, R2,LSR#4    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+D4	MOVNE           R1, R1,LSR#4    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+D8	BNE             unk_BD59C638    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+DC	CMP             R12, #0         	Z=0                                     	
00002961	libmyjni.so:JNI_OnUnLoad+E0	RSBMI           R0, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+E4	BX              LR              	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+13C	LDMFD           SP!, {R1,R2,LR} 	R1=00000004 R2=00000003 SP=FF7F38D8 LR=BD59C285 	
00002961	libmyjni.so:JNI_OnUnLoad+140	MUL             R3, R2, R0      	R3=00000003                             	
00002961	libmyjni.so:JNI_OnUnLoad+144	SUB             R1, R1, R3      	R1=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+148	BX              LR              	T=1                                     	
00002961	libmyjni.so:n2+8C       	CMP             R1, #1          	Z=1                                     	
00002961	libmyjni.so:n2+8E       	BNE             loc_BD59C2A8    	                                        	
00002961	libmyjni.so:n2+90       	LDR             R3, =0x8000000F 	R3=8000000F                             	
00002961	libmyjni.so:n2+92       	ADDS            R4, #5          	R4=00000009 C=0 Z=0                     	
00002961	libmyjni.so:n2+94       	ANDS            R3, R4          	R3=00000009                             	
00002961	libmyjni.so:n2+96       	CMP             R3, #0          	C=1                                     	
00002961	libmyjni.so:n2+98       	ITTT LT                         	                                        	
00002961	libmyjni.so:n2+A4       	MOV             R4, R3          	                                        	
00002961	libmyjni.so:n2+A6       	ADD             R3, SP, #0x18   	R3=FF7F38F0                             	
00002961	libmyjni.so:n2+A8       	ADD             R3, R4          	R3=FF7F38F9                             	
00002961	libmyjni.so:n2+AA       	LDRB.W          R2, [R3,#-0x17] 	R2=0000005F                             	
00002961	libmyjni.so:n2+AE       	B               loc_BD59C2D2    	                                        	
00002961	libmyjni.so:loc_BD59C2D2	LDRB            R3, [R5]        	R3=00000062          ;debug input[4]                   	
00002961	libmyjni.so:n2+DC       	ADDS            R6, #1          	R6=00000005 C=0                         	
00002961	libmyjni.so:n2+DE       	ADDS            R5, #1          	R5=EB437835 PSR=80840030 N=1            	
00002961	libmyjni.so:n2+E0       	EORS            R3, R2          	R3=0000003D PSR=00840030 N=0            ;debug result[4] 	
00002961	libmyjni.so:n2+E2       	STRB.W          R3, [R5,#-1]    	                                        	
00002961	libmyjni.so:n2+E6       	B               loc_BD59C278    	                                        	
00002961	libmyjni.so:loc_BD59C278	CMP             R6, R10         	PSR=80840030 N=1                        	
00002961	libmyjni.so:n2+82       	BGE             loc_BD59C2E0    	                                        	
00002961	libmyjni.so:n2+84       	MOV             R0, R6          	R0=00000005                             	
00002961	libmyjni.so:n2+86       	MOVS            R1, #3          	R1=00000003 PSR=00840030 N=0            	
00002961	libmyjni.so:n2+88       	BLX             unk_BD59C6C8    	T=0                                     	
00002961	libmyjni.so:unk_BD59C6C8	CMP             R1, #0          	C=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+130	BEQ             unk_BD59C6B8    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+134	STMFD           SP!, {R0,R1,LR} 	SP=FF7F38CC                             	
00002961	libmyjni.so:JNI_OnUnLoad+138	BL              unk_BD59C5F4    	LR=BD59C6D8                             	
00002961	libmyjni.so:unk_BD59C5F4	EOR             R12, R0, R1     	R12=00000006                            	
00002961	libmyjni.so:JNI_OnUnLoad+5C	RSBMI           R1, R1, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+60	SUBS            R2, R1, #1      	R2=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+64	BEQ             unk_BD59C684    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+68	MOVS            R3, R0          	R3=00000005                             	
00002961	libmyjni.so:JNI_OnUnLoad+6C	RSBMI           R3, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+70	CMP             R3, R1          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+74	BLS             unk_BD59C690    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+78	TST             R1, R2          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+7C	BEQ             unk_BD59C6A0    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+80	CLZ             R2, R1          	R2=0000001E                             	
00002961	libmyjni.so:JNI_OnUnLoad+84	CLZ             R0, R3          	R0=0000001D                             	
00002961	libmyjni.so:JNI_OnUnLoad+88	SUB             R0, R2, R0      	R0=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+8C	MOV             R2, #1          	R2=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+90	MOV             R1, R1,LSL R0   	R1=00000006                             	
00002961	libmyjni.so:JNI_OnUnLoad+94	MOV             R2, R2,LSL R0   	R2=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+98	MOV             R0, #0          	R0=00000000                             	
00002961	libmyjni.so:unk_BD59C638	CMP             R3, R1          	PSR=80840010 C=0 N=1                    	
00002961	libmyjni.so:JNI_OnUnLoad+A0	SUBCS           R3, R3, R1      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+A4	ORRCS           R0, R0, R2      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+A8	CMP             R3, R1,LSR#1    	PSR=20840010 C=1 N=0                    	
00002961	libmyjni.so:JNI_OnUnLoad+AC	SUBCS           R3, R3, R1,LSR#1	R3=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+B0	ORRCS           R0, R0, R2,LSR#1	R0=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+B4	CMP             R3, R1,LSR#2    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+B8	SUBCS           R3, R3, R1,LSR#2	R3=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+BC	ORRCS           R0, R0, R2,LSR#2	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C0	CMP             R3, R1,LSR#3    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C4	SUBCS           R3, R3, R1,LSR#3	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C8	ORRCS           R0, R0, R2,LSR#3	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+CC	CMP             R3, #0          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+D0	MOVNES          R2, R2,LSR#4    	R2=00000000 C=0 Z=1                     	
00002961	libmyjni.so:JNI_OnUnLoad+D4	MOVNE           R1, R1,LSR#4    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+D8	BNE             unk_BD59C638    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+DC	CMP             R12, #0         	C=1 Z=0                                 	
00002961	libmyjni.so:JNI_OnUnLoad+E0	RSBMI           R0, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+E4	BX              LR              	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+13C	LDMFD           SP!, {R1,R2,LR} 	R1=00000005 R2=00000003 SP=FF7F38D8 LR=BD59C285 	
00002961	libmyjni.so:JNI_OnUnLoad+140	MUL             R3, R2, R0      	R3=00000003                             	
00002961	libmyjni.so:JNI_OnUnLoad+144	SUB             R1, R1, R3      	R1=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+148	BX              LR              	T=1                                     	
00002961	libmyjni.so:n2+8C       	CMP             R1, #1          	                                        	
00002961	libmyjni.so:n2+8E       	BNE             loc_BD59C2A8    	                                        	
00002961	libmyjni.so:loc_BD59C2A8	CMP             R1, #2          	Z=1                                     	
00002961	libmyjni.so:n2+B2       	BNE             loc_BD59C2C0    	                                        	
00002961	libmyjni.so:n2+B4       	ADDS            R0, R4, #7      	R0=00000010 C=0 Z=0                     	
00002961	libmyjni.so:n2+B6       	MOVS            R1, #0xF        	R1=0000000F                             	
00002961	libmyjni.so:n2+B8       	BLX             unk_BD59C6C8    	LR=BD59C2B5 T=0                         	
00002961	libmyjni.so:unk_BD59C6C8	CMP             R1, #0          	C=1                                     	
00002961	libmyjni.so:JNI_OnUnLoad+130	BEQ             unk_BD59C6B8    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+134	STMFD           SP!, {R0,R1,LR} 	SP=FF7F38CC                             	
00002961	libmyjni.so:JNI_OnUnLoad+138	BL              unk_BD59C5F4    	LR=BD59C6D8                             	
00002961	libmyjni.so:unk_BD59C5F4	EOR             R12, R0, R1     	R12=0000001F                            	
00002961	libmyjni.so:JNI_OnUnLoad+5C	RSBMI           R1, R1, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+60	SUBS            R2, R1, #1      	R2=0000000E                             	
00002961	libmyjni.so:JNI_OnUnLoad+64	BEQ             unk_BD59C684    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+68	MOVS            R3, R0          	R3=00000010                             	
00002961	libmyjni.so:JNI_OnUnLoad+6C	RSBMI           R3, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+70	CMP             R3, R1          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+74	BLS             unk_BD59C690    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+78	TST             R1, R2          	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+7C	BEQ             unk_BD59C6A0    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+80	CLZ             R2, R1          	R2=0000001C                             	
00002961	libmyjni.so:JNI_OnUnLoad+84	CLZ             R0, R3          	R0=0000001B                             	
00002961	libmyjni.so:JNI_OnUnLoad+88	SUB             R0, R2, R0      	R0=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+8C	MOV             R2, #1          	R2=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+90	MOV             R1, R1,LSL R0   	R1=0000001E                             	
00002961	libmyjni.so:JNI_OnUnLoad+94	MOV             R2, R2,LSL R0   	R2=00000002                             	
00002961	libmyjni.so:JNI_OnUnLoad+98	MOV             R0, #0          	R0=00000000                             	
00002961	libmyjni.so:unk_BD59C638	CMP             R3, R1          	PSR=80840010 C=0 N=1                    	
00002961	libmyjni.so:JNI_OnUnLoad+A0	SUBCS           R3, R3, R1      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+A4	ORRCS           R0, R0, R2      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+A8	CMP             R3, R1,LSR#1    	PSR=20840010 C=1 N=0                    	
00002961	libmyjni.so:JNI_OnUnLoad+AC	SUBCS           R3, R3, R1,LSR#1	R3=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+B0	ORRCS           R0, R0, R2,LSR#1	R0=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+B4	CMP             R3, R1,LSR#2    	PSR=80840010 C=0 N=1                    	
00002961	libmyjni.so:JNI_OnUnLoad+B8	SUBCS           R3, R3, R1,LSR#2	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+BC	ORRCS           R0, R0, R2,LSR#2	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C0	CMP             R3, R1,LSR#3    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C4	SUBCS           R3, R3, R1,LSR#3	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+C8	ORRCS           R0, R0, R2,LSR#3	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+CC	CMP             R3, #0          	PSR=20840010 C=1 N=0                    	
00002961	libmyjni.so:JNI_OnUnLoad+D0	MOVNES          R2, R2,LSR#4    	R2=00000000 C=0 Z=1                     	
00002961	libmyjni.so:JNI_OnUnLoad+D4	MOVNE           R1, R1,LSR#4    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+D8	BNE             unk_BD59C638    	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+DC	CMP             R12, #0         	C=1 Z=0                                 	
00002961	libmyjni.so:JNI_OnUnLoad+E0	RSBMI           R0, R0, #0      	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+E4	BX              LR              	                                        	
00002961	libmyjni.so:JNI_OnUnLoad+13C	LDMFD           SP!, {R1,R2,LR} 	R1=00000010 R2=0000000F SP=FF7F38D8 LR=BD59C2B5 	
00002961	libmyjni.so:JNI_OnUnLoad+140	MUL             R3, R2, R0      	R3=0000000F                             	
00002961	libmyjni.so:JNI_OnUnLoad+144	SUB             R1, R1, R3      	R1=00000001                             	
00002961	libmyjni.so:JNI_OnUnLoad+148	BX              LR              	T=1                                     	
00002961	libmyjni.so:n2+BC       	ADD             R3, SP, #0x18   	R3=FF7F38F0                             	
00002961	libmyjni.so:n2+BE       	ADD             R3, R1          	R3=FF7F38F1                             	
00002961	libmyjni.so:n2+C0       	MOV             R4, R1          	R4=00000001                             	
00002961	libmyjni.so:n2+C2       	LDRB.W          R2, [R3,#-0x16] 	R2=00000061                             	
00002961	libmyjni.so:n2+C6       	B               loc_BD59C2D2    	                                        	
00002961	libmyjni.so:loc_BD59C2D2	LDRB            R3, [R5]        	R3=00000063              ;debug input[5]               	
00002961	libmyjni.so:n2+DC       	ADDS            R6, #1          	R6=00000006 C=0                         	
00002961	libmyjni.so:n2+DE       	ADDS            R5, #1          	R5=EB437836 PSR=80840030 N=1            	
00002961	libmyjni.so:n2+E0       	EORS            R3, R2          	R3=00000002 PSR=00840030 N=0            	 ;debug result[5]
00002961	libmyjni.so:n2+E2       	STRB.W          R3, [R5,#-1]    	                                        	
00002961	libmyjni.so:n2+E6       	B               loc_BD59C278    	                                        	
00002961	libmyjni.so:loc_BD59C278	CMP             R6, R10         	C=1 Z=1                                 	
00002961	libmyjni.so:n2+82       	BGE             loc_BD59C2E0    	                                        	
00002961	libmyjni.so:loc_BD59C2E0	MOV             R0, R9          	R0=EB437830                             	
00002961	libmyjni.so:n2+EA       	MOV             R1, R7          	R1=E5D4001C                             	
00002961	libmyjni.so:n2+EC       	BLX             unk_BD59C108    	LR=BD59C2E9 T=0                         	
00002961	libmyjni.so:unk_BD59C108	ADRL            R12, 0xBD59F110 	R12=BD59F110                            	
00002961	libmyjni.so:BD59C110    	LDR             PC, [R12,#0xEB4]!	R12=BD59FFC4 T=1                        	
00002961	libc.so:fputs           	PUSH            {R4-R7,LR}      	SP=FF7F38C4                             	
00002961	                        	                                	R0=00000000 R1=8F855664 R2=00000000 R3=D05BC170 R4=00000001 R5=EB437836 R6=00000006 R7=E5D4001C R8=E8AD3124 R9=EB437830 R10=00000006 R11=FF7F399C R12=E8AD0114 SP=FF7F38D8 LR=E8AC0787 PSR=60840030 D0= D1= D2= D3= D4= D5= D6= D7= D8= D9= D10= D11= D12= D13= D14= D15= D16= D17= D18= D19= D20= D21= D22= D23= D24= D25= D26= D27= D28= D29= D30= 	
00002961	libmyjni.so:n2+F0       	LDR             R2, [SP,#0x14]  	R2=8F855664                             	
00002961	libmyjni.so:n2+F2       	LDR.W           R3, [R8]        	R3=8F855664                             	
00002961	libmyjni.so:n2+F6       	CMP             R2, R3          	                                        	
00002961	libmyjni.so:n2+F8       	BEQ             loc_BD59C2F6    	                                        	
00002961	libmyjni.so:loc_BD59C2F6	MOV             R0, R7          	R0=E5D4001C                             	
00002961	libmyjni.so:n2+100      	ADD             SP, SP, #0x18   	SP=FF7F38F0                             	
00002961	libmyjni.so:n2+102      	POP.W           {R4-R10,LR}     	R4=E558808C R5=00000030 R6=BD871B82 R7=00000003 R8=00000000 R9=E26C5410 R10=FF7F39D8 SP=FF7F3910 LR=BD6EA0E7 	
00002961	libmyjni.so:n2+106      	B.W             sub_BD59DC24    	                                        	
00002961	libmyjni.so:sub_BD59DC24	BX              PC              	T=0                                     	
00002961	libmyjni.so:sub_BD59DC24:loc_BD59DC28	LDR             R12, =(sub_BD59C12C - 0xBD59DC34)	R12=FFFFE4F8                            	
00002961	libmyjni.so:sub_BD59DC24+8	ADD             PC, R12, PC; sub_BD59C12C	                                        	
00002961	libmyjni.so:sub_BD59C12C	ADRL            R12, 0xBD59F134 	R12=BD59F134                            	
00002961	libmyjni.so:sub_BD59C12C+8	LDR             PC, [R12,#(off_BD59FFD0 - 0xBD59F134)]!; fclose	R12=BD59FFD0 T=1                        	
00002961	libc.so:fclose          	CMP             R0, #0          	PSR=A0840030 Z=0 N=1                    	


input 123abc(0x31 0x32 0x33 0x61 0x62 0x63) -> FmR.=.(0x46 0x6d 0x52 0x16 0x3d 0x02)
```

输入可以自己确定输入什么，这里输入的是 123abc，输出可以根据代码分析而得到sdcard/reg.dat得到输出内容是 FmR.=.。

根据输入log日志，可以定位到libmyjni.so:n2+E0这个偏移就是输出内容

```
libmyjni.so:n2+E0       	EORS            R3, R2     R3=00000046 PSR=00840030 N=0   ;debug result[0] 
libmyjni.so:n2+E0       	EORS            R3, R2     R3=0000006D PSR=00840030 N=0   ;debug result[1]
libmyjni.so:n2+E0       	EORS            R3, R2     R3=00000052 PSR=00840030 N=0   ;debug result[2]
libmyjni.so:n2+E0       	EORS            R3, R2     R3=00000016 PSR=00840030 N=0   ;debug result[3]
libmyjni.so:n2+E0       	EORS            R3, R2     R3=0000003D PSR=00840030 N=0   ;debug result[4]
libmyjni.so:n2+E0       	EORS            R3, R2     R3=00000002 PSR=00840030 N=0   ;debug result[5]
```

同样可以定位到输入是libmyjni.so:loc_BD59C2D2。全局搜索后可以得到

```
libmyjni.so:loc_BD59C2D2	LDRB            R3, [R5]   R3=00000031           ;debug input[0]
libmyjni.so:loc_BD59C2D2	LDRB            R3, [R5]   R3=00000032           ;debug input[1] 
libmyjni.so:loc_BD59C2D2	LDRB            R3, [R5]   R3=00000033           ;debug input[2]
libmyjni.so:loc_BD59C2D2	LDRB            R3, [R5]   R3=00000061           ;debug input[3]
libmyjni.so:loc_BD59C2D2	LDRB            R3, [R5]   R3=00000062           ;debug input[4]
libmyjni.so:loc_BD59C2D2	LDRB            R3, [R5]   R3=00000063           ;debug input[5]
```

定位到输入输出之后，就可以按照指令执行逻辑来推演算法，这里暂时先推演了，这个案例比较简单，直接用IDA F5就可以看到算法了，主要是记录trace 方法和过程。

#### python 脚本IDA 7.7 兼容

看雪有一篇文章介绍了IDA 脚本的迁移与变化，https://bbs.kanxue.com/thread-269643.htm 可以先看看这个，就能轻松适配IDA7.7了。官方文档 [IDA7.4变化适配](https://docs.hex-rays.com/archive/porting-guide-for-ida-7.4-turning-off-ida-6.x-api-backwards-compatibility-by-default)

使用到的IDA 脚本API简介：

| trace脚本用到的API                | API作用简介              | 新版本API                  |
| --------------------------------- | ------------------------ | -------------------------- |
| idc.MakeCode(ea)                  | 分析代码区               | idc.create_insn            |
| idc.add_bpt(ea)                   | 在指定的地点设置软件断点 | ida_dbg.add_bpt            |
| ida_idd.regval_t()                | 用于获取值的寄存器       |                            |
| ida_dbg.get_reg_val(register, rv) | 读取register的值存放到rv |                            |
| idc.get_current_thread()          | 获取当前线程             | ida_dbg.get_current_thread |
| idc.get_thread_qty()              | 获取线程数量             | ida_dbg.get_thread_qty     |
| idc.getn_thread(i)                | 获取对应线程             | ida_dbg.getn_thread(i)     |
| idc.suspend_thread(other_thread)  | 暂停线程                 | ida_dbg.suspend_thread()   |
| idc.resume_thread(other_thread)   | 恢复线程                 | ida_dbg.resume_thread()    |
| idc.resume_process()              | 恢复进程                 |                            |
| idc._get_modules()                | 获取进程所有module       |                            |
|                                   |                          |                            |
|                                   |                          |                            |
|                                   |                          |                            |
|                                   |                          |                            |
|                                   |                          |                            |
|                                   |                          |                            |
|                                   |                          |                            |
|                                   |                          |                            |
|                                   |                          |                            |
|                                   |                          |                            |

并不是所有的API都需要配替换，可以去IDA安装目录的官网 python\3\idc.py查看哪些idc API没了，没了的就按照新的API引入ida_dbg调用即可。 [DBG_Hooks](https://python.docs.hex-rays.com/ida_dbg#dbg_hooks) 详细介绍。官方的 [IDA python demo](https://github.com/idapython/src/blob/master/examples/debugging/dbghooks/dbg_trace.py)

兼容后的脚本在/ProjectDocs\UseToolsRecord\IDATracePython\trace_xman_77.py

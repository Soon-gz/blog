# 注入技术的实现原理

## Android平台ptrace注入技术

### ptrace函数介绍

ptrace注入技术的核心就是ptrace函数，在ptrace注入过程中，将多次调用ptrace函数。Linux的man文档(超链接至: http://man7.org/linux/man-pages/man2/ptrace.2.html) 中提到，ptrace函数为一个进程提供了监视和控制其他进程的方法，在注入进程后，父进程还可以读取和修改子进程的内存空间以及寄存器值。ptrace函数的原型如下所示，其中request参数为一个联合体，该参数决定了ptrace函数的行为，pid参数为远程进程的ID，addr参数与data参数在不同的request参数取值下表示不同的含义。

```
long ptrace(enum __ptrace_request request, pid_t pid, void *addr, void *data);
```

request参数取值较多，由于篇幅所限这里仅介绍一部分ptrace注入进程的过程中需要使用到的request参数。

```
PTRACE_ATTACH，表示附加到指定远程进程;

PTRACE_DETACH，表示从指定远程进程分离

PTRACE_GETREGS，表示读取远程进程当前寄存器环境

PTRACE_SETREGS，表示设置远程进程的寄存器环境

PTRACE_CONT，表示使远程进程继续运行

PTRACE_PEEKTEXT，从远程进程指定内存地址读取一个word大小的数据

PTRACE_POKETEXT，往远程进程指定内存地址写入一个word大小的数据
```

### 实现原理

目前有两种实现ptrace注入模块到远程进程的方法，

第1种方法是直接远程调用dlopen> dlsym等函 数加载被注入模块并执行指定的代码。

第2种方法是使用ptrace将shellcode注入远程进程的内存空间中，然后通 过执行shellcode加载远程进程模块；

![](images\12.png)

1. #### Attach到远程进程

   ptrace注入的第一个步骤是先附加到远程进程上，如下所示，附加到远程进程是通过调用request参数为PTRACE_ATTACH的ptrace函数，pid为对应需要附加的远程进程的ID，addr参数和data参数为NULL。

   ```
   ptrace(PTRACE_ATTACH, pid, NULL, NULL);
   ```

   在附加到远程进程后，远程进程的执行会被中断，此时父进程可以通过调用waitpid函数来判断子进程是否进入暂停状态。waitpid的函数原型如下所示，其中当options参数为WUNTRACED ，表示若对应pid的远程进程进入暂停状态，则马上返回，可用于等待远程进程进入暂停状态。

   ```
   pid_t waitpid(pid_t pid,int * status,int options);
   ```

2. #### 读取和写入寄存器值

   在通过ptrace改变远程进程执行流程前，需要先读取远程进程的所有寄存器值进行保存，在detach时向远程进程写入保存的原寄存器值用于恢复远程进程原有的执行流程。

   如下所示，为读取和写入寄存器值的ptrace调用，request参数分别为PTRACE_GETREGS和PTRACE_SETREGS，pid为对应进程的ID。

   ```
   ptrace(PTRACE_GETREGS, pid, NULL, regs);
   
   ptrace(PTRACE_SETREGS, pid, NULL, regs);
   ```

   在ARM处理器下，data参数的regs为pt_regs结构的指针，从远程进程获取的寄存器值将存储到该结构中，pt_regs结构的定义如下所示，其中ARM_r0成员用于存储R0寄存器的值，函数调用后的返回值会存储在R0寄存器中，ARM_pc成员存储当前执行地址，ARM_sp成员存储当前栈顶地址，ARM_lr成员存储返回地址，ARM_cpsr成员存储状态寄存器的值。

   ```
   struct pt_regs {
    long uregs[18];
   };
   
   #define ARM_cpsr uregs[16]
   #define ARM_pc uregs[15]
   #define ARM_lr uregs[14]
   #define ARM_sp uregs[13]
   #define ARM_ip uregs[12]
   #define ARM_fp uregs[11]
   #define ARM_r10 uregs[10]
   #define ARM_r9 uregs[9]
   #define ARM_r8 uregs[8]
   #define ARM_r7 uregs[7]
   #define ARM_r6 uregs[6]
   #define ARM_r5 uregs[5]
   #define ARM_r4 uregs[4]
   #define ARM_r3 uregs[3]
   #define ARM_r2 uregs[2]
   #define ARM_r1 uregs[1]
   #define ARM_r0 uregs[0]
   #define ARM_ORIG_r0 uregs[17]
   ```

3. #### 远程进程的内存读取和写入数据

   调用request参数为PTRACE_PEEKTEXT的ptrace函数可以从远程进程的内存空间中读取数据，一次读取一个word大小的数据。如下所示，其中addr参数为需读取数据的远程进程内存地址，返回值为读取出的数据。

   ```
   ptrace(PTRACE_PEEKTEXT, pid, pCurSrcBuf, 0);
   
   ptrace(PTRACE_POKETEXT, pid, pCurDestBuf, lTmpBuf) ;
   ```

   调用request参数为PTRACE_POKETEXT的ptrace函数可以将数据写入到远程进程的内存空间中，同样一次写入一个word大小的数据，ptrace函数的addr参数为要写入数据的远程进程内存地址，data参数为要写入的数据。

   写入数据时需要注意，若写入数据长度不是一个word大小的倍数，写入最后一个不足word大小的数据时，要先保存原地址处的高位数据。

   如下代码所示，首先通过request参数为PTRACE_PEEKTEXT的ptrace函数读取原内存中的一个word大小的数据，然后将要写入的数据复制到读取出的数据的低位，然后调用ptrace函数将修改后的数据写入远程进程的内存地址处。

   ```
   lTmpBuf = ptrace(PTRACE_PEEKTEXT, pid, pCurDestBuf, NULL); 
   memcpy((void *)(&lTmpBuf), pCurSrcBuf, nRemainCount);
   if (ptrace(PTRACE_POKETEXT, pid, pCurDestBuf, lTmpBuf) < 0)
   {
       LOGD("Write Remote Memory error, MemoryAddr:0x%lx", (long)pCurDestBuf);
       return -1;
   }
   ```

4. #### 远程调用函数

   在ARM处理器中，函数调用的前四个参数通过R0-R3寄存器来传递，剩余参数按从右到左的顺序压入栈中进行传递。如下代码所示，在远程调用函数前，需要先判断函数调用的参数个数，如果小于4个，则将参数按顺序分别写入R0-R3寄存器中，若大于4个，则首先调整SP寄存器在栈中分配空间，然后通过调用ptrace函数将剩余参数写入到栈中。

   ```
   for (i = 0; i < num__params && i < 4; i ++)
   {
   	regs->uregs[i] = parameters[i];
   }
   if (i < num_params)
   {
   	regs->ARM_sp -= (num_params - i) * sizeof(long);
   if (ptrace_writedata (pid, (void *) regs->ARM_spz (uint8_t *) &parameters [i] / (num_params - i) * sizeof(long)) == -1) return -1;
   }
   ```

   在写入函数的参数后，修改进程的PC寄存器为需要执行的函数地址。这里有一点需要注意，在ARM架构下有ARM和Thumb两种指令，因此在调用函数前需要判断函数被解析成哪种指令，如下所示的代码就是通过地址的最低位是否为1来判断调用地址处指令为ARM或Thumb，若为Thumb指令，则需要将最低位重新设置为0，并且将CPSR寄存器的T标志位置位，若为ARM指令，则将CPSR寄存器的T标志位复位。

   ```
   if (regs->ARM_pc & 1) {                      /* thumb */    
   	regs->ARM_pc &= (~1u);    
   	regs->ARM_cpsr |= CPSR_T_MASK;    
   } else {                                    /* arm */    
   	regs->ARM_cpsr &= ~CPSR_T_MASK;    
   }
   ```

   在使远程进程恢复运行前，还需要设置远程进程的LR寄存器值为0，并且在在本地进程调用options参数为WUNTRACED的waitpid函数等待远程进程重新进入暂停状态。远程进程的函数调用结束后，会跳转到LR寄存器存储的地址处，但由于LR寄存器被设置为0，会导致远程进程执行出错，此时进程会进入暂停状态，本地进程等待结束，通过读取远程进程的R0寄存器可以获取远程函数调用的返回结果，以上就是一次完整地调用远程函数的过程。

   在ptrace注入流程中需要多次调用函数，除了调用被注入模块的函数外，还需要调用mmap函数在远程进程地址空间内分配内存，调用dlopen函数来远程加载被注入模块，调用dlsym函数来获取被注入模块对应函数的地址，调用dlclose函数来关闭加载的模块。这些函数的原型如下所示，

   ```
   void* mmap(void* start,size_t length,int prot,int flags,int fd,off_t offset);
   
   void * dlopen( const char * pathname, int mode);
   
   void*dlsym(void*handle,constchar*symbol);
   
   int dlclose (void *handle);
   ```

   在调用这些函数前，需要首先获取到这些系统函数在远程进程中的地址，mmap函数是在”/system/lib/libc.so”模块中，dlopen、dlsym与dlclose函数均是在”/system/bin/linker”模块中。

   读取”/proc/pid/maps”可以获取到系统模块在本地进程和远程进程的加载基地址，要获取远程进程内存空间中mmap等函数的虚拟地址，可通过计算本地进程中mmap等函数相对于模块的地址偏移，然后使用此地址偏移加上远程进程对应模块的基地址，这个地址就是远程进程内存空间中对应函数的虚拟地址。

   

5. #### 恢复寄存器值

   在从远程进程detach前，需要将远程进程的原寄存器环境恢复，保证远程进程原有的执行流程不被破坏，如果不恢复寄存器值，detach时会导致远程进程的崩溃。

6. #### Detach 进程

   从远程进程脱离是ptrace注入的最后一个步骤，在detach后被注入进程将继续运行。如下所示，从远程进程detach是调用request参数为PTRACE_DETACH的ptrace函数。

   ```
   ptrace(PTRACE_DETACH, pid, NULL, 0);
   ```



### ptrace 实例测试

#### 注入so库方式

使用2048小游戏进行注入测试，ptraceInject代码地址：ProjectDocs\UseToolsRecord\AndroidInject\\PtraceInject,

- 测试机型：一加8T android 11 root
- 确定需要注入的进程名：com.estoty.game2048
- 注入模块全路径：/data/local/tmp/source13/libInjectModule.so （**注意，高版本需要放在应用安装目录下，避免dlopen没有权限**）
- 注入模块后调用模块函数名称：Inject_entry
- **需要注意不同版本libc和linker在maps的映射路径**
- /apex/com.android.runtime/bin/linker
- /apex/com.android.runtime/lib/bionic/libc.so
- 书籍携带源码无法正常运行，基于此改动了符号偏移定位的方案。

新增GetRemoteFuncLinkerAddr函数，主要逻辑代码如下：

```
Elf32_Ehdr* ehdr = (Elf32_Ehdr*)map;
    Elf32_Shdr* shdr = (Elf32_Shdr*)((char*)map + ehdr->e_shoff);
	const char* shstrtab = (const char*)map + shdr[ehdr->e_shstrndx].sh_offset;

    int sym_count = 0;
	const char* strtab = NULL;
	Elf32_Sym* symtab = NULL;

    for (int i = 0; i < ehdr->e_shnum; ++i) {
    	const char* section_name = shstrtab + shdr[i].sh_name;
        if (shdr[i].sh_type == SHT_DYNSYM && strcmp(section_name, ".dynsym") == 0) {
            sym_count = shdr[i].sh_size / shdr[i].sh_entsize;
            symtab = (Elf32_Sym*)((char*)map + shdr[i].sh_offset);
            printf("SHT_DYNSYM sym_count:%d sh_offset:%lx \n",sym_count,shdr[i].sh_offset);
        }
        if (shdr[i].sh_type == SHT_STRTAB && strcmp(section_name, ".dynstr") == 0) {
        	strtab = (const char*)((char*)map + shdr[i].sh_offset);
        	printf("SHT_STRTAB sh_offset:%lx \n",shdr[i].sh_offset);
        }
    }

    if (sym_count == 0 || !strtab || !symtab ) {
        printf("Failed to find symtab or strtab  section\n");
        return -1;
    }

    uint32_t symbol_offset = 0;

    for (int i = 0; i < sym_count; ++i) {
        const char* sym_name = strtab + symtab[i].st_name;
        if (strcmp(sym_name, method_name) == 0) {
            printf("%s: %p\n", sym_name, (void*)(symtab[i].st_value));
        	symbol_offset = (uint32_t)symtab[i].st_value;
        	break;
        }
    }
```

inject的主函数源码

```
int main(int argc, char *argv[]) {
	char InjectModuleName[MAX_PATH] = "/data/local/tmp/source13/libInjectModule.so";    // 注入模块全路径
	char RemoteCallFunc[MAX_PATH] = "Inject_entry";              // 注入模块后调用模块函数名称
	char InjectProcessName[MAX_PATH] = "com.estoty.game2048";                      // 注入进程名称
	
	// 当前设备环境判断
	#if defined(__i386__)  
	LOGD("Current Environment x86");
	return -1;
	#elif defined(__arm__)
	LOGD("Current Environment ARM");
	#else     
	LOGD("other Environment");
	return -1;
	#endif
	
	pid_t pid = FindPidByProcessName(InjectProcessName);
	if (pid == -1)
	{
		printf("Get Pid Failed");
		return -1;
	}	
	
	printf("begin inject process, RemoteProcess pid:%d, InjectModuleName:%s, RemoteCallFunc:%s\n", pid, InjectModuleName, RemoteCallFunc);
	int iRet = inject_remote_process(pid,  InjectModuleName, RemoteCallFunc,  NULL, 0);
	//int iRet = inject_remote_process_shellcode(pid,  InjectModuleName, RemoteCallFunc,  NULL, 0);
	
	if (iRet == 0)
	{
		printf("Inject Success\n");
	}
	else
	{
		printf("Inject Failed\n");
	}
	printf("end inject,%d\n", pid);
    return 0;  
}  
```

注入模块的源码

```
#include <stdio.h>
#include <stdlib.h>
#include <utils/PrintLog.h>

int Inject_entry()
{
	LOGD("Inject_entry Func is called\n");
	return 0;
}
```

然后使用ndk-build编译生成对应so libInjectModule.so和可执行文件 inject。可以在环境变量加入 Sdk\ndk\21.3.6528147\ndk-build.cmd。可以根据需要的架构修改arm和arm64。

```
E:blog\ProjectDocs\UseToolsRecord\AndroidInject\PtraceInject\InjectModule\jni> ndk-build
    Android NDK: APP_PLATFORM not set. Defaulting to minimum supported version android-16.
    [armeabi-v7a] Compile arm    : InjectModule <= InjectModule.c
    [armeabi-v7a] SharedLibrary  : libInjectModule.so
    [armeabi-v7a] Install        : libInjectModule.so => libs/armeabi-v7a/libInjectModule.so
```

```
E:\blog\ProjectDocs\UseToolsRecord\AndroidInject\PtraceInject\PtraceInject\jni> ndk-build
	Android NDK: APP_PLATFORM not set. Defaulting to minimum supported version android-16.
	[armeabi-v7a] Compile thumb  : inject <= ptraceInject.c
	8 warnings generated.
    [armeabi-v7a] Compile thumb  : inject <= InjectModule.c
    [armeabi-v7a] Compile thumb  : inject <= shellcode.s
    [armeabi-v7a] Executable     : inject
    [armeabi-v7a] Install        : inject => libs/armeabi-v7a/inject
```

将inject放入设备中,执行以下脚本。

```
echo %cd%
adb shell su -c "rm /data/local/tmp/source13/inject"
adb push "libs\armeabi-v7a\inject" "/data/local/tmp/source13/inject"
adb shell su -c "chmod 777 /data/local/tmp/source13/*"
adb shell su -c "/data/local/tmp/source13/inject"
```

在测试过程，android 11 无法dlopen data/local文件。

```
dlopen error:dlopen failed: couldn't map "/data/local/tmp/source13/libInjectModule.so" segment 1: Permission denied
```

所以需要将so放在应用lib目录下。

注入成功后，inject 输出的日志：

```
E:\blog\ProjectDocs\UseToolsRecord\AndroidInject\PtraceInject\PtraceInject>adb shell su -c "/data/local/tmp/source13/inject"
     current pid is 20660
    begin inject process, RemoteProcess pid:15178, InjectModuleName:/data/app/~~xBNgt1KaOO4XWCy65QHr-w==/com.estoty.game2048-AGgpdeFMRdt5g9zYqA2Ydw==/lib/arm/libInjectModule.so, RemoteCallFunc:Inject_entry
    ======== inject_remote_process =======
    attach process pid:15178ARM_r0:0xfffffffc, ARM_r1:0xff7f4c20, ARM_r2:0x10, ARM_r3:0xabf, ARM_r4:0x0, ARM_r5:0x8, ARM_r6:0xe5f44e10, ARM_r7:0x15a, ARM_r8:0xe5f44e5c, ARM_r9:0xe5f44e10, ARM_r10:0x0, ARM_ip:0xff7f4bd0, ARM_sp:0xff7f4bc0, ARM_lr:0xe8a8822d, ARM_pc:0xe8ab695c
    Remote init Addr:0xfffffffc
    
    ------- [GetRemoteFuncAddr] -------
     pid -1 find /apex/com.android.runtime/lib/bionic/libc.so
     pid 15178 find /apex/com.android.runtime/lib/bionic/libc.so
    ModuleName:/apex/com.android.runtime/lib/bionic/libc.so LocalModuleAddr:0xf44cb000  RemoteModuleAddr:0xe8a44000 LocalFuncAddr:0xf4511d7d offset:0x46d7d
    mmap RemoteFuncAddr:0xe8a8ad7d
    ptrace call ret status is 2943
    Remote Process Map Memory Addr:0xeaf48000
    
    ------- [GetRemoteFuncLinkerAddr] __loader_dlopen-------
    SHT_DYNSYM sym_count:25 sh_offset:1b4
    SHT_STRTAB sh_offset:40c
    __loader_dlopen: 0x198b1
     pid 15178 find /apex/com.android.runtime/bin/linker
    ModuleName:/apex/com.android.runtime/bin/linker   RemoteModuleAddr:0xeb7b0000  offset:0x198b1
    
    ------- [GetRemoteFuncLinkerAddr] __loader_dlsym-------
    SHT_DYNSYM sym_count:25 sh_offset:1b4
    SHT_STRTAB sh_offset:40c
    __loader_dlsym: 0x1994d
     pid 15178 find /apex/com.android.runtime/bin/linker
    ModuleName:/apex/com.android.runtime/bin/linker   RemoteModuleAddr:0xeb7b0000  offset:0x1994d
    
    ------- [GetRemoteFuncLinkerAddr] __loader_dlclose-------
    SHT_DYNSYM sym_count:25 sh_offset:1b4
    SHT_STRTAB sh_offset:40c
    __loader_dlclose: 0x19981
     pid 15178 find /apex/com.android.runtime/bin/linker
    ModuleName:/apex/com.android.runtime/bin/linker   RemoteModuleAddr:0xeb7b0000  offset:0x19981
    
    ------- [GetRemoteFuncLinkerAddr] __loader_dlerror-------
    SHT_DYNSYM sym_count:25 sh_offset:1b4
    SHT_STRTAB sh_offset:40c
    __loader_dlerror: 0x197a1
     pid 15178 find /apex/com.android.runtime/bin/linker
    ModuleName:/apex/com.android.runtime/bin/linker   RemoteModuleAddr:0xeb7b0000  offset:0x197a1
    
    dlopen RemoteFuncAddr:0xeb7c98b1
    dlsym RemoteFuncAddr:0xeb7c994d
    dlclose RemoteFuncAddr:0xeb7c9981
    dlerror RemoteFuncAddr:0xeb7c97a1
    ptrace call ret status is 2943
    Remote Process load module Addr:0xeb684810
    ptrace call ret status is 2943
    Remote Process ModuleFunc Addr:0xb0e5bea0
    ptrace call ret status is 2943
    Recover Regs Success
    detach process pid:15178
    Inject Success
    end inject,15178
```

logcat输出的日志：

```
2024-11-13 16:51:36.864 20212-20212 INJECT                  inject                               D  Current Environment ARM
2024-11-13 16:51:37.008 15177-15177 INJECT                  com.estoty.game2048                  D  Inject_entry Func is called
```

#### 注入shellcode 方式

shellcode 的方式会提前预埋_inject_start_s，_inject_end_s来实现远程函数的调用，inject的android.mk已经编译进去了。

```
LOCAL_SRC_FILES := ptraceInject.c InjectModule.c shellcode.s
```

shellcode注入和dlopen方式类似，也是需要先找到远程进程的dlopen等函数地址，然后将地址复制给shellcode.s的汇编变量，然后将shellcode的汇编通过ptracce注入到远程mmap申请的内存空间中，然后设置寄存器的pc和sp为远端code的地址，最后执行code，代码中有详细的注释以及测试内容。

inject输出以下内容：

```
E:\MyLearnSource\blog\ProjectDocs\UseToolsRecord\AndroidInject\PtraceInject\PtraceInject>adb shell su -c "/data/local/tmp/source13/inject"
     current pid is 21836
    begin inject process, RemoteProcess pid:17683, InjectModuleName:/data/app/~~xBNgt1KaOO4XWCy65QHr-w==/com.estoty.game2048-AGgpdeFMRdt5g9zYqA2Ydw==/lib/arm/libInjectModule.so, RemoteCallFunc:Inject_entry
    ======== inject_remote_process_shellcode =======
    attach process pid:17683ARM_r0:0xfffffffc, ARM_r1:0xff7f4c20, ARM_r2:0x10, ARM_r3:0x1288, ARM_r4:0x0, ARM_r5:0x8, ARM_r6:0xe5f44e10, ARM_r7:0x15a, ARM_r8:0xe5f44e5c, ARM_r9:0xe5f44e10, ARM_r10:0x0, ARM_ip:0xff7f4bd0, ARM_sp:0xff7f4bc0, ARM_lr:0xe8a8822d, ARM_pc:0xe8ab695c
    
    ------- [GetRemoteFuncAddr] -------
     pid -1 find /apex/com.android.runtime/lib/bionic/libc.so
     pid 17683 find /apex/com.android.runtime/lib/bionic/libc.so
    ModuleName:/apex/com.android.runtime/lib/bionic/libc.so LocalModuleAddr:0xf1481000  RemoteModuleAddr:0xe8a44000 LocalFuncAddr:0xf14c7d7d offset:0x46d7d
    mmap RemoteFuncAddr:0xe8a8ad7d
    ptrace call ret status is 2943
    Remote Process Map Memory Addr:0xe9a1d000
    
    ------- [GetRemoteFuncLinkerAddr] __loader_dlopen-------
    SHT_DYNSYM sym_count:25 sh_offset:1b4
    SHT_STRTAB sh_offset:40c
    __loader_dlopen: 0x198b1
     pid 17683 find /apex/com.android.runtime/bin/linker
    ModuleName:/apex/com.android.runtime/bin/linker   RemoteModuleAddr:0xeb7b0000  offset:0x198b1
    
    ------- [GetRemoteFuncLinkerAddr] __loader_dlsym-------
    SHT_DYNSYM sym_count:25 sh_offset:1b4
    SHT_STRTAB sh_offset:40c
    __loader_dlsym: 0x1994d
     pid 17683 find /apex/com.android.runtime/bin/linker
    ModuleName:/apex/com.android.runtime/bin/linker   RemoteModuleAddr:0xeb7b0000  offset:0x1994d
    
    ------- [GetRemoteFuncLinkerAddr] __loader_dlclose-------
    SHT_DYNSYM sym_count:25 sh_offset:1b4
    SHT_STRTAB sh_offset:40c
    __loader_dlclose: 0x19981
     pid 17683 find /apex/com.android.runtime/bin/linker
    ModuleName:/apex/com.android.runtime/bin/linker   RemoteModuleAddr:0xeb7b0000  offset:0x19981
    
    ------- [GetRemoteFuncLinkerAddr] __loader_dlerror-------
    SHT_DYNSYM sym_count:25 sh_offset:1b4
    SHT_STRTAB sh_offset:40c
    __loader_dlerror: 0x197a1
     pid 17683 find /apex/com.android.runtime/bin/linker
    ModuleName:/apex/com.android.runtime/bin/linker   RemoteModuleAddr:0xeb7b0000  offset:0x197a1
    
    dlopen RemoteFuncAddr:0xeb7c98b1
    dlsym RemoteFuncAddr:0xeb7c994d
    dlclose RemoteFuncAddr:0xeb7c9981
    dlerror RemoteFuncAddr:0xeb7c97a1
    
    Inject Code Start:0x45d2008, end:0x45d2084
    code_length : 124  dlopen_param1_ptr:0x45d20a4
    detach process pid:17683
    Inject Success
    end inject,17683
```

logcat输出以下内容：

```
2024-11-13 17:33:36.129 21836-21836 INJECT                  pid-21836                            D  Current Environment ARM
2024-11-13 17:33:36.172 17683-17683 INJECT                  com.estoty.game2048                  D  Inject_entry Func is called
```

## Android平台zygote注入技术

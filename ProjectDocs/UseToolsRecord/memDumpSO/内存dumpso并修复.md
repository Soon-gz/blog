内存dump so主要是dump内存maps的数据。github上有开源修复工具，修复了一下开源的64位回编的bug。github：[SoFixer](https://github.com/Soon-gz/SoFixer.git)

#### 简单原理

作者参考了linker的ElfReader，对dump的文件按照Header，phdr，segments，sections进行解析。

```
bool ElfReader::Load() {
    // try open
    return ReadElfHeader() &&
           VerifyElfHeader() &&
           ReadProgramHeader() &&
           ReserveAddressSpace() &&
           LoadSegments() &&
           FindPhdr() &&
           PatchPhdr();
}
```

由于内存dump的数据都是绝对地址，所以需要对偏移进行修复，这样用IDA看起来会更方便。所以就有了ElfRebuilder，对数据进行偏移修复。同样的，修复的逻辑也是elf的结构来。

```
bool ElfRebuilder::Rebuild() {
    return RebuildPhdr() && ReadSoInfo() && RebuildShdr() && RebuildRelocs() && RebuildFin();
}
```

#### 实操

使用root机器或者模拟器开启root权限，安装 xman.apk，该apk在ProjectDocs\UseToolsRecord\memDumpSO目录下。获取当前进程号。

```
adb shell ps |findstr xman
u0_a1399      12321   7875 1392040  98148 0                   0 S com.gdufs.xman
```

然后 adb shell 查看对应maps的数据其实地址和大小。

```
 cat /proc/12321/maps |grep myjni.so
 
 c141d000-c1420000 r-xp 00000000 fc:0c 535472                             /data/app/~~Pjlr0l_3GCmvON7WegtO7Q==/com.gdufs.xman-2RHeB70T4wkejWQICMJLyw==/lib/arm/libmyjni.so
c1421000-c1422000 r--p 00003000 fc:0c 535472                             /data/app/~~Pjlr0l_3GCmvON7WegtO7Q==/com.gdufs.xman-2RHeB70T4wkejWQICMJLyw==/lib/arm/libmyjni.so
c1422000-c1423000 rw-p 00004000 fc:0c 535472                             /data/app/~~Pjlr0l_3GCmvON7WegtO7Q==/com.gdufs.xman-2RHeB70T4wkejWQICMJLyw==/lib/arm/libmyjni.so
OnePlus8T:/ # cat /proc/12321/maps |grep -A 20 myjni
c141d000-c1420000 r-xp 00000000 fc:0c 535472                             /data/app/~~Pjlr0l_3GCmvON7WegtO7Q==/com.gdufs.xman-2RHeB70T4wkejWQICMJLyw==/lib/arm/libmyjni.so
c1420000-c1421000 ---p 00000000 00:00 0
c1421000-c1422000 r--p 00003000 fc:0c 535472                             /data/app/~~Pjlr0l_3GCmvON7WegtO7Q==/com.gdufs.xman-2RHeB70T4wkejWQICMJLyw==/lib/arm/libmyjni.so
c1422000-c1423000 rw-p 00004000 fc:0c 535472                             /data/app/~~Pjlr0l_3GCmvON7WegtO7Q==/com.gdufs.xman-2RHeB70T4wkejWQICMJLyw==/lib/arm/libmyjni.so
```

可以确认首地址和大小是0xc141d000 ,0x6000,可以使用dd命名进行dump。[dd命名解析](https://www.ibm.com/docs/zh-tw/aix/7.3?topic=d-dd-command)

```
dd if=/proc/12321/mem of=/sdcard/dump/xman_jni.so skip=0xc141d ibs=0x1000 count=6
    6+0 records in
    48+0 records out
    24576 bytes (24 K) copied, 0.004504 s, 5.2 M/s
```

拿到dump的内存so文件后，使用Sofixer进行修复。

```
 .\Sofixer32.exe -s .\xman_jni_0xc141d000_0x6000.so -o fix.so -d -m 0xc141d000
/*********************************************sofixer***************************************/
args counts : 9
[main::argcv ]source : .\xman_jni_0xc141d000_0x6000.so
[main::argcv ]output : fix.so
[main::argcv ]base : 0xFFFFFFF3
start to rebuild elf file
[ElfRebuilder::RebuildPhdr:19]=======================RebuildPhdr=========================
[ElfRebuilder::RebuildPhdr:29]=====================RebuildPhdr End======================
[ElfRebuilder::ReadSoInfo:457]=======================ReadSoInfo=========================
[ElfRebuilder::ReadSoInfo:506]name plt_rel_count (DT_PLTRELSZ) 19
[ElfRebuilder::ReadSoInfo:502]name plt_rel (DT_JMPREL) found at 102c
[ElfRebuilder::ReadSoInfo:510]name rel (DT_REL) found at f8c
[ElfRebuilder::ReadSoInfo:514]name rel_size (DT_RELSZ) 20
[ElfRebuilder::ReadSoInfo:607]Unused DT entry: type 0x6ffffffa arg 0x00000011
[ElfRebuilder::ReadSoInfo:492]symbol table found at 18c
[ElfRebuilder::ReadSoInfo:488]string table found at 6ac
[ElfRebuilder::ReadSoInfo:604]soname
[ElfRebuilder::ReadSoInfo:545] destructors (DT_FINI_ARRAY) found at 4e4c
[ElfRebuilder::ReadSoInfo:549] destructors (DT_FINI_ARRAYSZ) 2
[ElfRebuilder::ReadSoInfo:537] constructors (DT_INIT_ARRAY) found at 4e54
[ElfRebuilder::ReadSoInfo:541] constructors (DT_INIT_ARRAYSZ) 1
[ElfRebuilder::ReadSoInfo:607]Unused DT entry: type 0x6ffffffb arg 0x00000001
[ElfRebuilder::ReadSoInfo:607]Unused DT entry: type 0x6ffffff0 arg 0x00000eac
[ElfRebuilder::ReadSoInfo:607]Unused DT entry: type 0x6ffffffc arg 0x00000f50
[ElfRebuilder::ReadSoInfo:607]Unused DT entry: type 0x6ffffffd arg 0x00000001
[ElfRebuilder::ReadSoInfo:607]Unused DT entry: type 0x6ffffffe arg 0x00000f6c
[ElfRebuilder::ReadSoInfo:607]Unused DT entry: type 0x6fffffff arg 0x00000001
[ElfRebuilder::ReadSoInfo:611]=======================ReadSoInfo End=========================
[ElfRebuilder::RebuildShdr:34]=======================RebuildShdr=========================
[ElfRebuilder::RebuildShdr:448]=====================RebuildShdr End======================
[ElfRebuilder::RebuildRelocs:640]=======================RebuildRelocs=========================
[ElfRebuilder::RebuildRelocs:671]=======================RebuildRelocs End=======================
[ElfRebuilder::RebuildFin:617]=======================try to finish file=========================
[ElfRebuilder::RebuildFin:635]=======================End=========================
Done!!!
```

这个时候用IDA查看是可以正常打开查看完整的so结构的，如果so加固，或者结构被破坏，需要手动去定位修改的结构，去修复了。

#### python脚本配合frida dump并修复

这个方案在github上也有现成的了，比较好用，方便快捷，拿来主义，能偷懒就偷懒。[frida_dump](https://github.com/lasting-yang/frida_dump.git)

```
python .\dump_so.py libmyjni.so
    {'name': 'libmyjni.so', 'base': '0xc141d000', 'size': 24576, 'path': '/data/app/~~Pjlr0l_3GCmvON7WegtO7Q==/com.gdufs.xman-2RHeB70T4wkejWQICMJLyw==/lib/arm/libmyjni.so'}
    android/SoFixer32: 1 file pushed, 0 skipped. 89.0 MB/s (91848 bytes in 0.001s)
    libmyjni.so.dump.so: 1 file pushed, 0 skipped. 91.2 MB/s (24576 bytes in 0.000s)
    adb shell /data/local/tmp/SoFixer -m 0xc141d000 -s /data/local/tmp/libmyjni.so.dump.so -o /data/local/tmp/libmyjni.so.dump.so.fix.so
    [main_loop:87]start to rebuild elf file
    [Load:69]dynamic segment have been found in loadable segment, argument baseso will be ignored.
    [RebuildPhdr:25]=============LoadDynamicSectionFromBaseSource==========RebuildPhdr=========================
    [RebuildPhdr:37]=====================RebuildPhdr End======================
    [ReadSoInfo:549]=======================ReadSoInfo=========================
    [ReadSoInfo:595]name plt_rel_count (DT_PLTRELSZ) 19
    [ReadSoInfo:591]name plt_rel (DT_JMPREL) found at 102c
    [ReadSoInfo:599]name rel (DT_REL) found at f8c
    [ReadSoInfo:603]name rel_size (DT_RELSZ) 20
    [ReadSoInfo:699]Unused DT entry: type 0x6ffffffa arg 0x00000011
    [ReadSoInfo:584]symbol table found at 18c
    [ReadSoInfo:580]string table found at 6ac
    [ReadSoInfo:696]soname
    [ReadSoInfo:637] destructors (DT_FINI_ARRAY) found at 4e4c
    [ReadSoInfo:641] destructors (DT_FINI_ARRAYSZ) 2
    [ReadSoInfo:629] constructors (DT_INIT_ARRAY) found at 4e54
    [ReadSoInfo:633] constructors (DT_INIT_ARRAYSZ) 1
    [ReadSoInfo:699]Unused DT entry: type 0x6ffffffb arg 0x00000001
    [ReadSoInfo:699]Unused DT entry: type 0x6ffffff0 arg 0x00000eac
    [ReadSoInfo:699]Unused DT entry: type 0x6ffffffc arg 0x00000f50
    [ReadSoInfo:699]Unused DT entry: type 0x6ffffffd arg 0x00000001
    [ReadSoInfo:699]Unused DT entry: type 0x6ffffffe arg 0x00000f6c
    [ReadSoInfo:699]Unused DT entry: type 0x6fffffff arg 0x00000001
    [ReadSoInfo:703]=======================ReadSoInfo End=========================
    [RebuildShdr:42]=======================RebuildShdr=========================
    [RebuildShdr:536]=====================RebuildShdr End======================
    [RebuildRelocs:783]=======================RebuildRelocs=========================
    [RebuildRelocs:809]=======================RebuildRelocs End=======================
    [RebuildFin:709]=======================try to finish file rebuild =========================
    [RebuildFin:733]=======================End=========================
    [main:123]Done!!!
    /data/local/tmp/libmyjni.so.dump.so.fix.so: 1 file pulled, 0 skipped. 8.9 MB/s (25258 bytes in 0.003s)
    libmyjni.so_0xc141d000_24576_fix.so
```


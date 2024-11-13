/************************************************************
  FileName: ptraceInject.c
  Description:       ptrace注入      
***********************************************************/


#include <stdio.h>    
#include <stdlib.h>    
// #include <asm/user.h>    
#include <asm/ptrace.h>    
#include <sys/ptrace.h>    
#include <sys/wait.h>    
#include <sys/mman.h>    
#include <dlfcn.h>    
#include <dirent.h>    
#include <unistd.h>    
#include <string.h>    
#include <elf.h>    
#include <utils/PrintLog.h> 
#include <ptraceInject.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>

#define CPSR_T_MASK     ( 1u << 5 )

#define REMOTE_ADDR( addr, local_base, remote_base ) ( (uint32_t)(addr) + (uint32_t)(remote_base) - (uint32_t)(local_base) ) 

const char *libc_path = "/apex/com.android.runtime/lib/bionic/libc.so";    
const char *linker_path = "/apex/com.android.runtime/bin/linker";   

/*************************************************
  Description:    ptrace使远程进程继续运行
  Input:          pid表示远程进程的ID
  Output:         无
  Return:         返回0表示continue成功，返回-1表示失败
  Others:         无
*************************************************/
int ptrace_continue(pid_t pid)
{
	if (ptrace(PTRACE_CONT, pid, NULL, NULL) < 0)
	{
		printf("ptrace cont error, pid:%d", pid);
		return -1;
	}
	return 0;
}

/*************************************************
  Description:    使用ptrace Attach到指定进程
  Input:          pid表示远程进程的ID
  Output:         无
  Return:         返回0表示attach成功，返回-1表示失败
  Others:         无
*************************************************/  
int ptrace_attach(pid_t pid)    
{
	int status = 0;
	
    if (ptrace(PTRACE_ATTACH, pid, NULL, 0) < 0) {    
        printf("attach process error, pid:%d\n", pid);    
        return -1;    
    }    
	
 	printf("attach process pid:%d", pid);          
    waitpid(pid, &status , WUNTRACED);       	
    
    return 0;    
} 

/*************************************************
  Description:    使用ptrace detach指定进程
  Input:          pid表示远程进程的ID
  Output:         无
  Return:         返回0表示detach成功，返回-1表示失败
  Others:         无
*************************************************/    
int ptrace_detach(pid_t pid)    
{    
    if (ptrace(PTRACE_DETACH, pid, NULL, 0) < 0) {    
        printf("detach process error, pid:%d\n", pid);     
        return -1;    
    }    
    
	printf("detach process pid:%d\n", pid);  
    return 0;    
}

/*************************************************
  Description:    使用ptrace获取远程进程的寄存器值
  Input:          pid表示远程进程的ID，regs为pt_regs结构，存储了寄存器值
  Output:         无
  Return:         返回0表示获取寄存器成功，返回-1表示失败
  Others:         无
*************************************************/ 
int ptrace_getregs(pid_t pid, struct pt_regs *regs)
{
	if (ptrace(PTRACE_GETREGS, pid, NULL, regs) < 0)
	{
		printf("Get Regs error, pid:%d\n", pid);
		return -1;
	}
	
	return 0;
}

/*************************************************
  Description:    使用ptrace设置远程进程的寄存器值
  Input:          pid表示远程进程的ID，regs为pt_regs结构，存储需要修改的寄存器值
  Output:         无
  Return:         返回0表示设置寄存器成功，返回-1表示失败
  Others:         无
*************************************************/ 
int ptrace_setregs(pid_t pid, struct pt_regs *regs)
{
	if (ptrace(PTRACE_SETREGS, pid, NULL, regs) < 0)
	{
		printf("Set Regs error, pid:%d", pid);
		return -1;
	}
	
	return 0;
}

/*************************************************
  Description:    获取返回值，ARM处理器中返回值存放在ARM_r0寄存器中
  Input:          regs存储远程进程当前的寄存器值
  Output:         无
  Return:         在ARM处理器下返回r0寄存器值
  Others:         无
*************************************************/ 
long ptrace_getret(struct pt_regs * regs)    
{       
    return regs->ARM_r0;      
}

/*************************************************
  Description:    获取当前执行代码的地址，ARM处理器下存放在ARM_pc中
  Input:          regs存储远程进程当前的寄存器值
  Output:         无
  Return:         在ARM处理器下返回pc寄存器值
  Others:         无
*************************************************/ 
long ptrace_getpc(struct pt_regs * regs)    
{       
    return regs->ARM_pc;    
} 

/*************************************************
  Description:    使用ptrace从远程进程内存中读取数据
  Input:          pid表示远程进程的ID，pSrcBuf表示从远程进程读取数据的内存地址
				  pDestBuf表示用于存储读取出数据的地址，size表示读取数据的大小
  Output:         无
  Return:         返回0表示读取数据成功
  Others:         无
*************************************************/ 
int ptrace_readdata(pid_t pid, uint8_t *pSrcBuf, uint8_t *pDestBuf, uint32_t size)
{
	uint32_t nReadCount = 0;
	uint32_t nRemainCount = 0;
	uint8_t *pCurSrcBuf = pSrcBuf;
	uint8_t *pCurDestBuf = pDestBuf;
	long lTmpBuf = 0;
	uint32_t i = 0;	
 
	nReadCount = size / sizeof(long);
	nRemainCount = size % sizeof(long);

	for (i = 0; i < nReadCount; i ++ )
	{
		lTmpBuf = ptrace(PTRACE_PEEKTEXT, pid, pCurSrcBuf, 0);
		memcpy(pCurDestBuf, (char *)(&lTmpBuf), sizeof(long));
		pCurSrcBuf += sizeof(long);
		pCurDestBuf += sizeof(long);
	}

	if ( nRemainCount > 0 )
	{
		lTmpBuf = ptrace(PTRACE_PEEKTEXT, pid, pCurSrcBuf, 0);
		memcpy(pCurDestBuf, (char *)(&lTmpBuf), nRemainCount);
	}

	return 0;
}

// 
/*************************************************
  Description:    使用ptrace将数据写入到远程进程空间中
  Input:          pid表示远程进程的ID，pWriteAddr表示写入数据到远程进程的内存地址
				  pWriteData用于存储写入数据的地址，size表示写入数据的大小
  Output:         无
  Return:         返回0表示写入数据成功，返回-1表示写入数据失败
  Others:         无
*************************************************/ 
int ptrace_writedata(pid_t pid, uint8_t *pWriteAddr, uint8_t *pWriteData, uint32_t size)
{
	uint32_t nWriteCount = 0;
	uint32_t nRemainCount = 0;
	uint8_t *pCurSrcBuf = pWriteData;
	uint8_t *pCurDestBuf = pWriteAddr;
	long lTmpBuf = 0;
	uint32_t i = 0;
	
	nWriteCount = size / sizeof(long);
	nRemainCount = size % sizeof(long);

	// 先讲数据以sizeof(long)字节大小为单位写入到远程进程内存空间中
	for (i = 0; i < nWriteCount; i ++)
	{
		memcpy((void *)(&lTmpBuf), pCurSrcBuf, sizeof(long));
		if (ptrace(PTRACE_POKETEXT, pid, pCurDestBuf, lTmpBuf) < 0)  // PTRACE_POKETEXT表示从远程内存空间写入一个sizeof(long)大小的数据
		{
			printf("Write Remote Memory error, MemoryAddr:0x%lx", (long)pCurDestBuf);
			return -1;
		}
		pCurSrcBuf += sizeof(long);
		pCurDestBuf += sizeof(long);
	}
	
	// 将剩下的数据写入到远程进程内存空间中
	if (nRemainCount > 0)
	{
		lTmpBuf = ptrace(PTRACE_PEEKTEXT, pid, pCurDestBuf, NULL); //先取出原内存中的数据，然后将要写入的数据以单字节形式填充到低字节处
		memcpy((void *)(&lTmpBuf), pCurSrcBuf, nRemainCount);
		if (ptrace(PTRACE_POKETEXT, pid, pCurDestBuf, lTmpBuf) < 0)
		{
			printf("Write Remote Memory error, MemoryAddr:0x%lx", (long)pCurDestBuf);
			return -1;			
		}
	}
	
	return 0;	
}

/*************************************************
  Description:    使用ptrace远程call函数
  Input:          pid表示远程进程的ID，ExecuteAddr为远程进程函数的地址
                  parameters为函数参数的地址，regs为远程进程call函数前的寄存器环境
  Output:         无
  Return:         返回0表示call函数成功，返回-1表示失败
  Others:         无
*************************************************/ 
int ptrace_call(pid_t pid, uint32_t ExecuteAddr, long *parameters, long num_params, struct pt_regs* regs)    
{    
    int i = 0;
	// ARM处理器，函数传递参数，将前四个参数放到r0-r3，剩下的参数压入栈中
    for (i = 0; i < num_params && i < 4; i ++) {    
        regs->uregs[i] = parameters[i];    
    }    
    
    if (i < num_params) {    
        regs->ARM_sp -= (num_params - i) * sizeof(long) ;    // 分配栈空间，栈的方向是从高地址到低地址
        if (ptrace_writedata(pid, (void *)regs->ARM_sp, (uint8_t *)&parameters[i], (num_params - i) * sizeof(long))  == -1)
			return -1;
    }    
    
    regs->ARM_pc = ExecuteAddr;           //设置ARM_pc寄存器为需要调用的函数地址
    // 与BX跳转指令类似，判断跳转的地址位[0]是否为1，如果为1，则将CPST寄存器的标志T置位，解释为Thumb代码
	// 若为0，则将CPSR寄存器的标志T复位，解释为ARM代码
	if (regs->ARM_pc & 1) {    
        /* thumb */    
        regs->ARM_pc &= (~1u);    
        regs->ARM_cpsr |= CPSR_T_MASK;    
    } else {    
        /* arm */    
        regs->ARM_cpsr &= ~CPSR_T_MASK;    
    }    
    
    regs->ARM_lr = 0;        
    
    if (ptrace_setregs(pid, regs) == -1 || ptrace_continue(pid) == -1) {    
        printf("ptrace set regs or continue error, pid:%d", pid);  
        return -1;    
    }    
    
    int stat = 0;  

    waitpid(pid, &stat, WUNTRACED);  
    
	// 判断是否成功执行函数
    printf("ptrace call ret status is %d\n", stat); 
    while (stat != 0xb7f) {  
        if (ptrace_continue(pid) == -1) {  
            printf("ptrace call error\n");  
            return -1;  
        }  
        waitpid(pid, &stat, WUNTRACED);  
    }

	// 获取远程进程的寄存器值，方便获取返回值
	if (ptrace_getregs(pid, regs) == -1)
	{
		printf("After call getregs error\n");
		return -1;
	}
    
    return 0;    
}    

/*************************************************
  Description:    在指定进程中搜索对应模块的基址
  Input:          pid表示远程进程的ID，若为-1表示自身进程，ModuleName表示要搜索的模块的名称
  Output:         无
  Return:         返回0表示获取模块基址失败，返回非0为要搜索的模块基址
  Others:         无
*************************************************/ 
void* GetModuleBaseAddr(pid_t pid, const char* ModuleName)    
{
    FILE *fp = NULL;    
    long ModuleBaseAddr = 0; 	
	char *ModulePath, *MapFileLineItem;
    char szFileName[50] = {0};    
    char szMapFileLine[1024] = {0};
	char szProcessInfo[1024] = {0};
    
	// 读取"/proc/pid/maps"可以获得该进程加载的模块
    if (pid < 0) {    
        //  枚举自身进程模块 
        snprintf(szFileName, sizeof(szFileName), "/proc/self/maps");    
    } else {    
        snprintf(szFileName, sizeof(szFileName), "/proc/%d/maps", pid);    
    }    
    
    fp = fopen(szFileName, "r");    
    
    if (fp != NULL) 
	{      
        while (fgets(szMapFileLine, sizeof(szMapFileLine), fp)) {
			if (strstr(szMapFileLine, ModuleName))
			{
				printf(" pid %d find %s\n",pid,ModuleName);
				MapFileLineItem = strtok(szMapFileLine, " \t"); // 基址信息
                char *Addr = strtok(szMapFileLine, "-");    
                ModuleBaseAddr = strtoul(Addr, NULL, 16 );    
    
                if (ModuleBaseAddr == 0x8000)    
                    ModuleBaseAddr = 0;    
    
                break;   				
			}
        }    
    
        fclose(fp) ;    
    }    
    
    return (void *)ModuleBaseAddr;       
}    

/*************************************************
  Description:    获取远程进程与本进程都加载的模块中函数的地址
  Input:          pid表示远程进程的ID，ModuleName表示模块名称，LocalFuncAddr表示本地进程中该函数的地址
  Output:         无
  Return:         返回远程进程中对应函数的地址
  Others:         无
*************************************************/ 
void* GetRemoteFuncAddr(pid_t pid, const char *ModuleName, void *LocalFuncAddr)
{
	printf("------- [GetRemoteFuncAddr] -------\n");
	void *LocalModuleAddr, *RemoteModuleAddr, *RemoteFuncAddr;
	
	LocalModuleAddr = GetModuleBaseAddr(-1, ModuleName);
	RemoteModuleAddr = GetModuleBaseAddr(pid, ModuleName);
	auto offset = (long)LocalFuncAddr - (long)LocalModuleAddr;
	printf("ModuleName:%s LocalModuleAddr:%p  RemoteModuleAddr:%p LocalFuncAddr:%p offset:0x%x\n",ModuleName,LocalModuleAddr,RemoteModuleAddr,LocalFuncAddr,offset);

	RemoteFuncAddr = (void *)( offset + (long)RemoteModuleAddr);
	
	return RemoteFuncAddr;
}

uint32_t getSymbolOffset(const char* method_name);

void* GetRemoteFuncLinkerAddr(pid_t pid, const char *ModuleName, const char* method_name)
{
	printf("------- [GetRemoteFuncLinkerAddr] %s-------\n",method_name);
	void *RemoteModuleAddr, *RemoteFuncAddr;
	
	auto offset = getSymbolOffset(method_name);
	RemoteModuleAddr = GetModuleBaseAddr(pid, ModuleName);
	printf("ModuleName:%s   RemoteModuleAddr:%p  offset:0x%x\n",ModuleName,RemoteModuleAddr,offset);

	RemoteFuncAddr = (void *)( offset + (long)RemoteModuleAddr);
	
	return RemoteFuncAddr;
}

// 打印指定地址的十六进制数
void print_hex(const void* address, size_t num_bytes) {
    const uint8_t* byte_ptr = (const uint8_t*)address;
    for (size_t i = 0; i < num_bytes; ++i) {
        printf("%02x ", byte_ptr[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

uint32_t getSymbolOffset(const char* method_name) {
	int fd = open(linker_path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return -1;
    }

    void* map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }

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

    munmap(map, st.st_size);
    close(fd);
    return symbol_offset;
}

/*************************************************
  Description:    通过远程直接调用dlopen\dlsym的方法ptrace注入so模块到远程进程中
  Input:          pid表示远程进程的ID，LibPath为被远程注入的so模块路径，FunctionName为远程注入的模块后调用的函数
				  FuncParameter指向被远程调用函数的参数（若传递字符串，需要先将字符串写入到远程进程空间中），NumParameter为参数的个数
  Output:         无
  Return:         返回0表示注入成功，返回-1表示失败
  Others:         无
*************************************************/ 
int inject_remote_process(pid_t pid, char *LibPath, char *FunctionName, long *FuncParameter, long NumParameter)
{
	printf("======== inject_remote_process =======\n");
	int iRet = -1;
	struct pt_regs CurrentRegs, OriginalRegs;  // CurrentRegs表示远程进程中当前的寄存器值，OriginalRegs存储注入前的寄存器值，方便恢复
	void *mmap_addr, *dlopen_addr, *dlsym_addr, *dlclose_addr, *dlerror_addr;   // 远程进程中需要调用函数的地址
	void *RemoteMapMemoryAddr, *RemoteModuleAddr, *RemoteModuleFuncAddr; // RemoteMapMemoryAddr为远程进程空间中映射的内存基址，RemoteModuleAddr为远程注入的so模块加载基址，RemoteModuleFuncAddr为注入模块中需要调用的函数地址
	long parameters[6];  
	
	// Attach远程进程
	if (ptrace_attach(pid) == -1)
		return iRet;
	
	// 获取远程进程的寄存器值
	if (ptrace_getregs(pid, &CurrentRegs) == -1)
	{
		ptrace_detach(pid);
		return iRet;
	}

	printf("ARM_r0:0x%lx, ARM_r1:0x%lx, ARM_r2:0x%lx, ARM_r3:0x%lx, ARM_r4:0x%lx, ARM_r5:0x%lx, ARM_r6:0x%lx, ARM_r7:0x%lx, ARM_r8:0x%lx, ARM_r9:0x%lx, ARM_r10:0x%lx, ARM_ip:0x%lx, ARM_sp:0x%lx, ARM_lr:0x%lx, ARM_pc:0x%lx\n", \
		CurrentRegs.ARM_r0, CurrentRegs.ARM_r1, CurrentRegs.ARM_r2, CurrentRegs.ARM_r3, CurrentRegs.ARM_r4, CurrentRegs.ARM_r5, CurrentRegs.ARM_r6, CurrentRegs.ARM_r7, CurrentRegs.ARM_r8, CurrentRegs.ARM_r9, CurrentRegs.ARM_r10, CurrentRegs.ARM_ip, CurrentRegs.ARM_sp, CurrentRegs.ARM_lr, CurrentRegs.ARM_pc);
	
	// 保存远程进程空间中当前的上下文寄存器环境
	memcpy(&OriginalRegs, &CurrentRegs, sizeof(CurrentRegs)); 
	
	void * init = (void *)ptrace_getret(&CurrentRegs);
	printf("Remote init Addr:0x%lx\n", (long)init);

	// 获取mmap函数在远程进程中的地址
	mmap_addr = GetRemoteFuncAddr(pid, libc_path, (void *)mmap);
	printf("mmap RemoteFuncAddr:0x%lx\n", (long)mmap_addr);
	
	// 设置mmap的参数
	// void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offsize);
    parameters[0] = 0;  // 设置为NULL表示让系统自动选择分配内存的地址    
    parameters[1] = 0x1000; // 映射内存的大小    
    parameters[2] = PROT_READ | PROT_WRITE | PROT_EXEC;  // 表示映射内存区域可读可写可执行   
    parameters[3] =  MAP_ANONYMOUS | MAP_PRIVATE; // 建立匿名映射    
    parameters[4] = 0; //  若需要映射文件到内存中，则为文件的fd  
    parameters[5] = 0; //文件映射偏移量 	
	
	// 调用远程进程的mmap函数，建立远程进程的内存映射
	if (ptrace_call(pid, (long)mmap_addr, parameters, 6, &CurrentRegs) == -1)
	{
		printf("Call Remote mmap Func Failed\n");
		ptrace_detach(pid);
		return iRet;
	}
	
	// 获取mmap函数执行后的返回值，也就是内存映射的起始地址
	RemoteMapMemoryAddr = (void *)ptrace_getret(&CurrentRegs);
	printf("Remote Process Map Memory Addr:0x%lx\n", (long)RemoteMapMemoryAddr);

	// 分别获取dlopen、dlsym、dlclose等函数的地址
	dlopen_addr = GetRemoteFuncLinkerAddr(pid, linker_path, "__loader_dlopen");
	dlsym_addr = GetRemoteFuncLinkerAddr(pid, linker_path, "__loader_dlsym");
	dlclose_addr = GetRemoteFuncLinkerAddr(pid, linker_path, "__loader_dlclose");
	dlerror_addr = GetRemoteFuncLinkerAddr(pid, linker_path, "__loader_dlerror");

	printf("dlopen RemoteFuncAddr:0x%lx\n", (long)dlopen_addr);
	printf("dlsym RemoteFuncAddr:0x%lx\n", (long)dlsym_addr);
	printf("dlclose RemoteFuncAddr:0x%lx\n", (long)dlclose_addr);
	printf("dlerror RemoteFuncAddr:0x%lx\n", (long)dlerror_addr);
	
	// 将要加载的so库路径写入到远程进程内存空间中
	if (ptrace_writedata(pid, RemoteMapMemoryAddr, LibPath, strlen(LibPath) + 1) == -1)
	{
		printf("Write LibPath:%s to RemoteProcess error\n", LibPath);
		ptrace_detach(pid);
		return iRet;
	}

	// 设置dlopen的参数,返回值为模块加载的地址
	// void *dlopen(const char *filename, int flag);
	parameters[0] = (long)RemoteMapMemoryAddr;
	parameters[1] = RTLD_NOW| RTLD_GLOBAL;
	
	if (ptrace_call(pid, (long)dlopen_addr, parameters, 2, &CurrentRegs) == -1)
	{
		printf("Call Remote dlopen Func Failed\n");
		ptrace_detach(pid);
		return iRet;
	}

	// RemoteModuleAddr为远程进程加载注入模块的地址
	RemoteModuleAddr = (void *)ptrace_getret(&CurrentRegs);
	printf("Remote Process load module Addr:0x%lx\n", (long)RemoteModuleAddr);
	
	if ((long)RemoteModuleAddr == 0x0)   // dlopen 错误
	{
		printf("dlopen error\n");
		if (ptrace_call(pid, (long)dlerror_addr, parameters, 0, &CurrentRegs) == -1)
		{
			printf("Call Remote dlerror Func Failed\n");
			ptrace_detach(pid);
			return iRet;
		}
		char *Error = (void *)ptrace_getret(&CurrentRegs);
		char LocalErrorInfo[1024] = {0};
		ptrace_readdata(pid, Error, LocalErrorInfo, 1024);
		printf("dlopen error:%s\n", LocalErrorInfo);
		ptrace_detach(pid);
		return iRet;
	}	
	
	// 将so库中需要调用的函数名称写入到远程进程内存空间中
	if (ptrace_writedata(pid, RemoteMapMemoryAddr + strlen(LibPath) + 2, FunctionName, strlen(FunctionName) + 1) == -1)
	{
		printf("Write FunctionName:%s to RemoteProcess error\n", FunctionName);
		ptrace_detach(pid);
		return iRet;
	}
	
	// 设置dlsym的参数，返回值为远程进程内函数的地址
	// void *dlsym(void *handle, const char *symbol);
	parameters[0] = (long)RemoteModuleAddr;
	parameters[1] = (long)(RemoteMapMemoryAddr + strlen(LibPath) + 2);
	
	if (ptrace_call(pid, (long)dlsym_addr, parameters, 2, &CurrentRegs) == -1)
	{
		printf("Call Remote dlsym Func Failed\n");
		ptrace_detach(pid);
		return iRet;
	}	
	
	// RemoteModuleFuncAddr为远程进程空间内获取的函数地址
	RemoteModuleFuncAddr = (void *)ptrace_getret(&CurrentRegs);
	printf("Remote Process ModuleFunc Addr:0x%lx\n", (long)RemoteModuleFuncAddr);
	
	if (ptrace_call(pid, (long)RemoteModuleFuncAddr, FuncParameter, NumParameter, &CurrentRegs) == -1)
	{
		printf("Call Remote injected Func Failed\n");
		ptrace_detach(pid);
		return iRet;
	}	

	if (ptrace_setregs(pid, &OriginalRegs) == -1)
	{
		printf("Recover reges failed\n");
		ptrace_detach(pid);
		return iRet;		
	}
	printf("Recover Regs Success\n");
	ptrace_getregs(pid, &CurrentRegs);
	if (memcmp(&OriginalRegs, &CurrentRegs, sizeof(CurrentRegs)) != 0)
	{
		printf("Set Regs Error\n");
	}
	
	//Detach
	if (ptrace_detach(pid) == -1)
	{
		printf("ptrace detach failed\n");
		return iRet;
	}
	
	return 0;
}

/*************************************************
  Description:    通过shellcode方式ptrace注入so模块到远程进程中
  Input:          pid表示远程进程的ID，LibPath为被远程注入的so模块路径，FunctionName为远程注入的模块后调用的函数
				  FuncParameter指向被远程调用函数的参数（若传递字符串，需要先将字符串写入到远程进程空间中），NumParameter为参数的个数
  Output:         无
  Return:         返回0表示注入成功，返回-1表示失败
  Others:         无
*************************************************/ 
int inject_remote_process_shellcode(pid_t pid, char *LibPath, char *FunctionName, long *FuncParameter, long NumParameter)
{
	printf("======== inject_remote_process_shellcode =======\n");
	int iRet = -1;
	struct pt_regs CurrentRegs, OriginalRegs;  // CurrentRegs表示远程进程中当前的寄存器值，OriginalRegs存储注入前的寄存器值，方便恢复
	void *mmap_addr, *dlopen_addr, *dlsym_addr, *dlclose_addr, *dlerror_addr;   // 远程进程中需要调用函数的地址
	void *RemoteMapMemoryAddr, *RemoteModuleAddr, *RemoteModuleFuncAddr; // RemoteMapMemoryAddr为远程进程空间中映射的内存基址，RemoteModuleAddr为远程注入的so模块加载基址，RemoteModuleFuncAddr为注入模块中需要调用的函数地址
	long parameters[10];  
	
	uint8_t *dlopen_param1_ptr, *dlsym_param2_ptr, *saved_r0_pc_ptr, *inject_param_ptr, *remote_code_start_ptr, *local_code_start_ptr, *local_code_end_ptr;
	
	extern uint32_t _dlopen_addr_s, _dlopen_param1_s, _dlopen_param2_s, _dlsym_addr_s, \
			_dlsym_param2_s, _dlclose_addr_s, _inject_start_s, _inject_end_s, _inject_function_param_s, \
			_saved_cpsr_s, _saved_r0_pc_s;	
			
	uint32_t code_length;
	
	// Attach远程进程
	if (ptrace_attach(pid) == -1)
		return iRet;
	
	// 获取远程进程的寄存器值
	if (ptrace_getregs(pid, &CurrentRegs) == -1)
	{
		ptrace_detach(pid);
		return iRet;
	}
	
	printf("ARM_r0:0x%lx, ARM_r1:0x%lx, ARM_r2:0x%lx, ARM_r3:0x%lx, ARM_r4:0x%lx, ARM_r5:0x%lx, ARM_r6:0x%lx, ARM_r7:0x%lx, ARM_r8:0x%lx, ARM_r9:0x%lx, ARM_r10:0x%lx, ARM_ip:0x%lx, ARM_sp:0x%lx, ARM_lr:0x%lx, ARM_pc:0x%lx\n", \
		CurrentRegs.ARM_r0, CurrentRegs.ARM_r1, CurrentRegs.ARM_r2, CurrentRegs.ARM_r3, CurrentRegs.ARM_r4, CurrentRegs.ARM_r5, CurrentRegs.ARM_r6, CurrentRegs.ARM_r7, CurrentRegs.ARM_r8, CurrentRegs.ARM_r9, CurrentRegs.ARM_r10, CurrentRegs.ARM_ip, CurrentRegs.ARM_sp, CurrentRegs.ARM_lr, CurrentRegs.ARM_pc);
	
	// 保存远程进程空间中当前的上下文寄存器环境
	memcpy(&OriginalRegs, &CurrentRegs, sizeof(CurrentRegs)); 
	
	// 获取mmap函数在远程进程中的地址
	mmap_addr = GetRemoteFuncAddr(pid, libc_path, (void *)mmap);
	printf("mmap RemoteFuncAddr:0x%lx\n", (long)mmap_addr);
	
	// 设置mmap的参数
	// void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offsize);
    parameters[0] = 0;  // 设置为NULL表示让系统自动选择分配内存的地址    
    parameters[1] = 0x4000; // 映射内存的大小    
    parameters[2] = PROT_READ | PROT_WRITE | PROT_EXEC;  // 表示映射内存区域可读可写可执行   
    parameters[3] =  MAP_ANONYMOUS | MAP_PRIVATE; // 建立匿名映射    
    parameters[4] = 0; //  若需要映射文件到内存中，则为文件的fd  
    parameters[5] = 0; //文件映射偏移量 	
	
	// 调用远程进程的mmap函数，建立远程进程的内存映射
	if (ptrace_call(pid, (long)mmap_addr, parameters, 6, &CurrentRegs) == -1)
	{
		printf("Call Remote mmap Func Failed\n");
		ptrace_detach(pid);
		return iRet;
	}
	
	// 获取mmap函数执行后的返回值，也就是内存映射的起始地址
	RemoteMapMemoryAddr = (void *)ptrace_getret(&CurrentRegs);
	printf("Remote Process Map Memory Addr:0x%lx\n", (long)RemoteMapMemoryAddr);
	
	// 分别获取dlopen、dlsym、dlclose等函数的地址
	dlopen_addr = GetRemoteFuncLinkerAddr(pid, linker_path, "__loader_dlopen");
	dlsym_addr = GetRemoteFuncLinkerAddr(pid, linker_path, "__loader_dlsym");
	dlclose_addr = GetRemoteFuncLinkerAddr(pid, linker_path, "__loader_dlclose");
	dlerror_addr = GetRemoteFuncLinkerAddr(pid, linker_path, "__loader_dlerror");
	
	printf("dlopen RemoteFuncAddr:0x%lx\n", (long)dlopen_addr);
	printf("dlsym RemoteFuncAddr:0x%lx\n", (long)dlsym_addr);
	printf("dlclose RemoteFuncAddr:0x%lx\n", (long)dlclose_addr);
	printf("dlerror RemoteFuncAddr:0x%lx\n", (long)dlerror_addr);	

	remote_code_start_ptr = RemoteMapMemoryAddr + 0x3C00;    // 远程进程中存放shellcode代码的起始地址
	local_code_start_ptr = (uint8_t *)&_inject_start_s;     // 本地进程中shellcode的起始地址
	local_code_end_ptr = (uint8_t *)&_inject_end_s;          // 本地进程中shellcode的结束地址

	_dlopen_addr_s = (uint32_t)dlopen_addr;
	_dlsym_addr_s = (uint32_t)dlsym_addr;
	_dlclose_addr_s = (uint32_t)dlclose_addr;

	printf("Inject Code Start:0x%x, end:0x%x\n", (int)local_code_start_ptr, (int)local_code_end_ptr);
	
	// 计算shellcode中一些变量的存放起始地址
	code_length = (uint32_t)&_inject_end_s - (uint32_t)&_inject_start_s;//32条指令，4字节大小
	dlopen_param1_ptr = local_code_start_ptr + code_length + 0x20;//inject进程中字符串的地址
	dlsym_param2_ptr = dlopen_param1_ptr + MAX_PATH;
	saved_r0_pc_ptr = dlsym_param2_ptr + MAX_PATH;
	inject_param_ptr = saved_r0_pc_ptr + MAX_PATH;

	printf("code_length : %d  dlopen_param1_ptr:%p\n",code_length,dlopen_param1_ptr);

	// 写入dlopen的参数LibPath
	strcpy( dlopen_param1_ptr, LibPath );
	_dlopen_param1_s = REMOTE_ADDR( dlopen_param1_ptr, local_code_start_ptr, remote_code_start_ptr );//计算字符串offset，加上远程code基址，得到远端进程的地址

	// 写入dlsym的第二个参数，需要调用的函数名称
	strcpy( dlsym_param2_ptr, FunctionName );
	_dlsym_param2_s = REMOTE_ADDR( dlsym_param2_ptr, local_code_start_ptr, remote_code_start_ptr );

	//保存cpsr寄存器
	_saved_cpsr_s = OriginalRegs.ARM_cpsr;

	//保存r0-pc寄存器
	memcpy( saved_r0_pc_ptr, &(OriginalRegs.ARM_r0), 16 * 4 ); // r0 ~ r15
	_saved_r0_pc_s = REMOTE_ADDR( saved_r0_pc_ptr, local_code_start_ptr, remote_code_start_ptr );

	memcpy( inject_param_ptr, FuncParameter, NumParameter );
	_inject_function_param_s = REMOTE_ADDR( inject_param_ptr, local_code_start_ptr, remote_code_start_ptr );

	ptrace_writedata( pid, remote_code_start_ptr, local_code_start_ptr, 0x400 );

	memcpy( &CurrentRegs, &OriginalRegs, sizeof(CurrentRegs) );
	CurrentRegs.ARM_sp = (long)remote_code_start_ptr;//设置sp和pc为远端code起始地址
	CurrentRegs.ARM_pc = (long)remote_code_start_ptr;
	ptrace_setregs( pid, &CurrentRegs );//执行远端code
	ptrace_detach( pid );
	
	return 0;	
}         
## 游戏安全 手游安全技术入门

腾讯游戏研发部游戏安全中心出版的手游入门，个人感觉收益匪浅，可惜有点老了，也没有新版继续更新，甚感可惜了，简单记录一下观后感，方便查阅。

## 基础知识

外挂可分为两大类：辅助版外挂和破解版外挂。这两类外挂的核心区别在于：是 否需要依赖游戏客户端。辅助版外挂是需要结合游戏客户端运行的；破解版外挂则是 可独立运行的非法客户端。

### 外挂的定义

辅助版外挂：

辅助版外挂需要依赖游戏客户端，不能独自生效。根据其作用范围可以再划分为 两个小类：专用插件和通用工具。专用插件如叉叉助手和圈圈助手。通用工具类外挂可分 为：通用内存修改器、变速器、按键精灵、模拟器、抓包工具。

破解版外挂：

在本质上是一个非法客户端。常见的破解版外挂可分为两类：脱机挂 和修改游戏原客户端的破解版外挂。

### 游戏玩法分类

#### MMORPG 类游戏

MMORPG 类游戏（Massively Multiplayer Online Role・Playing Game）即大型多人在线角色扮演游戏，例如三国志、魔兽争霸系列，反恐精英，实时MMORPG拥有“野外”概念，玩家在野外可以和怪物直接进行战斗。MMORPG代表游戏有《梦幻西游》《大话西游》《热血传奇》《神武2》《六龙争霸》。

#### FPS类游戏

FPS类游戏(First Person Shooter)即第1人称射击游戏，是以主视角进行的射击 游戏。玩家从显示设备模拟出的主角的视点中观察存在的物体并进行射击、运动、跳 跃、对话等活动。FPS代表游戏有《穿越火线》《全民枪王》《全民枪战》。

#### ARPG类游戏

ARPG类游戏（Action Role-Playing Game）即动作角色扮演游戏属于电子游戏类 型，是指将动作游戏、角色扮演游戏（RPG）和冒险游戏的要素合并的作品，ARPG的代表游戏有《火影忍者》《全民无双》《艾尔战记》等。

#### 卡牌类游戏

手机卡牌游戏主要指运行在智能手机平台上的以卡牌形式发行的游戏，卡牌类的代表游戏有《拳皇98终极之战》《炉石传说》等。

#### RTS类游戏

RTS类游戏(Real.time Strategy, RTS)为即时战略游戏，是战略游戏的一种。顾 名思义，游戏的过程是即时进行而不是釆用回合制的。RTS的代表游戏有《部落冲突》《COK列王的战争》等。

#### 消除类游戏

消除类游戏是益智游戏的一种，玩家在游戏过程中主要是将一定量的相同游戏元 素如水果、宝石、动物头像或积木麻将牌等彼此相邻且配对消除来获胜。消除类的代表游戏有《开心消消乐》《天天爱消除》等。

####  MOBA类游戏

MOBA类游戏(Multiplayer Online Battle Arena)即多人在线竞技游戏，是即时战 略游戏的一个子类。玩家被分为两队，单个玩家只能控制其中一队的一个角色。MOBA的代表游戏有《王者荣耀》《全民超神》等。

#### 跑酷类游戏

跑酷类游戏是动作游戏的一种，主要玩法是由玩家控制游戏人物不断地前行，同时需要躲避时不时出现的障碍物。《天天酷跑》手游属于典型的跑酷类游戏。

### 游戏系统相关概念

#### 手游系统

有很多组成部分，就商业化的手游来说，一般比较重要的有登录系统、 成长系统、战斗系统、经济系统（包含物品系统和交易系统）、邮件系统（可能包含 任务系统）、聊天系统等。我们现在讨论的正是这些系统，下面逐一介绍这些系统的 一些概念和功能。

#### 手游网络模式

强联网和弱联网的概念是不针对游戏的，而是针对游戏模式的，比如一般游戏的 PVE模式是弱联网的，PVP模式是强联网的。

#### 游戏引擎

游戏的运行实现可以切分成4个层，从下往上依次是硬件层、三方功能组件层、游戏引擎层、游戏逻辑层。

![](images\图片1.png)

重要部件包括：渲染系统、音频系统、 物理系统和人工智能。

##### 渲染系统

游戏引擎的渲染系统，一般是基于成熟的图像API实现的，比如DirectX或者 OpenGL,这些API封装了 GPU和显卡的部分功能。渲染系统的实现涉及浓淡处理、 纹理映射、距离模糊、阴影、反射、透明等较多的复杂技术的算法，透视外挂都是关闭引擎的z-buffer，导致在墙壁等阻碍物后 面的人物也能被渲染出来。

##### 主流引擎

目前国内的移动游戏，除去部分自研引擎，比较主流的游戏引擎有两个：Cocos2D 和Unity 3D。

### 游戏漏洞概述

安全界对漏洞的定义为：在硬件、软件、系统等具体实现或者系统安全策略中存 在的缺陷，使攻击者能够达到某种破坏目的。以漏洞的实现原理为依据，可将游戏安全漏洞划分成三大类：游戏逻辑漏洞、协议稳定型漏洞、服务端校验疏忽漏洞。

#### 动作类角色扮演类（ACT&RPG）的手游的风险表

| 风险点                 | 外挂行为                                                     | 外挂危害                                               |
| ---------------------- | ------------------------------------------------------------ | ------------------------------------------------------ |
| 血量                   | 修改人物血量和怪物血量                                       | 弱怪、秒怪、无敌                                       |
| 伤害                   | 修改人物受到的伤害和怪物受到的伤害                           | 秒怪、无敌                                             |
| 技能***\*CD\****       | 技能少***\*CD\****                                           | 快速发技能、无限技能                                   |
| 直接胜利               | 七动发送胜利请求，不产生怪物                                 | 快速瞬间获取收益                                       |
| 技能***\*ID\****       | 修改角色技能***\*ID\****                                     | 使用未获得的技能、使用高级技能、使用他人 技能          |
| ***\*buffid\****       | 修改自己或者敌方当前的***\*buffid\****为其他***\*buffid\**** | 增强自己技能的***\*buff\****、削弱敌方的***\*buff\**** |
| ***\*buff\****持续时间 | 增加***\*buff\****时长、锁定***\*buff\****时长               | 我方的***\*buff\****无限，减少敌方的***\*buff\****时间 |
| 定怪                   | 使怪物不动                                                   | 快速杀怪通关                                           |
| 怪物不攻击             | 怪物不攻击玩家                                               | 不掉血、快速杀怪通关                                   |
| 全屏                   | 全屏攻击命中                                                 | 快速大范围杀敌                                         |
| 穿透                   | 使所有攻击从主角身上穿透过去                                 | 主角无敌状态                                           |
| 属性                   | 修改英雄属性和装备属性                                       | 相应属性有所加成，主角更易通关或获取分数               |
| 物品数量               | 修改物品数量和无限物品                                       | 可无限使用物品的效果或者技能的效果                     |

#### 卡牌类角色扮演类（卡牌RPG）手游的风险表

| 风险点                 | 外挂行为                                                     | 外挂危害                                               |
| ---------------------- | ------------------------------------------------------------ | ------------------------------------------------------ |
| 血量                   | 修改人物血量和怪物血最                                       | 弱怪、秒怪、无敌                                       |
| 伤害                   | 修改人物受到的伤害和怪物受到的伤害                           | 秒怪、无敌                                             |
| 技能***\*CD\****       | 技能少***\*CD\****                                           | 快速发技能、无限技能                                   |
| 直接胜利               | 主动发送胜利请求，不产生怪物                                 | 快速瞬间获取收益                                       |
| 技能***\*ID\****       | 修改角色技能***\*ID\****                                     | 使用未获得的技能、使用高级技能、使用他人 技能.         |
| ***\*buffid\****       | 修改自己或者敌方当前的***\*buffid\****为其他***\*buffid\**** | 增强自己技能的***\*buff\****、削弱敌方的***\*buff\**** |
| ***\*buff\****持续时冋 | 增加***\*buff\****时长、锁定***\*buff\****时长               | 我方的***\*buff\****无限.减少敌方的***\*bufF\****时间  |
| 怪物不攻击             | 怪物不攻击玩家                                               | 不掉血、快速杀怪通关                                   |
| 全屏                   | 全屏攻击命中                                                 | 快速大范围杀敌                                         |
| 穿透                   | 使所有攻击从主角身上穿透过去                                 | 主角无敌状态                                           |
| 属性                   | 修改英雄属性、装备属性                                       | 相应属性有所加成，主角更易通关或获取分数               |
| 物品数量               | 修改物品数量、无限物品                                       | 可无限使用物品的效果或者技能的效果                     |

#### 竞速类手游的风险表

| 风险点                 | 外挂行为                                         | 外挂危害                                      |
| ---------------------- | ------------------------------------------------ | --------------------------------------------- |
| 修改距离               | 修改竞速距离、跳跃距离                           | 刷排名、获取髙收益                            |
| 修改分数               | 修改无尽模式得分                                 | 高分数高收益                                  |
| 技能***\*CD\****       | 技能少***\*CD\****                               | 快速发技能、无限技能                          |
| 道具***\*ID\****       | 修改道貝***\*1D\****                             | 使用未获得的道具                              |
| 技能***\*ID\****       | 修改角色技能***\*ID\****                         | 使用未获得的技能、使用高级技能、使用他人 技能 |
| ***\*buffid\****       | 修改自己的***\*buffid\****为其他***\*buffid\**** | 增强自己技能的***\*buff\****                  |
| ***\*buff\****持续时间 | 增加***\*buff\****时长、锁定***\*buff\****时长   | 我方的***\*buff\****无限                      |

#### 射击类手游的风险表如表

| 风险点   | 外挂行为                           | 外挂危害                                 |
| -------- | ---------------------------------- | ---------------------------------------- |
| 血量     | 修改人物血量和怪物血量             | 弱怪、秒怪、无敌                         |
| 伤害     | 修改人物受到的伤害和怪物受到的伤害 | 秒怪、无敌                               |
| 枪枪爆头 | 只要击中敌人，就被游戏当作爆头攻击 | 高伤害快速杀故                           |
| 子弹数   | 修改增加弹夹里面的子弹             | 不用换弹夹、无限子弹                     |
| 换弹时间 | ***\*0\****秒换弹                  | 实时对战不用换弹                         |
| 修改分数 | 修改高分                           | 刷排名、获取高收益                       |
| 全屏     | 全屏攻击命中                       | 快速大范围杀敌                           |
| 敌人生成 | 整局不生成故人，直接通关           | 快速通关                                 |
| 穿透     | 使所有攻击从主角身上穿透过去       | 主角无敌状态                             |
| 属性值   | 修改角色、装备或者物品的属性       | 相应属性有所加成，主角更易通关或获取分数 |
| 物品数量 | 修改物品数量、无限物品             | 可无限使用物品的效果或者技能的效果       |

#### 塔防类手游的风险表

| 风险点     | 外挂行为                     | 外挂危害                       |
| ---------- | ---------------------------- | ------------------------------ |
| 金币       | 增加局内金币，买更多的建筑   | 快速杀怪通关                   |
| 建筑时间   | 减少建筑时间                 | 更快生成建筑                   |
| 直接胜利   | 主动发送胜利请求，不产生怪物 | 快速、瞬间获取收益             |
| 定怪       | 使怪物不动                   | 不受伤害杀怪通关               |
| 攻击频率   | 修改塔，使其不断高频率攻击   | 快速杀怪通关                   |
| 攻击速度   | 修改塔的攻击速度             | 快速杀怪通关                   |
| 怪物不攻击 | 怪物不攻击玩家               | 不掉血快速杀怪通关             |
| 攻击范围   | 修改塔的攻击范围             | 大范围杀怪                     |
| 属性值     | 修改角色、装备和物品的属性   | 相应属性有所加成，主角更易通关 |
| 物品数量   | 修改物品的数量、无限物品     | 可无限使用物品的效果或者技能   |
| 移动速度   | 怪物不移动或者移动慢         | 降低通关难度，争取时间建塔     |

#### 消除类手游的风险表

| 风险点                 | 外挂行为                                       | 外挂危害                           |
| ---------------------- | ---------------------------------------------- | ---------------------------------- |
| 分数                   | 修改分数                                       | 刷排名、获取髙收益                 |
| 消除类型               | 任意类型消除                                   | 任意消除、快速通关                 |
| ***\*buffid\****       | 修改***\*buffid\****为其他***\*buffid\****     | 增强自己的***\*buff\****           |
| ***\*buff\****持续时间 | 增加***\*buff\****时长，锁定***\*buff\****时长 | 我方的***\*buff\****无限           |
| 道具***\*ID\****       | 修改道具***\*ID\****                           | 使用未获得的道具                   |
| 蓄气                   | 快速蓄气                                       | 快速蓄气、高频率使用技能           |
| 技能***\*CD\****       | 技能少***\*CD\****、技能无***\*CD\****         | 快速发技能、无限技能               |
| 物品数量               | 修改物品数量、无限物品                         | 可无限使用物品的效果或者技能的效果 |

#### 音乐类手游的风险表

| 风险点                 | 外挂行为                                           | 外挂危害                       |
| ---------------------- | -------------------------------------------------- | ------------------------------ |
| 分数                   | 修改分数                                           | 刷排名，获取高收益             |
| 命中类型               | 音符生成，任意类型的音符消除                       | 全***\*perfect\****通关        |
| 音符速度               | 减少音符的速度                                     | 降低全***\*combo\****难度通关  |
| ***\*buffid\****       | 修改自己的***\*buffid\****为其他***\*buffid\****   | 增强自己技能的***\*buff\****   |
| ***\*buff\****持续时间 | 增加***\*buff\****的时长，锁定***\*buff\****的时长 | 我方***\*buff\****无限         |
| 物品或者技能数量       | 修改物品或者技能数量、无限物品                     | 可无限使用物品效果或者技能效果 |

#### MOBA类手游的风险表

| 风险点                 | 外挂行为                                                     | 外挂危害                                               |
| ---------------------- | ------------------------------------------------------------ | ------------------------------------------------------ |
| 血量                   | 修改人物血量和怪物血量                                       | 弱怪、秒怪、无敌                                       |
| 伤害                   | 修改人物受到的伤害和怪物受到的伤害                           | 秒怪、无故                                             |
| 技能***\*CD\****       | 技能少***\*CD\****                                           | 快速发技能、无限技能                                   |
| 属性                   | 修改英雄属性和装备属性                                       | 相应属性有所加成，主角更易通关或获取分数               |
| ***\*buffid\****       | 修改自己或者敌方的当前***\*buffid\****为其他***\*buffid\**** | 增强自己技能的***\*buff,\****削弱敌方的***\*buff\****  |
| ***\*buff\****持续时间 | 增加***\*buff\****时长，锁定***\*buff\****时长               | 我方的***\*buff\****无限，减少敌方的***\*buff\****时间 |
| 定怪                   | 使怪物不动                                                   | 快速杀怪通关                                           |
| 怪物不攻击             | 怪物不攻击玩家                                               | 不掉血且快速杀怪通关                                   |
| 全屏                   | 全屏攻击命中                                                 | 快速大范围杀敌                                         |
| 穿透                   | 使所有攻击从主角身上穿透过去                                 | 主角无敌状态                                           |
| 物品数量               | 修改物品数量、无限物品                                       | 可无限使用物品的效果或者技能的效果                     |

## ARM指令样例解析

### 指令格式解析

#### B/BL指令

B/BL指令是最常用的几条指令，作用是跳转到指定的位置。其二进制解释如图

![](images\2.png)

对图10.2中重要的信息解释如下。

（1）	Cond位表示这条指令会在什么条件下执行，其实每个条件都会给B指令加 上后缀，比如“BNE/BEQ/BGT” 等,也就是我们熟悉的条件跳转指令，作为程序的 分支使用。

（2）	“101”是B指令的标识码。

（3）	L位表示是否需要保存当前指令的下一条指令地址至LR寄存器，BL指令一 般用于跳转到函数内部，其中的LR用于保存返回地址。

（4）	Offset是目标地址与该指令的相对偏移（需要考虑到CPU的指令预读，并左 移两位，偏移是按4字节对齐的）。

如图10.3所示是IDA反汇编中的一个条件跳转示例。

![](images\3.png)

图10.3中的两条指令表示“如果R2等于R0,则跳转到loc_1C04标号所在的地址”。 如图10.4所示的指令为图10.3中的条件判断指令，下面分析其中各个字段的 含义。

![](images\4.png)

对应的十六进制机器码如图10.5所示。

![](images\5.png)

按照B/BL指令的二进制解释图10.2中的格式排列(从高位到低位)，描述如下。

(1) Cond对应的二进制数据为0000,表示EQ操作，当R2等于R0时发生跳转。

(2) 101对应的二进制数据为101,表示固定标识。

(3) L对应的二进制数据为0,表示不需要保存下一条指令。

(4) Offset对应的二进制数据为0000 0000 0000 0000 0000 1011,表示计算目标跳 转地址所需的偏移信息。

#### LDR/STR 指令

LDR/STR指令用于向内存中读、写数据，对指令的二进制解释如图10.6所示。

![](images\6.png)

LDR/STR指令解释图详细描述了指令的内存结构及执行原理。下面通过实例介 绍指令的处理方式。LDR指令对应的实例如图10.7所示。

![](images\7.png)

LDR指令对应的十六进制机器码如图10.8所示。

![](images\8.png)

图10.8中的LDR指令按照图10.6的二进制格式排列如下：

```
1110	01	011001	1100	0010	0000 0000 0000
Cond	op	IPUBWL	Rn		Rd		Offset
```

其中，实例指令中的每个字段的含义如下。

（1）	Cond表示任何条件都可以。

（2）	01是指令标识。

（3）	I是0,表示offset是一个立即数（即在立即寻址方式指令中给出的数值）。

（4）	B是0,表示传输的是一个字。

（5）	L是1,表示读内存。

（6）	Rn是OxC,表示基址寄存器是R12.

（7）	Rd是2,表示把内存值读到R2。

（8）	Offset是0,表示基址寄存器需要加上0。

依据以上解释，对应的指令为：LDR R2, [R12]o

“LDR R2, [R12]”指令的实际含义为：读取R12寄存器指向的内存到R2寄存 器中，一共读取4个字节。

### 函数传参

#### arm32传参

从汇编层面来看，不同的编译方式对函数传参的做法不同，一种比较常见的做法是把前4个参数放到R0〜3寄存器中，把剩下的参数放到栈中，把函数的返回值放在 R0中。

##### 直接调用传参

```
int func(int a, int b, int c, int d, int e) {
    int v1 = 1;
    int r = a + b + c + d + e + v1;
    return r;
}
int main() {
    int i = 1, j = 2;
    int r = func(i, j, 3, 4, 5);
    printf("%d \n", r);
}
```

对应main函数的汇编代码

```
; int __cdecl main(int argc, const char **argv, const char **envp)

PUSH            {R11,LR}                    ;保存现场
ADD             R11, SP, #4                 ;新栈的基址 FP(BP)
SUB             SP, SP, #0x18               ;开辟栈空间
MOV             R3, #1                      ;r3 = 1
STR             R3, [R11,#var_8]            ;[r11+var_8] = 1
MOV             R3, #2                      ;r3 = 2
STR             R3, [R11,#var_C]            ;[r11+var_C] = 2
MOV             R3, #5                      ;r3 = 5
STR             R3, [SP,#0x1C+var_1C] ; int ;[sp+0x1c+var_1c] = 5
LDR             R0, [R11,#var_8]            ;r0 = [r11+var_8] = 1    
LDR             R1, [R11,#var_C]            ;r1 = [r11+var_C] = 2
MOV             R2, #3                      ;r2 = 3
MOV             R3, #4                      ;r3 = 4
BL              _Z4funciiiii ; func(int,int,int,int,int)
```

此时栈及寄存器如下：

![](images\10.png)

对应func函数的汇编代码

```
; _DWORD __fastcall func(int, int, int, int, int)
PUSH            {R11}
ADD             R11, SP, #0
SUB             SP, SP, #0x1C
STR             R0, [R11,#var_10]   ; r0 = 1
STR             R1, [R11,#var_14]   ; r1 = 2
STR             R2, [R11,#var_18]   ; r2 = 3
STR             R3, [R11,#var_1C]   ; r3 = 4
MOV             R3, #1
STR             R3, [R11,#var_8]
LDR             R2, [R11,#var_10]
LDR             R3, [R11,#var_14]
ADD             R2, R2, R3 ; 1 + 2
LDR             R3, [R11,#var_18]
ADD             R2, R2, R3 ; 1+2+3
LDR             R3, [R11,#var_1C]
ADD             R2, R2, R3 ; 1+2+3+4
LDR             R3, [R11,#arg_0]    ; 注意这里 r11+4，就是在main中传入的第5个参数
ADD             R2, R2, R3
```



![](images\11.png)

结论： 传递基本数据类型，前4个放入寄存器r0-r3中，多于的放入栈上。

##### 传递数组

```
void func6(int a[], int len) {
    for (int i = 0; i < len; i++) {
        printf("index: %d , num: %d\n", i, a[i]);
    }
}
int main() {
    int a[] = {0, 1, 2};
    int n = sizeof(a) / sizeof(int);
    func6(a, n);
}
```

main函数对应的汇编

```
PUSH            {R11,LR}
ADD             R11, SP, #4
SUB             SP, SP, #0x10
LDR             R2, =(dword_20BC - 0x8C8)
ADD             R2, PC, R2 ; dword_20BC     ; r2存储数组首地址
SUB             R3, R11, #-var_14
LDM             R2, {R0-R2}                 ; 把数组[0,1,2]内容存到r0,r1,r2
STM             R3, {R0-R2}                 ; 再存到r3所指向的栈上
MOV             R3, #3
STR             R3, [R11,#var_8]
SUB             R3, R11, #-var_14           ; 获取数组在栈上首地址。数组首地址放到r0中，作为参数传递
MOV             R0, R3  ; int *
LDR             R1, [R11,#var_8] ; int
BL              _Z5func6Pii                 ; func6(int *,int)
MOV             R3, #0
MOV             R0, R3
SUB             SP, R11, #4
POP             {R11,PC}
```

结论：传递数组 实际上传递的是数组首地址指针，取数组内容通过首地址偏移

##### 结构体的传递

```
struct st_a {
    int a;
    int b;
    int c;
    int d;
    int e;
};
void func1(st_a stA) {
    int a = stA.a;
    int b = stA.b;
    int c = stA.c;
    int d = stA.d;
    int e = stA.e;
    printf("%d, %d \n", a, b, c, d, e);
}
int main() {
    st_a stA;
    stA.a = 123;
    stA.b = 234;
    stA.c = 345;
    stA.d = 456;
    stA.e = 567;
    func1(stA);
}
```

main函数对应汇编

```
PUSH            {R11,LR}
ADD             R11, SP, #4
SUB             SP, SP, #0x20
MOV             R3, #123
STR             R3, [R11,#var_18]       ; 123 放到栈上[r11-0x18]
MOV             R3, #234
STR             R3, [R11,#var_14]       ; 234 放到栈上[r11-0x14]
LDR             R3, =345
STR             R3, [R11,#var_10]       ; 345 放到栈上[r11-0x10]
MOV             R3, #456
STR             R3, [R11,#var_C]        ; 456 放到栈上[r11-0xc]
LDR             R3, =567
STR             R3, [R11,#var_8]        ; 567 放到栈上[r11-0x8]
LDR             R3, [R11,#var_8]
STR             R3, [SP,#0x24+var_24]   ; 567 放到栈上[sp]
SUB             R3, R11, #-var_18
LDM             R3, {R0-R3}             ; 将 123 234 345 456放到寄存器r0,r1,r2,r3
BL              _Z5func14st_a ; func1(st_a)
MOV             R3, #0
MOV             R0, R3
SUB             SP, R11, #4
POP             {R11,PC}
```

结论：结构体的传递和直接传递参数一样 --> func1(int a, int b, int c, int d, int e)

##### 传递枚举

```
enum en_a {
    p1 = 103,
    p2 = 201
};
void func2(en_a enA) {
    printf("%d \n", enA);
}
int main() {
    func2(p1);
}
```

```
PUSH            {R11,LR}
ADD             R11, SP, #4
MOV             R0, #103
BL              _Z5func24en_a ; func2(en_a)
MOV             R3, #0
MOV             R0, R3
POP             {R11,PC}
```

结论：传递枚举，直接传递的值 类似于 func2(int a)

##### 传递对象指针

```
class Person {
public:
    Person() {
        printf("调用\n");
    }

    int get() {
        return this->a;
    }

    int getb() {
        return this->b;
    }

    void set(int pa, int pb) {
        this->a = pa;
        this->b = pb;
    }

private:
    int a;
    int b;
};
void func3(Person *person) {
    printf("%d \n", person->get());
}
int main() {
    Person person;
    person.set(102, 222);
    func3(&person);
}
```

```
PUSH            {R11,LR}
ADD             R11, SP, #4
SUB             SP, SP, #8
SUB             R3, R11, #-var_C
MOV             R0, R3              ; this  分配栈上地址，调用构造函数
BL              _ZN6PersonC2Ev      ; Person::Person(void)
SUB             R3, R11, #-var_C
MOV             R0, R3  ; this
MOV             R1, #102 ; int
MOV             R2, #222 ; int
BL              _ZN6Person3setEii   ; 调用函数的时候，会多传一个参数，为对象分配的地址  Person::set(int,int)
SUB             R3, R11, #-var_C
MOV             R0, R3              ; 传递的是栈上地址 Person * 
BL              _Z5func3P6Person    ; func3(Person *)
MOV             R3, #0
MOV             R0, R3
SUB             SP, R11, #4
POP             {R11,PC}
```

结论：传递对象指针 传递过来的是地址

##### 传递对象引用

```
void func4(Person &person) {
    printf("%d \n", person.get());
}
int main() {
    func4(person);
}
```

```
PUSH            {R11,LR}
ADD             R11, SP, #4
SUB             SP, SP, #8
SUB             R3, R11, #-var_C
MOV             R0, R3              ; this
BL              _ZN6PersonC2Ev      ; Person::Person(void)
SUB             R3, R11, #-var_C
MOV             R0, R3  ; this
MOV             R1, #102 ; int
MOV             R2, #222 ; int
BL              _ZN6Person3setEii   ; Person::set(int,int)
SUB             R3, R11, #-var_C
MOV             R0, R3  ; Person *
BL              _Z5func4R6Person    ; func4(Person &)
MOV             R3, #0
MOV             R0, R3
SUB             SP, R11, #4
POP             {R11,PC}
```

传递对象引用，和传递对象地址是一样的。

##### 传递对象

```
void func5(Person person) {
    printf("a: %d ,b: %d\n", person.get(), person.getb());
    person.set(102, 333);
}
int main() {
    func5(person);
}
```

main函数汇编

```
PUSH            {R11,LR}
ADD             R11, SP, #4
SUB             SP, SP, #8
SUB             R3, R11, #-var_C
MOV             R0, R3              ; this
BL              _ZN6PersonC2Ev      ; Person::Person(void)
SUB             R3, R11, #-var_C
MOV             R0, R3  ; this
MOV             R1, #102 ; int
MOV             R2, #222 ; int
BL              _ZN6Person3setEii   ; Person::set(int,int)
SUB             R3, R11, #-var_C
LDM             R3, {R0,R1}         ; 将对象的属性赋值给r1和r2
BL              _Z5func56Person     ; func5(Person)
MOV             R3, #0
MOV             R0, R3
SUB             SP, R11, #4
POP             {R11,PC}
```

func5对应的部分汇编代码

```
PUSH            {R4,R11,LR}
ADD             R11, SP, #8
SUB             SP, SP, #0xC
SUB             R3, R11, #-var_14
STM             R3, {R0,R1}         ; 重新为对象分配空间并赋值
SUB             R3, R11, #-var_14
MOV             R0, R3  ; this
BL              _ZN6Person3getEv    ; Person::get(void)
```

结论：传递对象 传递过来的是成员变量值，在被调用方再分配空间。 c++对象内存模型中只有成员变量占对象内存空间

#### arm32寄存器分布

r0（通用寄存器）：用于存储函数的返回值或临时变量。

r1（通用寄存器）：用于存储函数参数或临时变量。

r2（通用寄存器）：用于存储函数参数或临时变量。

r3（通用寄存器）：用于存储函数参数或临时变量。

r4-r11（通用寄存器）：用于存储临时变量。

 r12（通用寄存器）：用于存储临时变量。

sp（r13 堆栈指针）：用于指向当前栈顶。

 lr（r14 链接寄存器）：用于存储函数返回地址。

 pc（r15 程序计数器）：用于存储下一条指令的地址。

cpsr（状态寄存器）：用于存储当前程序状态，例如条件码、中断控制、模式等。

 spsr（保存状态寄存器）：用于存储先前cpsr的值，以便在中断返回时恢复状态。

 ip（内部寄存器）：用于存储临时变量。

 fp（帧寄存器）：用于存储当前函数的帧指针，指向当前函数的第一个局部变量。

cpsr_c（协处理器寄存器）：用于存储协处理器状态信息。

 cpsr_x（扩展状态寄存器）：用于存储扩展状态信息。

#### arm64寄存器分布

x0-x7：传递子程序的参数和返回值，使用时不需要保存，多余的参数用堆栈传递，64位的返回结果保存在x0中。

x8：用于保存子程序的返回地址，使用时不需要保存。

x9-x15：临时寄存器，也叫可变寄存器，子程序使用时不需要保存。

x16-x17：子程序内部调用寄存器（IPx），使用时不需要保存，尽量不要使用。

x18：平台寄存器，它的使用与平台相关，尽量不要使用。

x19-x28：临时寄存器，子程序使用时必须保存。

x29：帧指针寄存器（FP），用于连接栈帧，使用时必须保存。

x30：链接寄存器（LR），用于保存子程序的返回地址。

x31：堆栈指针寄存器（SP），用于指向每个函数的栈顶。

# 注入技术的实现原理

## Android平台ptrace注入技术

### 实现原理

目前有两种实现ptrace注入模块到远程进 程的方法，

第1种方法是直接远程调用dlopen> dlsym等函 数加载被注入模块并执行指定的代码。

第2种方法是使用ptrace将shellcode注入远程进程的内存空间中，然后通 过执行shellcode加载远程进程模块；

![](\images\12.png)

1. 附加到远程进程上

   ptrace注入的第1步是附加到远程进程上，调用方式如下：

   ptrace(PTRACE_ATTACHr pid, NULL, NULL);

   附加到远程进程上是通过调用request参数为PTRACE_ATTACH的ptrace函数， pid为需要附加的进程ID, addr参数和data参数为NULL。

   在附加到远程进程后，远程进程的执行会被中断，此时父进程可以通过调用 waitpid函数来判断子进程是否进入暂停状态。waitpid的函数原型如下：

   Pid_t waitpid (pid__t pid, int * status, int options);

   其中，options参数为WUNTRACED时，表示如果对应pid的远程进程进入暂停状态, 则函数马上返回，可用于等待远程进程进入暂停状态。

2.  读取和写入寄存器值

   在通过ptrace改变远程进程的执行流程之前，需要先读取和保存远程进程的所有 寄存器的值，当detach操作发生时，可将远程进程写入已保存的原寄存器的值，用于 恢复远程进程原有的执行环境。

   在实现读取和写入寄存器的值时，调用ptrace函数的request参数分别为 PTRACE_GETREGS和PTRACE_SETREGS。对应调用的代码如下：

   ptrace(PTRACE_GETREGSr pid, NULL, regs);

   ptrace(PTRACE_SETREGSr pid, NULL, regs);

   在ARM处理器下，ptrace函数中data参数的regs为pt_regs结构的指针，从远程 进程获取的寄存器值将存储到该结构中。

3. 远程进程的内存读取和写入数据

   调用request参数为PTRACE_PEEKTEXT的ptrace函数可以从远程进程的内存空 间中读取数据，一次读取一个word大小的数据。该函数调用的实现如下：

   ptrace(PTRACE_PEEKTEXTZ pid, pCurSrcBufz 0);

   ptrace(PTRACE_POKETEXTZ pid, pCurDestBufx ITmpBuf);

   其中addr参数为需要读取数据的远程进程内存地址，返回值为读取的数据。

   调用request参数为PTRACE_POKETEXT的ptrace函数可以将数据写入远程进程 的内存空间中，同样一次写入一个word大小的数据。ptrace函数的addr参数为需要 写入数据的远程进程的内存地址，data参数为需要写入的数据内容。

   在写入数据时需要注意，若写入的数据长度不是word大小的倍数，则在写入最 后一个不足word大小的数据时，要先保存原地址处的高位数据。

4.  远程调用函数

   在ARM处理器中，函数调用的前4个参数通过R0.R3寄存器来传递，剩余的参 数按从右到左的顺序压入栈中进行传递。实现代码如下：

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

   在远程调用函数之前，需要先判断函数调用的参数的个数，如果小于4个，则将 参数按顺序分别写入R0-R3寄存器中，若大于4个，则首先调整SP寄存器在栈中 分配的空间大小，然后通过调用ptrace函数将剩余的参数写入栈中。

   在写入函数的参数后，修改进程的PC寄存器为需要执行的函数地址。这里有一 点需要注意，在ARM架构下有ARM和Thumb两种指令，因此在调用函数前需要判 断函数被解析成哪种指令。可通过地址的最低位是否为1来判断调用地址处的指令是 为ARM还是Thumbo若为Thumb指令，则需要将最低位重新设置为0,并且将CPSR 寄存器的T标志位置位：若为ARM指令，则将CPSR寄存器的T标志位复位。

5. 恢复寄存器值

   在远程进程执行detach操作之前，需要将远程进程的原寄存器的环境恢复，保证 远程进程原有的执行流程不被破坏。如果不恢复寄存器的值，则执行detach操作之后 会导致远程进程崩溃。

6. Detach 进程

   脱离远程进程是ptrace注入的最后一个步骤，在执行detach操作之后，被注入进 程将继续运行。detach函数的调用如下：

   ```
   ptrace(PTRACE_DETACH, pid, NULL, 0);
   ```

### ptrace 实例测试

使用2048小游戏进行注入测试，ptraceInject代码地址：

https://github.com/Soon-gz/blog/tree/main/ProjectDocs/gameSafe/code/

- 确定需要注入的进程名：com.estoty.game2048
- 注入模块全路径：/data/local/tmp/source13/libInjectModule.so （注意，高版本需要放在应用安装目录下，避免dlopen没有权限）
- 注入模块后调用模块函数名称：Inject_entry
- 需要注意不同版本libc和linker在maps的映射路径
- /apex/com.android.runtime/bin/linker
- /apex/com.android.runtime/lib/bionic/libc.so
- 注意：书籍中的代码获取dlopen等函数的起始地址方式在高版本不适用，可以通过解析动态表，拿到符号表以及字符串表地址，然后解析获取偏移地址，我在实验时用的IDA手动找当前手机的linker对应dlopen的偏移地址，所以在测试的时候，需要根据不同手机做修改。

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

注入成功后，inject 输出的日志：

![](images\13.png)

logcat输出的日志：

![](\images\16.png)

### shellcode 方式测试

shellcode注入和dlopen方式类似，也是需要先找到远程进程的dlopen等函数地址，然后将地址复制给shellcode.s的汇编变量，然后将shellcode的汇编通过ptracce注入到远程mmap申请的内存空间中，然后设置寄存器的pc和sp为远端code的地址，最后执行code，代码中有详细的注释以及测试内容。

inject输出以下内容：

![](\images\18.png)

logcat输出以下内容：

![](\images\17.png)

## Android平台Zygote注入技术

代码地址：https://github.com/Soon-gz/blog/tree/main/ProjectDocs/gameSafe/code/

Zygote是一个很重要的进程，因为绝大部分的应用程序进程都是由Zygote进程 “fork”生成的。“fork”是Linux操作系统中的一种进程复用技术，在这里需要了解的 是，如果进程A执行fork操作生成了进程B,那么进程B在创建时便拥有和进程A 完全相同的模块信息。

![](\images\14.png)

Zygote注入需要注意如下两个关键点。

- 目标进程需要在注入Zygote完成后启动，才能成功被注入。
- 成功注入Zygote之后启动的新进程将包含己注入Zygote的模块信息，所以 需要在新启动的进程执行前获得控制权，然后判断当前进程是否为目标进程，如果是, 则执行其余代码，否则交还控制权。

![](\images\15.png)

如图流程阐述了注入器怎样跨进程执行代码，以及在Linux进程之间通信。

注入器总共执行了如下三次跨进程调用。

(1) 首先调用mmap函数申请目标进程的地址空间，用于保存注入的shellcode 代码。

(2) 执行注入的shellcode (shellcode是注入目标进程中并执行的汇编代码)。

(3) 调用munmap函数释放之前申请的内存。

注入器的各功能的实现过程。

1.  关闭 SeLinux  

   adb shell ,执行以下命令

   ```
   setenforce 0
   ```

2. 附加到Zygote,保存进程现场

   使用ptrace附加进程。

   ```
   res = ptrace(PTRACE_ATTACHZ pid, NULL, NULL);
   ```

3. 获取Zygote进程申关键函数的地址

4. 调用关键函数，如mmap函数

5. 配置 shellcode

   shellcode是一段汇编代码，对函数地址、字符串地址的引用都使用了相对地址。

6. 远程调用shellcode。

7. 调用munmap释放内存
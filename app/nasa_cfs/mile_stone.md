阶段 0：前置准备与缺陷修复（你主导）
做什么
1. 修复内核 VMM 缺陷：DelVaFromTree 空实现（vmm.c:1085）、PE 头泄漏（loader.c）、DoCommit 的 lpIndexMgr 空指针。
2. hxlib 的 errno 改为线程局部（TLS）—— cFS 重度多线程，这个是硬前提。
3. 复制源码：cFS → CFS_HOME_DIR/cFS，hxlib → CFS_HOME_DIR/hxlib。
4. 目录就位 + git 初始化。
完成标准（客观可验证）
- 内核缺陷修复后，VM 重启，hxlib 的 mqueue 压力测试（20 轮）连续多次稳定通过，无偶发 page fault / EBADF。
- errno 已是线程局部（多线程下各自独立）。
是否需确认：✅ 确认点 0 —— 你确认缺陷修好、环境就位，我再开始写移植代码。

阶段 1：构建系统 + adapt 层（我做）
做什么
1. 写 CFS_HOME_DIR/script/Makefile（clang --target=i686-w64-windows-gnu + ld.lld，glob 收集 cFE/OSAL(shared+portable+hellox)/PSP(hellox+shared)/apps 的 .c）。
2. 写配置：osconfig.h（关 DYNAMIC_LOADER/SHELL/IPv6，开 STATIC_LOADER）、cfe_platform_cfg.h/cfe_mission_cfg.h（从 sample_defs 裁剪）。
3. 写启动脚本 cfe_es_startup.scr（静态符号名）。
4. adapt 层：clock_settime/clock_getres/clock_nanosleep、getpid/geteuid/getegid/sysconf/strerror、sched_get_priority_max/min + SCHED_FIFO/RR。
说明：adapt 预期很薄——因为 hellox OSAL 是新写的，直接用 hxlib 原生原语（pthread/sem/mqueue/SetTimer/文件/网络），不照搬 POSIX 版的 getpid/sysconf/strerror 等，所以真正需要补的只有少数几个。
完成标准
- make 能编译链接出 PE32 exe（此时 OSAL/PSP 用最小桩，目标是先把工具链+配置+源文件收集跑通）。
是否需确认：⚠️ 确认点 1 —— 你确认"能编译出 exe、Makefile 方案、配置文件裁剪范围"合理。

阶段 2：OSAL hellox 移植层（我做，最核心）
做什么
- 新建 osal/src/os/hellox/{inc,src}，实现全部 OS_*_Impl：task、mutex、binsem、countsem、condvar、queue（用 mqueue）、timebase/timer（用 SetTimer+线程+链表）、file、dir、console、network、以及动态加载的 no-loader 桩。
- 复用 osal/src/os/portable/ 的 file/dir/socket/select 等。
完成标准
- OSAL 编译通过；OSAL 测试用例（queue/timer/timebase/mutex/binsem/countsem/task/file/dir）在 HelloX 上通过；shell/network 等未实现的在测试报告里标注"未跑"。
是否需确认：✅ 确认点 2 —— 你验收 OSAL 测试结果，确认后进入 PSP。

阶段 3：PSP hellox（我做）
做什么
- 新建 psp/fsw/hellox/：cfe_psp_start/support/memory/watchdog/ssr/exception.c + cfe_psp_config.h。
- 模块：soft_timebase + eeprom_notimpl + ram_notimpl + port_notimpl + iodriver。
- 按决策：Reset=POR、异常=abort+打印、watchdog/中断=NOT_IMPLEMENTED、内存保留区=volatile。
完成标准
- PSP 编译通过；cFE ES 能启动并进入 OPERATIONAL 状态（打印启动日志）。
是否需确认：✅ 确认点 3 —— 你验收"cFE 能启动"这个关键里程碑。

阶段 4：cFE + apps 集成联调（我做代码，你测试）
做什么
- 静态链接 apps（ci_lab/to_lab/sch_lab/sample_app/sample_lib），用 OS_STATIC_LOADER + OS_STATIC_SYMBOL_TABLE 注册入口。
- 启动脚本用静态符号名，确保 ES 能加载启动 app。
完成标准
- cFE ES OPERATIONAL，ci_lab/to_lab/sch_lab 正常启动，能收发命令、输出遥测（telnet/串口观察）。
是否需确认：✅ 确认点 4 —— 你验收功能，确认核心 cFS 跑通。

阶段 5：测试报告 + 收尾（我做，你验收）
做什么
- 输出 OSAL 测试报告（明确列出跑了哪些、哪些没跑、原因）。
- 代码整理 + git 分段提交到正式 code base。
完成标准
- 测试报告完整；代码提交完成，可回滚。
是否需确认：✅ 确认点 5 —— 你最终验收并确认提交。

功能三：时钟中断 PC 采样（最通用，直接定位热点函数）
这是 Linux perf/oprofile 的思路，也最通用——不关心线程在干嘛，直接回答“它把时间花在哪个函数”。
- 在系统时钟中断里（system.c 的 tick 处理），若当前被抢占线程 == 目标线程，记录一次它的 EIP/PC。
- 采样一定次数（如 1000 次）后，按 PC 聚合出热点分布（PC → 命中次数）。
- 符号解析：结合用户态模块加载基址（你已有 PE 加载器 + 静态符号表 cfe_static_symbol_list），把 PC 映射到“模块名 + 函数名/偏移”；解析不了就先打印 模块名 + 偏移，离线用 nm/objdump 查。
能定位：长占用型线程卡在哪个具体函数、死循环在哪一行附近。这一层对 cFS、对任何用户程序、甚至内核线程都通用。
通用化设计要点清单
- 目标选择接口：统一成一个 set_target(name|id)，内部解析成 thread id 存下来；钩子/中断里只做一次 id 比较。
- 数据容器：所有 trace 用固定大小环形缓冲 + 原子计数器；直方图用定长数组。杜绝动态分配、杜绝在中断里打印。
- 开关与查询：在现有 sysdiag 视图下加几个命令，例如 threadprof <target>、threadtrace <target>、threadpc <target>，复用 cpuload/showstk 那套 message→stat 线程的框架（STAT_S.C）。
- 开销控制：默认全关；只有被选中的目标才付出钩子/采样开销；PC 采样可配频率（每 tick / 每 N tick）。
- 符号：PC 采样要真正好用，必须接上模块加载信息 + 符号表，这一步通用化价值最大，建议单独做成“地址→模块/符号”的查询功能（内核里维护已加载模块列表 + 各自符号表）。

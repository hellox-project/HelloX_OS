Architecture level decision for porting cFS to HelloX:
1. Only use HelloX user application library(hxlib) as support platform, compile the whole cFS into a EXE binary that could be loaded by HelloX kernel;
2. Whole project's source and output are put into ~/hellox/v190/app/nasa_cfs directory, named CFS_HOME_DIR. hxlib is under CFS_HOME_DIR/hxlib, cFS source
   code is under CFS_HOME_DIR/cFS, make file and other scripts under CFS_HOME_DIR/script, put compiling output such as obj files into CFS_HOME_DIR/build,
   put the binary EXE file into CFS_HOME_DIR/bin;
3. Do not touch any source code of hxlib, since it's managed in another project. If there are issues or gaps, or un-offered functions, please give a specific
   requirement, I will modify or enhance hxlib in another project, then copy it back to CFS_HOME_DIR/hxlib;
4. Put any adaptation code into CFS_HOME_DIR/adapt directory, the adaptation refers the code generated based on hxlib to support cFS, when hxlib does not 
   offer directly, but can implemented by using current hxlib's capabilities. 也就是hxlib 缺口补齐层：clock_settime/clock_getres/clock_nanosleep、getpid/geteuid/getegid/sysconf/strerror、sched_get_priority_max/min 等"本应在 hxlib、暂时还没"的函数（用 hxlib 原语实现）。基于hxlib提供的API实现的timebase机制代码、与OSAL移植直接相关的适配HelloX OS的代码，则放在cFS/osal/os/hellox和cFS/psp/fsw/hellox/目录下；
5. Do not use signal mechanism, since HelloX kernel can not support it now;
6. cFS源代码结构：保持cFS源代码组织架构不变。在移植时，只在OSAL模块中增加OS相关目录（os/hellox/inc and os/hellox/src, etc)，所有适配HelloX OS的代码都放在该目录下；
7. 移植完成后，用OSAL提供的测试用例进行测试。对于shell等不支持的功能，可暂不测试，但是要输出测试报告、在测试报告中明确说明哪些测试用例没有跑；
8. 关于timebase与timer的实现：按Claude的解释，timebase是周期性的时间触发源，类似system tick， 定期的触发，而timer则是附着在timebase上的具体定时器，timer的定时周期必须是timebase的整数倍，因此在实现上，每当OSAL调用一次TimeBaseCreate的时候，就创建一个独立线程，专门以消息循环方式处理 TIMER消息，在OSAL调用TimeBaseSet的时候，再创建定时器并关联到线程；当OSAL调用TimerAdd的时候， 应该是把这个timer挂载到一个链表上，在timebase到期的处理函数中，遍历链表，找到到时的timer，并 调用其回调处理函数；在OSAL调用TimeBaseDelete时，通知后台线程退出（以消息方式）。注：虽然当前HelloX的timer是ms级，但未来会进一步增强到更高分辨率的定时机制，当前直接用HelloX的定时器机制即可，无需做额外增强；
9. FileSystem（不是file I/O）当前可以不做复杂实现，用 OS_FileSysAddFixedMap 把 /ram、/cf 映射到 HelloX 目录（c:\app\cfs\ram，c:\app\cfs\cf，这些目录需提前创建好，如果目录不存在则输出明显的错误提示信息)，应该不会影响cFS的核心功能；
10. Reset type of BSP/PSP just return POR(Power On Reset), and exception handler just invoke abort/exit, but show a message be fore invoking;
11. Memory Reserved Region: just keep volatile, return POR;
12. Watch dog, interrupt, just mark "NOT_IMPLEMENTED";
13. 用git控制整个项目，确保支持回滚功能。开发过程中，阶段性与我确认进展、确认后再提交到正式code base。如果开发过程导致进度失控，可以回滚到先前版本；
14. 构建系统：手写 Makefile，仿照 hxlib 的 clang --target=i686-w64-windows-gnu + ld.lld，glob 收集 cFE/OSAL(shared+portable+hellox)/PSP(hellox+shared)/apps 的 .c 文件，静态链接成单一 PE32 exe；
15. 编译构建和加载：cFE + OSAL + PSP + apps（ci_lab/to_lab/sch_lab/sample_app/sample_lib）全部静态链接进 exe；用 OS_STATIC_LOADER + OS_STATIC_SYMBOL_TABLE 注册 app 入口；启动脚本 cfe_es_startup.scr 用静态符号名（不动态加载 .so），并用 cFS 的 EMBED_FILELIST 机制把脚本内嵌进 exe；编译后的exe文件，统一命名为cfs.exe（注：目前HelloX只支持8.3文件名格式）；
16. 关于PSP：PSP 在 cFS/psp/fsw/hellox/ 新建平台目录（cfe_psp_start/support/memory/watchdog/ssr/exception.c + cfe_psp_config.h），并明确 CFE_SYSTEM_PSPNAME=hellox、模块选用 soft_timebase + eeprom_notimpl + ram_notimpl + port_notimpl + iodriver；
17. cFE配置：osconfig.h中，关 DYNAMIC_LOADER、SHELL、IPv6，开 STATIC_LOADER；
18. 功能开关默认值（写死）
    - 动态加载：关（静态链接）
    - shell：关
    - IPv6：关
    - 网络 socket：开（地面站通信用）
19. 移植过程中，每个步骤完成或过程中，请按下列方式进行测试，测试通过后再进入下一个步骤：

调试说明：移植过程中可按下列方法进行测试：
1. 启动VirtualBox虚拟平台，启动hellox_v190虚拟机；
2. 虚拟机连接方式：
   - telnet方式，找到虚拟机获取的IP地址（在本物理机上，IP地址是：192.168.1.204），telnet连接，然后输命令行即可；
   - 串口连接方式，通过VM的串口可直接连接上，命令行交互即可；
   - 在输入命令行之前，先输入几个回车键，确保HelloX的输入缓冲区换行。否则命令解析有问题。输入命令后再输入回车，即可执行命令；
3. 连接到VM后，启动FTP服务（仅仅第一次时需要），
   - 命令行：loadapp c:\ftp\ftpd.exe &
   - 注意最后的&一定要添加，否则CLI会阻塞
4. 然后在本机上通过FTP client上传exe文件。FTP登录用户和密码都是“ftp”三个小写字母。上传的目录是c:\ftp\
5. 上传后，可通过fs命令查看：fs进入文件系统视图，cd进入ftp目录，dir命令查看，确认后exit命令退出；
6. 上传成功后，执行loadapp c:\ftp\hxlib.exe & 加载执行，查看输出。



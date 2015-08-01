**** CHINESE VERSION ****

感谢试用HelloX V1.78测试版。
在bin目录下，已经生成了一个虚拟硬盘vdisk.vhd，可以用这个虚拟硬盘直接引导虚拟机。
如果希望通过USB来引导物理计算机，则运行bin目录下的make_usb_boot程序（运行前，先把USB盘插入计算机），格式化一个USB引导盘，再重新启动计算机即可。注意，制作USB启动盘之前，请先备份里面的数据。同时，需要设置计算机的引导顺序，确保USB引导优先。
祝您使用愉快！

各目录的主要内容如下：
/app：存放了基于HelloX开发的一些应用程序，主要是基于GUI模式开发的一些测试程序；
/bin：存放了可以直接引导虚拟机的虚拟硬盘（VHD）文件，以及生成引导物理计算机的相关工具和原始二进制文件；
/gui：HelloX GUI模块源代码；
/kernel：HelloX内核源代码，包含内核/网络协议栈/Java虚拟机/文件系统等部分的源代码；
/sdk：用于开发HelloX所需的相关文件，用于应用程序开发；
/tools：存放了支撑HelloX开发及应用相关的工具的源代码，比如引导设备制作工具，二进制处理工具，等等。

任何问题，欢迎加入QQ群讨论：38467832

**** ENGLISH VERSION ****

Welcome to use HelloX V1.78 beta version.
A virtual hard disk image was created under the /bin directory,which can be used to load virtual machine directly.The main stream virtual machines can be well supported by HelloX,such as Microsoft Virtual PC 2007,WMWare workstation,Oracle VirtualBox,Microsoft Hyper-V.
If you want to load actual PC through HelloX,please make a loadable USB stick by running make_usb_boot program under the /bin directory,then load PC.Please make sure the loading sequence of your PC is proper,so that USB stick is used before hard disk or other available loadable facilite.
**CAUTION**:PLEASE BACKUP YOUR USB DATA BEFORE RUN MAKE_USB_BOOT,SINCE IT WILL ERASE THE USB STICK.
Wish you can enjoy it.:-)

Main content of each directory as follows:
/app: Contains some applications developed for HelloX,especial GUI application;
/bin: Contains a already created virtual hard disk,and related images and tools to create loadable USB stick;
/gui: GUI source code of HelloX;
/kernel: Kernel code of HelloX,include essential kernel,network stack,Java VM,fiel system and others;
/sdk: Files to support HelloX application's developing under Microsoft visual studio;
/tools: The source code of some tools,which are used to assist HelloX's kernel and application's developing.

Please join QQ group to discuss if you have any question:38467832

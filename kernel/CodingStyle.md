#	Coding Style

## 前言
	随着内核项目不断增大，对于代码规范也是有一定的必要性的，当然这个规范也不可
	能很完善，也不能适应任何人，所以基本上都还是以建议为主，不能强加于人。当然
	但是有些硬性规定（带“*”的）还是必要统一的， 大家有好的建议也可以补充完善。

## 代码文件*
- 文件编码统一为：UTF-8
- 文件名统一为：小写，中间可以用“`_`”或者“`-`”分割，但不要同时出现  
	比如：`sys_arch.h`或者`·sys-arch.h`
- 大小写：文件名和代码内容统一

## 缩进*
- 统一为tab为缩进符号
- tab的宽度建议为4个，也可以为8个，但是在同一个文件要统一

## 静态局部变量
- 可以采用“`__`”/“`_`”开头。  
	比如：`__COMMON_OBJECT`, `_kThread`

## 全局变量/（extern）
- 可以用“`g_`”或者“`h_`”作为名字开头, 后面全部小写，单词用“`_`”分割  
	如：`g_object_type_driver/h_object_type_driver`
- 要么全部大写，中间单词中间用“`_`”分割  
	比如：`OBJECT_TYPE_DRIVER`
	
##  宏
- 最好是全部大写，单词之间使用“`_`”分割，最好前后各增加“`_`”。  
	比如：`_POSIX_`， `_X86_`

## 结构体
	
## typedef
- typedef的成员名字最好以“`_t`”,以区分结构体的变量
	
## 函数名
- 驼峰命名：`WriteDeviceSector`
- 全部小写：`write_device_sector`

## 函数参数
- 参数列表，每个参数之间在“，”之后增加一个空格（“` `”）  
	比如:`number(char *str, long num, int base, int size, int precision, int type)`

## 命名缩写
- 单词之间用“`_`”分割。
- 特别是感觉比较疑惑的命名尽量不要使用单词缩写。  
	比如不要把`connect_count`写成`connect_cnt`。 好的方式：`connect_count`
- 对于已经形成共识命名，可以多用缩写。  
	比如：`fopen, stdio, std, err, no, retval(return value)，comm`


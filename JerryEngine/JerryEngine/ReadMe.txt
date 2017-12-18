The souce code of JerryScript is downloaded in 2016.12 aroud...
Now is the early morning of Chicken year's first day in China Traditional Clendar(Lunnar).It's the must 
import day of one year in China.Now the clock is 0:06,I'm sitting on bed with fireworks soundly outside.
The time pass so fast,write several codes to memory the new year.
Here are the main points when port JerryScript to HelloX:
1. A new directory named hxlib is created and used to hold C lib source code of HelloX;
2. New header file stdbool.h is created to implement the standard C99's bool related operations;
3. Commented the __attribute__ descriptor of GCC,to fit VS IDE,as follows:
   void jerry_port_log(jerry_log_level_t level, const char *format, ...); /* __attribute__((format(printf, 2, 3)));*/
4. Omit all __attribute__ and __builtin_expect macros of GCC(in jrt.h file):
/**
 * Omit the default __attribute__ in GCC environment,since it can not be supported
 * in Microsoft Visual Studio IDE.
 */
#ifndef __GNUC__
#define __attribute__(x)
#endif

/**
 * Omit the builtin expectations which only applicable in GCC.
 */
#ifndef __GNUC__
#define __builtin_expect(x,y) (x)
#endif

5. Redefined inline and __func__ macros in config.h file:
/**
 * Satisfy Visual C++ IDE...
 */
#ifndef __cplusplus
#define inline  /* Skip the inline function since it may lead some link errors,don't know why...*/
#endif

#define __func__ __FUNCTION__
NOTE: Above definitions are moved into ide.h file of HelloX C Library.

6. Include config.h file in lit-strings.c;
7. Enable JERRY_JS_PARSER macro in config.h file:
/**
 * Enable JavaScript parser,which disables the snapshot function.
 */
#define JERRY_JS_PARSER

8. Define the following macros in jerry-port.h file,to replace the dynamic array feature:
	/**
	 * Curb dynamic array feature under C99,which can not be supported by
	 * Microsoft Visual Studio.
	 */
#define __CREATE_DYNAMIC_ARRAY(type,ptr,size) \
	type* ptr = (type*)malloc(size * sizeof(type)); \
	if(NULL == ptr) \
					{ \
		printf("Failed to allocate dynamic array[func:%s].\r\n",__func__); \
		exit(0); \
		}

#define __RELEASE_DYNAMIC_ARRAY(ptr) \
	if(NULL == ptr) \
				{ \
		printf("BUG: NULL pointer of dynamic array[func:%s].\r\n",__func__);\
		exit(0); \
			} \
			else \
			{ \
			free(ptr); \
			}

9. Replace dynamic array in vm.c;
10. Replace dynamic array in ecma-builtin-helpers-date.c;
11. Include stdlib.h in ecma-builtin-helpers-data.c file;
12. Set SDL(Security Development Lifecycle check) to NO to ignore C4146;
13. Implement the jerry-port.c file;
14. entry.c file is added to project,which contains the main routine;
15. Implements the following macros in config.h file to generate code and apply it in hellox-port.c file:
#define __DEFINE_DISPATCH_CONSTRUCT_ROUTINE(lowcase_id) \
	ecma_value_t \
	ecma_builtin_##lowcase_id##_dispatch_construct(const ecma_value_t *arguments_list_p, \
	ecma_length_t arguments_list_len) \
	{ \
	JERRY_ASSERT(arguments_list_len == 0 || arguments_list_p != NULL); \
	return ecma_raise_type_error(ECMA_ERR_MSG("'Function.prototype' is not a constructor.")); \
}

#define __DEFINE_DISPATCH_CALL_ROUTINE(lowcase_id) \
	ecma_value_t \
    ecma_builtin_##lowcase_id##_dispatch_call(const ecma_value_t *arguments_list_p, \
	ecma_length_t arguments_list_len) \
{ \
JERRY_ASSERT(arguments_list_len == 0 || arguments_list_p != NULL); \
return ecma_make_simple_value(ECMA_SIMPLE_VALUE_UNDEFINED); \
}

16. Comment off #include <float.h> sentence in jrt-types.h file,since it may no use
and will lead compiling error under HelloX C Library support;
17. Use __HXCL_DEFINE_ALIGNED_OBJECT macro to redefine the jerry_global_heap object in
jcontext.c file.And define it by speculating JMEM_ALIGNMENT_LOG's value;
18. Change the following macro's value to 128:
#define ECMA_MAX_CHARS_IN_STRINGIFIED_NUMBER 128
since it will lead assert_failure in case of show a number string if it's length exceed 64 bytes length;
19. 
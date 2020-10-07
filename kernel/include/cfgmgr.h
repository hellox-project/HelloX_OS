//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 06, 2020
//    Module Name               : cfgmgr.h
//    Module Funciton           : 
//                                System configuration manager is defined in
//                                this file. The system config manager is 
//                                responsible to manage all system configurations.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#ifndef __CFGMGR_H__
#define __CFGMGR_H__

/* 
 * All system configurations are saved into a json file,
 * we use cJSON to manipulate this file.
 */
#include <cJSON/cJSON.h>
#include <KAPI.H>

/* default config file name. */
#define SYSTEM_CONFIG_FILE_NAME "c:\\syscfg\\syscfg.jsn"

/* Maximal config file's size. */
#define SYSTEM_CONFIG_MAX_FILELENGTH (1024 * 1024 * 4)

/* Maximal key and value's length. */
#define SYSTEM_CONFIG_MAX_KVLENGTH 64
#if SYSTEM_CONFIG_MAX_KVLENGTH < MAX_THREAD_NAME
#error "SYSTEM_CONFIG_MAX_KVLENGTH must large than thread name"
#endif

/* System configuration manager. */
typedef struct tag__SYSTEM_CONFIG_MANAGER {
	/* The corresponding json object of config file. */
	cJSON* cfg_root;
	/* refer counter of the json object. */
	__atomic_t ref_count;
	/* Memory buffer to hold the json file. */
	char* json_file_buff;
	/* handle of json file. */
	HANDLE json_file;
	/* Mutex to protect this object. */
	HANDLE _mutex;

	/* Initialization routine of this object. */
	BOOL (*Initialize)(struct tag__SYSTEM_CONFIG_MANAGER* pCfgMgr);

	/* Operations this object offers. */
	/* 
	 * Get current thread(kernel/user)'s configure 
	 * profile. All thread's configure profiles are
	 * put into the syscfg file together,this routine
	 * returns the specified or current(threadname == NULL)
	 * thread's configuration profile.
	 * The return value is a handle,it's actually the
	 * corresponding json object's base address.
	 */
	HANDLE (*GetConfigProfile)(const char* threadname);
	/* Get lower level configuration file. */
	HANDLE (*GetSubConfigProfile)(HANDLE hParent, const char* name);
	/* Get configure entry from a profile. */
	BOOL (*GetConfigEntry)(HANDLE hCfgProfile, const char* key_name, 
		char* value_buff, size_t buff_sz);
	/* Add a new config profile for a thread. */
	HANDLE (*AddConfigProfile)(const char* threadname);
	/* Add a sub level config profile. */
	HANDLE (*AddSubConfigProfile)(HANDLE hParent, const char* name);
	/* Add config key/value pair under a profile. */
	BOOL (*AddConfigEntry)(HANDLE hCfgProfile, const char* key_name,
		const char* value);
	/* 
	 * Release the config profile opened by GetConfigProfile,
	 * GetSubConfigProfile, AddConfigProfile, AddSubConfigProfile
	 * routines. This routine decrease the refer counter and
	 * release all resources if refer counter reach 0.
	 */
	VOID (*ReleaseConfigProfile)(HANDLE hCfgProfile);
}__SYSTEM_CONFIG_MANAGER;

/* Global system configure manager object. */
extern __SYSTEM_CONFIG_MANAGER SystemConfigManager;

#endif //__CFGMGR_H__

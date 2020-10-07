//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Oct 06, 2020
//    Module Name               : cfgmgr.h
//    Module Funciton           : 
//                                System configuration manager is implemented in
//                                this file. The system config manager is 
//                                responsible to manage all system configurations.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include <StdAfx.h>
#include <cfgmgr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* 
 * A local helper routine to construct the system 
 * configuration file's name. It just returns the
 * pre-defined string currently.
 */
static char* __stdcall __GetSysCfgFileName()
{
	return SYSTEM_CONFIG_FILE_NAME;
}

/* Initializer of the system config manager. */
static BOOL __Initialize(__SYSTEM_CONFIG_MANAGER* pCfgMgr)
{
	BOOL bResult = FALSE;

	BUG_ON(NULL == pCfgMgr);

	/* Initialize all members of this object. */
	pCfgMgr->cfg_root = NULL;
	pCfgMgr->json_file = NULL;
	pCfgMgr->json_file_buff = NULL;
	pCfgMgr->ref_count = 0;
	
	pCfgMgr->_mutex = CreateMutex();
	if (NULL == pCfgMgr->_mutex)
	{
		goto __TERMINAL;
	}
	bResult = TRUE;

__TERMINAL:

	return bResult;
}

/* 
 * Phase 2 initialization compare to the initialize routine.
 * The config file are opened and read into memory in this
 * routine, the corresponding json object is generated.
 */
static BOOL __Phase2_Init()
{
	BOOL bResult = FALSE;
	__SYSTEM_CONFIG_MANAGER* pCfgMgr = &SystemConfigManager;
	char* cfg_file_name = __GetSysCfgFileName();
	HANDLE json_file = NULL;
	char* json_file_buff = NULL;
	cJSON* cfg_root = NULL;
	unsigned long file_sz = 0, read_sz = 0;

	/* Verify the object. */
	BUG_ON(NULL != pCfgMgr->json_file);
	BUG_ON(NULL != pCfgMgr->cfg_root);
	BUG_ON(NULL != pCfgMgr->json_file_buff);
	BUG_ON(NULL == pCfgMgr->_mutex); /* mutex must be created in initialize. */
	BUG_ON(NULL == cfg_file_name);

	/* Open the configure file. */
	json_file = CreateFile(cfg_file_name, FILE_ACCESS_READWRITE, 0, NULL);
	if (NULL == json_file)
	{
		__LOG("[%s]failed to open cfg file\r\n", __func__);
		goto __TERMINAL;
	}
	/* get config file's size. */
	file_sz = GetFileSize(json_file, NULL);
	if (0 == file_sz)
	{
		__LOG("[%s]invalid config file size.\r\n", __func__);
		goto __TERMINAL;
	}
	if (file_sz > SYSTEM_CONFIG_MAX_FILELENGTH)
	{
		__LOG("[%s]config file too large[%d]\r\n", __func__, file_sz);
		goto __TERMINAL;
	}

	/* Allocate the file's buffer and read the file into memory. */
	json_file_buff = (char*)_hx_malloc(file_sz + 1);
	if (NULL == json_file_buff)
	{
		__LOG("[%s]out of memory\r\n", __func__);
		goto __TERMINAL;
	}
	if (!ReadFile(json_file, file_sz, json_file_buff, &read_sz))
	{
		__LOG("[%s]could not load config file\r\n", __func__);
		goto __TERMINAL;
	}
	json_file_buff[file_sz] = '\0';

	/* Parse the config file. */
	cfg_root = cJSON_Parse(json_file_buff);
	if (NULL == cfg_root)
	{
		__LOG("[%s]failed to parse json file\r\n", __func__);
		goto __TERMINAL;
	}

	/* Everything is ok, save them into config manager. */
	pCfgMgr->cfg_root = cfg_root;
	pCfgMgr->json_file = json_file;
	pCfgMgr->json_file_buff = json_file_buff;

	bResult = TRUE;

__TERMINAL:
	if (!bResult)
	{
		/* release all allocated resources. */
		if (json_file)
		{
			CloseFile(json_file);
		}
		if (json_file_buff)
		{
			_hx_free(json_file_buff);
		}
		if (cfg_root)
		{
			cJSON_Delete(cfg_root);
		}
	}
	return bResult;
}

/*
 * Get current thread(kernel/user)'s configure
 * profile. All thread's configure profiles are
 * put into the syscfg file together,this routine
 * returns the specified or current(threadname == NULL)
 * thread's configuration profile.
 * The return value is a handle,it's actually the
 * corresponding json object's base address.
 */
static HANDLE __GetConfigProfile(const char* threadname)
{
	cJSON* cfg_profile = NULL;
	char thread_name[SYSTEM_CONFIG_MAX_KVLENGTH];

	/* config manager must be initialized. */
	BUG_ON(NULL == SystemConfigManager._mutex);

	/* 
	 * This routine could not be invoked in interrupt 
	 * or in system initialization phase, must be invoked
	 * in thread context. 
	 */
	BUG_ON(IN_INTERRUPT() || IN_SYSINITIALIZATION());
	BUG_ON(NULL == __CURRENT_KERNEL_THREAD);

	/* Construct the config profile's name. */
	if (threadname)
	{
		if (strlen(threadname) >= SYSTEM_CONFIG_MAX_KVLENGTH)
		{
			goto __TERMINAL;
		}
		strcpy(thread_name, (const char*)threadname);
	}
	else {
		/* Use current kernel thread's name as profile name. */
		strcpy(thread_name, __CURRENT_KERNEL_THREAD->KernelThreadName);
	}

	/* Get the config profile from json object. */
	WaitForThisObject(SystemConfigManager._mutex);
	/* Invoke the phase 2 initialization if not yet. */
	if (NULL == SystemConfigManager.cfg_root)
	{
		if (!__Phase2_Init())
		{
			ReleaseMutex(SystemConfigManager._mutex);
			__LOG("[%s]failed to init cfgmgr at phase2\r\n", __func__);
			goto __TERMINAL;
		}
	}
	cfg_profile = cJSON_GetObjectItemCaseSensitive(SystemConfigManager.cfg_root,
		(const char*)thread_name);
	if (NULL == cfg_profile)
	{
		ReleaseMutex(SystemConfigManager._mutex);
		__LOG("[%s]could not get config profile[%s]\r\n", __func__,
			thread_name);
		goto __TERMINAL;
	}
	/* Increment the refer counter. */
	__ATOMIC_INCREASE(&SystemConfigManager.ref_count);
	ReleaseMutex(SystemConfigManager._mutex);

__TERMINAL:
	return (HANDLE)cfg_profile;
}

/* Get lower level configuration file. */
static HANDLE __GetSubConfigProfile(HANDLE hParent, const char* name)
{
	UNIMPLEMENTED_ROUTINE_CALLED;
	return NULL;
}

/* Get configure entry from a profile. */
static BOOL __GetConfigEntry(HANDLE hCfgProfile, const char* key_name,
	char* value_buff, size_t buff_sz)
{
	BOOL bResult = FALSE;
	cJSON* cfg_kv = NULL;

	BUG_ON((NULL == hCfgProfile) || (NULL == key_name) || (NULL == value_buff));
	BUG_ON(NULL == SystemConfigManager._mutex);

	WaitForThisObject(SystemConfigManager._mutex);
	cfg_kv = cJSON_GetObjectItemCaseSensitive((cJSON*)hCfgProfile, key_name);
	if (NULL == cfg_kv)
	{
		ReleaseMutex(SystemConfigManager._mutex);
		__LOG("[%s]could not get value for key[%s]\r\n", __func__, key_name);
		goto __TERMINAL;
	}
	/* Make sure the value is string. */
	if (!cJSON_IsString((const cJSON*)cfg_kv))
	{
		ReleaseMutex(SystemConfigManager._mutex);
		__LOG("[%s]key is not string\r\n", __func__);
		goto __TERMINAL;
	}
	/* Get value ok, copy to caller's buffer. */
	strncpy(value_buff, cfg_kv->valuestring, buff_sz);
	ReleaseMutex(SystemConfigManager._mutex);

	bResult = TRUE;

__TERMINAL:
	return bResult;
}

/* Add a new config profile for a thread. */
static HANDLE __AddConfigProfile(const char* threadname)
{
	UNIMPLEMENTED_ROUTINE_CALLED;
	return NULL;
}

/* Add a sub level config profile. */
static HANDLE __AddSubConfigProfile(HANDLE hParent, const char* name)
{
	UNIMPLEMENTED_ROUTINE_CALLED;
	return NULL;
}

/* Add config key/value pair under a profile. */
static BOOL __AddConfigEntry(HANDLE hCfgProfile, const char* key_name,
	const char* value)
{
	UNIMPLEMENTED_ROUTINE_CALLED;
	return FALSE;
}

/* Local helper to release all system config manager's resource. */
static VOID __ReleaseCfgMgrResource()
{
	BUG_ON(NULL == SystemConfigManager.cfg_root);
	BUG_ON(NULL == SystemConfigManager.json_file);
	BUG_ON(NULL == SystemConfigManager.json_file_buff);

	cJSON_Delete(SystemConfigManager.cfg_root);
	SystemConfigManager.cfg_root = NULL;
	_hx_free(SystemConfigManager.json_file_buff);
	SystemConfigManager.json_file_buff = NULL;
	CloseFile(SystemConfigManager.json_file);
	SystemConfigManager.json_file = NULL;
}

/*
 * Release the config profile opened by GetConfigProfile,
 * GetSubConfigProfile, AddConfigProfile, AddSubConfigProfile
 * routines. This routine decrease the refer counter and
 * release all resources if refer counter reach 0.
 */
static VOID __ReleaseConfigProfile(HANDLE hCfgProfile)
{
	BUG_ON(NULL == hCfgProfile);
	BUG_ON(NULL == SystemConfigManager._mutex);

	WaitForThisObject(SystemConfigManager._mutex);
	__ATOMIC_DECREASE(&SystemConfigManager.ref_count);
	if (0 == SystemConfigManager.ref_count)
	{
		/* All resource should be released. */
		__ReleaseCfgMgrResource();
	}
	ReleaseMutex(SystemConfigManager._mutex);
	return;
}

/* Global system configure manager object. */
__SYSTEM_CONFIG_MANAGER SystemConfigManager = {
	NULL,               //cfg_root;
	0,                  //ref_count;
	NULL,               //json_file_buff;
	NULL,               //json_file;
	NULL,               //_mutex;

	__Initialize,       //Initialize;

	__GetConfigProfile,       //GetConfigProfile;
	__GetSubConfigProfile,    //GetSubConfigProfile;
	__GetConfigEntry,         //GetConfigEntry;
	__AddConfigProfile,       //AddConfigProfile;
	__AddSubConfigProfile,    //AddSubConfigProfile;
	__AddConfigEntry,         //AddConfigEntry;
	__ReleaseConfigProfile,   //ReleaseConfigProfile;
};

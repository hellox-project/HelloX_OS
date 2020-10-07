//***********************************************************************/
//    Author                    : Garry
//    Original Date             : Jun 6,2019
//    Module Name               : examapp.c
//    Module Funciton           : 
//                                An example application for HelloX OS,it
//                                runs in user mode and as a process.
//    Last modified Author      :
//    Last modified Date        :
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              :
//***********************************************************************/

#include "hellox.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "malloc.h"

/* socket headers. */
#include "socket/genif.h"
#include "socket/dhcp_srv.h"

/* A local helper routine to show out system information. */
static void ShowSysInfo(SYSTEM_INFO* pSysInfo)
{
	_hx_printf("System information as follows:\r\n");
	_hx_printf("  Page size: %d\r\n", pSysInfo->dwPageSize);
	_hx_printf("  Minimum app addr: 0x%0X\r\n", (unsigned long)pSysInfo->lpMinimumApplicationAddress);
	_hx_printf("  Maximum app addr: 0x%0X\r\n", (unsigned long)pSysInfo->lpMaximumApplicationAddress);
	_hx_printf("  Number of processors: %d\r\n", pSysInfo->dwNumberOfProcessors);
	_hx_printf("  Allocation granularity: %d\r\n", pSysInfo->dwAllocationGranularity);
}

/* Show memory basic information. */
static void ShowMemoryBasicInfo(MEMORY_BASIC_INFORMATION* pMemInfo)
{
	_hx_printf("base_addr:0x%X,size:0x%X,state:%d\r\n",
		pMemInfo->BaseAddress,
		pMemInfo->RegionSize,
		pMemInfo->State);
}

/* Maximal memory block that malloc can give. */
#define MAX_MALLOC_BLOCK (4096 * 1024)
#define MALLOC_TEST_ROUND 128

/* Conduct performance test of ptmalloc. */
static unsigned long DoPerfTest(LPVOID pData)
{
	void* pMemBlocks[MALLOC_TEST_ROUND];
	HANDLE hLogFile = NULL;
	unsigned long thread_id = GetCurrentThreadID();

	/* Create log file to save the testing log. */

	/* Start intensive allocation test. */
	int i = 0, j = 0;
	long block_sz = 0;
	char* pBlockAddr = NULL;
	srand(thread_id);

	for (i = 0; i < MALLOC_TEST_ROUND; i++)
	{
		block_sz = rand();
		block_sz %= MAX_MALLOC_BLOCK;
		pMemBlocks[i] = dlmalloc(block_sz);
		if (NULL == pMemBlocks[i])
		{
			_hx_printf("[thread:%d]Failed to allocate at round[%d].\r\n", thread_id, i);
			break;
		}
		pBlockAddr = pMemBlocks[i];
		for (j = 0; j < block_sz; j++)
		{
			pBlockAddr[j] = 'X';
		}
		_hx_printf("[thread:%d]Intensive test[rnd:%d,blk_sz:%d,foot_prnt:%d\r\n",
			thread_id, i, block_sz, dlmalloc_footprint());
	}
	for (i = 0; i < MALLOC_TEST_ROUND; i++)
	{
		if (pMemBlocks[i])
		{
			dlfree(pMemBlocks[i]);
			pMemBlocks[i] = NULL;
		}
	}
	_hx_printf("[thread:%d]Foot print after intensive test:%d\r\n", thread_id, 
		dlmalloc_footprint());
	dlmalloc_trim(0);
	_hx_printf("[thread:%d]Foot print after trim:%d\r\n", thread_id, dlmalloc_footprint());

	/* Return to kernel. */
	ExitThread(0);

	return 0;
}

/* A helper routine to construct network addresses. */
static void build_addr(__COMMON_NETWORK_ADDRESS* comm_addr, unsigned char index)
{
	unsigned char part1 = 1;
	unsigned char part2 = index;
	unsigned char part3 = 168;
	unsigned char part4 = 192;
	__u32 ip_addr = 0;

	/* Set address and gateway part. */
	ip_addr = part1;
	ip_addr <<= 8;
	ip_addr += part2;
	ip_addr <<= 8;
	ip_addr += part3;
	ip_addr <<= 8;
	ip_addr += part4;
	comm_addr[0].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
	comm_addr[0].Address.ipv4_addr = ip_addr;
	comm_addr[2].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
	comm_addr[2].Address.ipv4_addr = ip_addr;

	/* Set mask part. */
	ip_addr = 0;
	ip_addr <<= 8;
	ip_addr += 255;
	ip_addr <<= 8;
	ip_addr += 255;
	ip_addr <<= 8;
	ip_addr += 255;
	comm_addr[1].AddressType = NETWORK_ADDRESS_TYPE_IPV4;
	comm_addr[1].Address.ipv4_addr = ip_addr;
}

/* Test the ptmalloc lib. */
int _hxmain_old(int argc, char* argv[])
{
	__GENERIC_NETIF* pGenif = NULL;
	unsigned long buf_req = 0;
	//__COMMON_NETWORK_ADDRESS comm_addr[3];

	/* Allocate a big enough memory. */
	pGenif = (__GENERIC_NETIF*)_hx_malloc(8 * sizeof(__GENERIC_NETIF));
	if (NULL == pGenif)
	{
		_hx_printf("Out of memory.\r\n");
		return -1;
	}

	/* Query all genifs in system. */
	memset(pGenif, 0, sizeof(__GENERIC_NETIF) * 8);
	buf_req = 8 * sizeof(__GENERIC_NETIF);
	int ret_val = GetGenifInfo(pGenif, &buf_req, 0);
	if (ret_val < 0)
	{
		_hx_printf("Get genif info failed\r\n");
		return -1;
	}

	/* Show out all genifs. */
	for (int i = 0; i < ret_val; i++)
	{
		_hx_printf("  -----------------------\r\n");
		_hx_printf("  genif index: %d\r\n", pGenif[i].if_index);
		_hx_printf("  genif name: %s\r\n", pGenif[i].genif_name);
		_hx_printf("  genif mtu: %d\r\n", pGenif[i].genif_mtu);

		/* Enable DHCP service on this if. */
		DHCPSrv_Start_Onif(pGenif[i].genif_name);

#if 0
		/* Then set an ip addr on this netif. */
		build_addr(&comm_addr[0], (unsigned char)pGenif[i].if_index);
		if (AddGenifAddress(pGenif[i].if_index,
			NETWORK_PROTOCOL_TYPE_IPV4,
			comm_addr, 3, FALSE))
		{
			_hx_printf("  set address to genif failed.\r\n");
		}
		else
		{
			_hx_printf("  set address OK.\r\n");
		}
#endif
	}

	/* recall all memory and return. */
	_hx_free(pGenif);
	return 0;
}

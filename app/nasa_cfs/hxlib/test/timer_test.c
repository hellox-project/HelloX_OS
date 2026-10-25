/* Test code for timer function. */

#include <hellox.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>

/* Timer IDs. */
#define TIMER_ID_5MS 1025
#define TIMER_ID_10MS 1026
#define TIMER_ID_20MS 1027

/* Timer base thread. */
static void* TimerBase(void* ptr)
{
	HANDLE hTimer5ms = NULL;
	HANDLE hTimer10ms = NULL;
	HANDLE hTimer20ms = NULL;
	MSG msg;
	unsigned long timer_5ms = 0, timer_10ms = 0, timer_20ms = 0;
	unsigned long msg_num = 0;
	CPU_INFO cpuInfo;

	/* Set several timers. */
	hTimer5ms = SetTimer(TIMER_ID_5MS, 5, TIMER_FLAGS_ALWAYS);
	if (NULL == hTimer5ms)
	{
		printf("[%s]set timer [5ms] failed\r\n", __func__);
		goto __TERMINAL;
	}
	hTimer10ms = SetTimer(TIMER_ID_10MS, 10, TIMER_FLAGS_ALWAYS);
	if (NULL == hTimer10ms)
	{
		printf("[%s]set timer [10ms] failed\r\n", __func__);
		goto __TERMINAL;
	}
	hTimer20ms = SetTimer(TIMER_ID_20MS, 20, TIMER_FLAGS_ALWAYS);
	if (NULL == hTimer20ms)
	{
		printf("[%s]set timer [20ms] failed\r\n", __func__);
		goto __TERMINAL;
	}

	/* Show out CPU information. */
	cpuInfo.cbSize = sizeof(CPU_INFO);
	if (!GetCPUInfo(&cpuInfo))
	{
		printf("[%s]failed to get CPU info.\r\n", __func__);
		goto __TERMINAL;
	}
	printf("[%s]cpu freq:%lld, cpu tsc:%lld\r\n", __func__,
		cpuInfo.cpuFreq,
		cpuInfo.cpuTSC);

	/* Main message loop. */
	while (TRUE)
	{
		if (GetMessage(&msg))
		{
			switch (msg.command)
			{
			case KERNEL_MESSAGE_TIMER:
				msg_num++;
				if (msg.lParam == TIMER_ID_5MS)
				{
					timer_5ms++;
				}
				if (msg.lParam == TIMER_ID_10MS)
				{
					timer_10ms++;
				}
				if (msg.lParam == TIMER_ID_20MS)
				{
					timer_20ms++;
				}
				/* Show message every 2s. */
				if (msg_num % 500 == 0)
				{
					printf("[%s]timer_5ms:%d, timer_10ms:%d, timer_20ms:%d\r\n",
						__func__, timer_5ms, timer_10ms, timer_20ms);
				}
				/* Run 20s. */
				if (msg_num % 1500 == 0)
				{
					goto __TERMINAL;
				}
				break;
			default:
				printf("[%s]Unknown message type[%d]\r\n", __func__,
					msg.command);
				break;
			}
		}
	}

__TERMINAL:
	/* Cancel timers if created. */
	if (hTimer5ms)
	{
		CancelTimer(hTimer5ms);
	}
	if (hTimer10ms)
	{
		CancelTimer(hTimer10ms);
	}
	if (hTimer20ms)
	{
		CancelTimer(hTimer20ms);
	}

	/* Show out CPU information again. */
	cpuInfo.cbSize = sizeof(CPU_INFO);
	if (!GetCPUInfo(&cpuInfo))
	{
		printf("[%s]failed to get CPU info again.\r\n", __func__);
		return NULL;
	}
	printf("[%s]cpu freq:%lld, cpu tsc:%lld\r\n", __func__,
		cpuInfo.cpuFreq,
		cpuInfo.cpuTSC);

	return NULL;
}

/* main routine. */
int timer_main()
{
	pthread_t timer_base = NULL;

	/* Create timer base thread. */
	pthread_create(&timer_base, NULL, TimerBase, NULL);
	pthread_join(timer_base, NULL);
	printf("Main exit.\r\n");
	return 0;
}

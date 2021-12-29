/*
 * MxOS Kernel V0.1
 * Copyright (C) 2020 StephenZhou.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * 1 tab == 4 spaces!
 */

#include "os_time.h"
#include "os_task.h"
#include "os_types.h"
#include "os_shell.h"
#include "os_trace.h"
#include "os_configs.h"
#include "os_critical.h"
#include "os_sw_timer.h"
#include "os_scheduler.h"
#include "os_error_code.h"

#if CONFIG_USE_SHELL
static OS_Uint32_t OS_ShellTaskHandle = 0;

extern void PlatformUartSendDataPolling(const char ch);
extern signed char PlatformUartRecvDataPolling(char *ch);

OS_Shell_t OS_Shell;

/* Comes from letter shell */
SHELL_TypeDef ShellParam;

void OS_ShellRegister(TaskFunction_t TaskEntry,
                      ShellInit_t ShellInit,
                      void *Param)
{
    OS_Shell.ShellInit = ShellInit;
    OS_Shell.TaskEntry = TaskEntry;
    OS_Shell.Param = Param;
}

static OS_Shell_t *OS_GetCurrentShell(void)
{
    return &OS_Shell;
}

void OS_ShellInit(void)
{
    OS_Shell_t *Shell = OS_GetCurrentShell();

    ShellParam.read = PlatformUartRecvDataPolling;
    ShellParam.write = PlatformUartSendDataPolling;

    OS_ShellRegister(shellTask, shellInit, (void *)&ShellParam);

    Shell->ShellInit((SHELL_TypeDef *)Shell->Param);
}

void OS_ShellTaskCreate(void)
{
    TaskInitParameter Param;
    OS_Shell_t *Shell = OS_GetCurrentShell();

    Param.Name[0] = 'S';
    Param.Name[1] = 'H';
    Param.Name[2] = 'E';
    Param.Name[3] = 'L';
    Param.Name[4] = 'L';
    Param.Name[5] = 0x00;
    Param.Priority = CONFIG_SHELL_TASK_PRIO;
    Param.PrivateData = Shell->Param;
    Param.StackSize = CONFIG_SW_TMR_TASK_STACK_SIZE;
    Param.TaskEntry = Shell->TaskEntry;

    OS_API_TaskCreate(Param, (void *)&OS_ShellTaskHandle);
}

#endif // CONFIG_USE_SHELL


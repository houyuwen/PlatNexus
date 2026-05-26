/**
 *****************************************************************************
 * @file   plat_thread_posix.c
 * @author houyuwenE@outlook.com
 * @brief  实现细节： 内部使用 POSIX 原生接口进行封装。
 * @version 0.1
 * @date 2026-05-24
 *****************************************************************************
 * @attention
 *
 * Copyright (c) 2026 houyuwen.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *****************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "plat_thread.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
plat_thread_t plat_thread_create(const char *name, 
                                 plat_thread_entry_t entry, 
                                 void *arg, 
                                 size_t stack_bytes, 
                                 plat_thread_prio_t prio) 
{
    pthread_t *thread_id = (pthread_t*)malloc(sizeof(pthread_t));
    if (!thread_id) return NULL;

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // 1. 设置栈大小（标准 POSIX 接收字节）
    if (stack_bytes > 0) {
        pthread_attr_setstacksize(&attr, stack_bytes);
    }

    // 注：由于普通 posix 环境下设置 SCHED_FIFO 需要 Root 权限，
    // 在纯仿真测试环境下，通常忽略优先级映射，或通过 nice 值模拟。
    // 这里为了不越权报错，我们保留默认的 SCHED_OTHER。

    // 2. 创建 POSIX 线程
    int ret = pthread_create(thread_id, &attr, (void *(*)(void *))entry, arg);
    pthread_attr_destroy(&attr);

    if (ret != 0) {
        free(thread_id);
        return NULL;
    }

    return (plat_thread_t)thread_id;
}

void plat_thread_destroy(plat_thread_t thread) 
{
    if (thread != NULL) {
        pthread_cancel(*(pthread_t*)thread);
        free(thread);
    } else {
        pthread_exit(NULL);
    }
}

/* End of file ---------------------------------------------------------------*/

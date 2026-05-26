/**
 *****************************************************************************
 * @file   plat_thread.h
 * @author houyuwenE@outlook.com
 * @brief  设计思想： 严格隔离底层依赖，向业务层提供不透明指针，统一定义枚举化的优先
 *         级和字节维度的栈大小。
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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>

/* Private define ------------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private types -------------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/* 不透明指针，隐藏底层的 pthread_t 或 TaskHandle_t */
typedef void* plat_thread_t;

/* 统一优先级枚举，屏蔽平台具体的数值差异 */
typedef enum {
    PLAT_PRIO_LOW = 0,
    PLAT_PRIO_NORMAL,
    PLAT_PRIO_HIGH,
    PLAT_PRIO_REALTIME
} plat_thread_prio_t;

/* 统一的线程入口函数签名 */
typedef void (*plat_thread_entry_t)(void *arg);

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @brief 创建并启动一个线程/任务
 */
plat_thread_t plat_thread_create(const char *name, 
                                 plat_thread_entry_t entry, 
                                 void *arg, 
                                 size_t stack_bytes, 
                                 plat_thread_prio_t prio);

/**
 * @brief 删除当前线程或指定线程
 */
void plat_thread_destroy(plat_thread_t thread);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/* End of file ---------------------------------------------------------------*/

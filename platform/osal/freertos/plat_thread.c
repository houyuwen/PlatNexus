/**
 *****************************************************************************
 * @file   plat_thread_freertos.c
 * @author houyuwenE@outlook.com
 * @brief  实现细节： 将栈大小从字节转换为字长，将统一定义的优先级映射为 FreeRTOS 
 *         具体的数值。
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
#include "FreeRTOS.h"
#include "task.h"

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
    TaskHandle_t handle = NULL;
    UBaseType_t rtos_prio;

    // 1. 优先级映射：FreeRTOS 数值越大优先级越高
    switch (prio) {
        case PLAT_PRIO_LOW:      rtos_prio = tskIDLE_PRIORITY + 1; break;
        case PLAT_PRIO_NORMAL:   rtos_prio = tskIDLE_PRIORITY + 2; break;
        case PLAT_PRIO_HIGH:     rtos_prio = configMAX_PRIORITIES - 2; break;
        case PLAT_PRIO_REALTIME: rtos_prio = configMAX_PRIORITIES - 1; break;
        default:                 rtos_prio = tskIDLE_PRIORITY + 2; break;
    }

    // 2. 栈大小转换：FreeRTOS 要求以“字长(Word)”为单位，这里统一由字节进行转换
    uint16_t stack_words = (uint16_t)(stack_bytes / sizeof(StackType_t));

    // 3. 创建任务
    BaseType_t ret = xTaskCreate((TaskFunction_t)entry,
                                 name,
                                 stack_words,
                                 arg,
                                 rtos_prio,
                                 &handle);

    return (ret == pdPASS) ? (plat_thread_t)handle : NULL;
}

void plat_thread_destroy(plat_thread_t thread) 
{
    vTaskDelete((TaskHandle_t)thread);
}

/* End of file ---------------------------------------------------------------*/

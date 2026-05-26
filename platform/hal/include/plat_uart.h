/**
 *****************************************************************************
 * @file   plat_uart.h
 * @author houyuwenE@outlook.com
 * @brief  设计思想： 拒绝裸露 ioctl。强制使用含有 used 的强类型异步回调。
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
typedef void* plat_uart_t;

/* 异步接收中断/事件回调签名，强制带有业务上下文 used */
typedef void (*plat_uart_rx_cb_t)(uint8_t *data, size_t len, void *used);

/* 强类型的硬件配置结构体 */
typedef struct {
    uint32_t         baudrate;
    uint8_t          databits; // 8 或 9
    uint8_t          stopbits; // 1 或 2
    uint8_t          parity;   // 0: None, 1: Odd, 2: Even
    plat_uart_rx_cb_t rx_cb;    // 异步接收回调
    void             *used; // 业务上下文透传
} plat_uart_cfg_t;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
/**
 * @brief 通过名称打开并配置串口硬件
 */
plat_uart_t plat_uart_open(const char *name, const plat_uart_cfg_t *cfg);

/**
 * @brief 发送数据（阻塞/同步）
 */
int32_t plat_uart_write(plat_uart_t handle, const uint8_t *data, size_t len);

/**
 * @brief 关闭串口
 */
void plat_uart_close(plat_uart_t handle);


#ifdef __cplusplus
}
#endif /* __cplusplus */

/* End of file ---------------------------------------------------------------*/

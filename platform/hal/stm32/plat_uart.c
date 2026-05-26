/**
 *****************************************************************************
 * @file   plat_uart_stm32.c
 * @author houyuwenE@outlook.com
 * @brief  实现细节： 将字符串名称映射为底层的 USART_TypeDef*。在物理中断（ISR）中
 *         转交数据，完美触发带上下文的回调。
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
#include "plat_uart.h"
#include "stm32f4xx_hal.h" // 底层芯片库，业务层完全看不见
#include <string.h>
#include <stdlib.h>

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
// 内部封装句柄对象
typedef struct {
    UART_HandleTypeDef hw_handle;
    plat_uart_rx_cb_t   rx_cb;
    void               *used;
    uint8_t            rx_buffer; // 单字节接收缓存
} stm32_uart_ctx_t;

/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
// 硬件资源映射表 (Device Registry)
static struct {
    const char *name;
    USART_TypeDef *hw_instance;
} g_uart_map[] = {
    {"gps_uart", USART1},
    {"modem_uart", USART2}
};

// 全局上下文数组（用于中断路由）
static stm32_uart_ctx_t *g_uart_contexts[3] = {NULL}; // 假设最多3个UART

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
plat_uart_t plat_uart_open(const char *name, const plat_uart_cfg_t *cfg) 
{
    USART_TypeDef *instance = NULL;
    int hw_idx = -1;

    // 1. 查找名称映射
    for (int i = 0; i < sizeof(g_uart_map)/sizeof(g_uart_map[0]); i++) {
        if (strcmp(name, g_uart_map[i].name) == 0) {
            instance = g_uart_map[i].hw_instance;
            hw_idx = i;
            break;
        }
    }
    if (!instance) return NULL;

    // 2. 分配并初始化自定义上下文对象
    stm32_uart_ctx_t *ctx = (stm32_uart_ctx_t *)malloc(sizeof(stm32_uart_ctx_t));
    ctx->rx_cb = cfg->rx_cb;
    ctx->used = cfg->used;
    
    // 3. 配置 STM32 原生 HAL 库结构
    ctx->hw_handle.Instance = instance;
    ctx->hw_handle.Init.BaudRate = cfg->baudrate;
    ctx->hw_handle.Init.WordLength = (cfg->databits == 9) ? UART_WORDLENGTH_9B : UART_WORDLENGTH_8B;
    ctx->hw_handle.Init.StopBits = (cfg->stopbits == 2) ? UART_STOPBITS_2 : UART_STOPBITS_1;
    ctx->hw_handle.Init.Parity = (cfg->parity == 1) ? UART_PARITY_ODD : 
                                 (cfg->parity == 2) ? UART_PARITY_EVEN : UART_PARITY_NONE;
    ctx->hw_handle.Init.Mode = UART_MODE_TX_RX;
    ctx->hw_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;

    HAL_UART_Init(&ctx->hw_handle);

    // 4. 保存到全局，开启中断接收
    g_uart_contexts[hw_idx] = ctx;
    HAL_UART_Receive_IT(&ctx->hw_handle, &ctx->rx_buffer, 1);

    return (plat_uart_t)ctx;
}

int32_t plat_uart_write(plat_uart_t handle, const uint8_t *data, size_t len) 
{
    stm32_uart_ctx_t *ctx = (stm32_uart_ctx_t *)handle;
    if (HAL_UART_Transmit(&ctx->hw_handle, (uint8_t*)data, len, 1000) != HAL_OK) {
        return -1;
    }
    return len;
}

// 物理中断处理函数 (ISR)
void USART1_IRQHandler(void) 
{
    stm32_uart_ctx_t *ctx = g_uart_contexts[0];
    HAL_UART_IRQHandler(&ctx->hw_handle); // 调用厂商底层 IRQ 处理

    // 回调通知业务层（严格的 ISR 上下文环境）
    if (ctx->rx_cb) {
        ctx->rx_cb(&ctx->rx_buffer, 1, ctx->used);
    }
    // 重新开启下一次接收
    HAL_UART_Receive_IT(&ctx->hw_handle, &ctx->rx_buffer, 1);
}

/* End of file ---------------------------------------------------------------*/

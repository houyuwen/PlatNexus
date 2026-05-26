/**
 *****************************************************************************
 * @file   plat_uart_posix.c
 * @author houyuwenE@outlook.com
 * @brief  实现细节： 将字符串映射到 /dev/ttyS*。为了模拟 RTOS 的异步接收硬件中断，
 *         我们在底层偷偷开启了一个 POSIX 后台监控线程，一旦 select 检测到数据可读，
 *         便触发回调。
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
// posix 平台专有的上下文控制块
typedef struct {
    int              fd;          // posix 文件描述符
    plat_uart_rx_cb_t rx_cb;
    void             *user_data;
    pthread_t        monitor_thread; // 用于模拟硬件中断的后台线程
    int              is_running;
} posix_uart_ctx_t;

/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
// 设备映射表
static struct {
    const char *name;
    const char *dev_path;
} g_uart_map[] = {
    {"gps_uart", "/dev/ttyS1"},
    {"modem_uart", "/dev/ttyS2"}
};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
// 偷偷运行的“中断伪造”线程
static void* posix_uart_monitor_thread(void *arg) 
{
    posix_uart_ctx_t *ctx = (posix_uart_ctx_t *)arg;
    uint8_t buffer[64];

    while (ctx->is_running) {
        // 使用阻塞 read 模拟硬件中断的到来
        int bytes = read(ctx->fd, buffer, sizeof(buffer));
        if (bytes > 0 && ctx->rx_cb != NULL) {
            // 触发回调！业务层会以为这是一个真实的中断
            ctx->rx_cb(buffer, bytes, ctx->user_data);
        }
        usleep(1000); 
    }
    return NULL;
}

plat_uart_t plat_uart_open(const char *name, const plat_uart_cfg_t *cfg) 
{
    const char *path = NULL;

    for (int i = 0; i < sizeof(g_uart_map)/sizeof(g_uart_map[0]); i++) {
        if (strcmp(name, g_uart_map[i].name) == 0) {
            path = g_uart_map[i].dev_path;
            break;
        }
    }
    if (!path) return NULL;

    int fd = open(path, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) return NULL;

    // 配置 termios (根据 cfg 设置波特率、数据位、停止位、校验位等)
    struct termios tty;
    tcgetattr(fd, &tty);

    // 波特率映射
    speed_t baud;
    switch (cfg->baudrate) {
        case 50: baud = B50; break;
        case 75: baud = B75; break;
        case 110: baud = B110; break;
        case 134: baud = B134; break;
        case 150: baud = B150; break;
        case 200: baud = B200; break;
        case 300: baud = B300; break;
        case 600: baud = B600; break;
        case 1200: baud = B1200; break;
        case 1800: baud = B1800; break;
        case 2400: baud = B2400; break;
        case 4800: baud = B4800; break;
        case 9600: baud = B9600; break;
        case 19200: baud = B19200; break;
        case 38400: baud = B38400; break;
        case 57600: baud = B57600; break;
        case 115200: baud = B115200; break;
        case 230400: baud = B230400; break;
        case 460800: baud = B460800; break;
        case 500000: baud = B500000; break;
        case 576000: baud = B576000; break;
        case 921600: baud = B921600; break;
        case 1000000: baud = B1000000; break;
        case 1152000: baud = B1152000; break;
        case 1500000: baud = B1500000; break;
        case 2000000: baud = B2000000; break;
        case 2500000: baud = B2500000; break;
        case 3000000: baud = B3000000; break;
        case 3500000: baud = B3500000; break;
        case 4000000: baud = B4000000; break;
        default: baud = B115200; break; // 默认 115200
    }
    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);

    // 数据位
    tty.c_cflag &= ~CSIZE;
    switch (cfg->databits) {
        case 5: tty.c_cflag |= CS5; break;
        case 6: tty.c_cflag |= CS6; break;
        case 7: tty.c_cflag |= CS7; break;
        default: tty.c_cflag |= CS8; break;
    }

    // 停止位
    if (cfg->stopbits == 2) {
        tty.c_cflag |= CSTOPB;
    } else {
        tty.c_cflag &= ~CSTOPB;
    }

    // 校验位
    if (cfg->parity == 1) { // Even
        tty.c_cflag |= PARENB;
        tty.c_cflag &= ~PARODD;
    } else if (cfg->parity == 2) { // Odd
        tty.c_cflag |= PARENB;
        tty.c_cflag |= PARODD;
    } else { // None
        tty.c_cflag &= ~PARENB;
    }

    // 关闭软件流控
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    // 设置为原始模式
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_oflag &= ~OPOST;

    // 使能接收，忽略调制解调器状态线
    tty.c_cflag |= (CLOCAL | CREAD);

    // 设置最小接收字符和超时
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    tcsetattr(fd, TCSANOW, &tty);

    posix_uart_ctx_t *ctx = (posix_uart_ctx_t *)malloc(sizeof(posix_uart_ctx_t));
    ctx->fd = fd;
    ctx->rx_cb = cfg->rx_cb;
    ctx->user_data = cfg->used;
    ctx->is_running = 1;

    // 若配置了接收回调，则创建一个隐蔽的后台线程模拟中断发生
    if (cfg->rx_cb) {
        pthread_create(&ctx->monitor_thread, NULL, posix_uart_monitor_thread, ctx);
    }

    return (plat_uart_t)ctx;
}

int32_t plat_uart_write(plat_uart_t handle, const uint8_t *data, size_t len) 
{
    posix_uart_ctx_t *ctx = (posix_uart_ctx_t *)handle;
    return write(ctx->fd, data, len);
}

void plat_uart_close(plat_uart_t handle) 
{
    posix_uart_ctx_t *ctx = (posix_uart_ctx_t *)handle;
    if (!ctx) return;
    
    ctx->is_running = 0;
    if (ctx->rx_cb) {
        pthread_join(ctx->monitor_thread, NULL);
    }
    close(ctx->fd);
    free(ctx);
}

/* Exported functions --------------------------------------------------------*/

/* End of file ---------------------------------------------------------------*/

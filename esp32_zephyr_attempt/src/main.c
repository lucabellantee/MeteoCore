#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <zephyr/data/json.h>
#include <zephyr/devicetree.h>
#include <zephyr/net/http/client.h>
#include <zephyr/net/net_event.h>
#include "secrets.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

#define UART_NODE DT_NODELABEL(uart0)
#define BUFFER_SIZE 512

static const struct device *uart_dev;
static char uart_buffer[BUFFER_SIZE];
static int uart_pos = 0;

static void uart_cb(const struct device *dev, void *user_data) {
    uint8_t c;

    if (!uart_irq_update(dev) || !uart_irq_rx_ready(dev)) {
        return;
    }

    while (uart_fifo_read(dev, &c, 1)) {
        if (c == '\n') {
            uart_buffer[uart_pos] = '\0';
            uart_pos = 0;

            LOG_INF("Received line: %s", log_strdup(uart_buffer));
            // TODO: Parse JSON and send via HTTP
        } else if (uart_pos < BUFFER_SIZE - 1) {
            uart_buffer[uart_pos++] = c;
        }
    }
}

static void connect_to_wifi(void) {
    struct wifi_connect_req_params params = {
        .ssid = WIFI_SSID,
        .ssid_length = strlen(WIFI_SSID),
        .psk = WIFI_PASSWORD,
        .psk_length = strlen(WIFI_PASSWORD),
        .channel = WIFI_CHANNEL_ANY,
        .security = WIFI_SECURITY_TYPE_PSK,
    };

    struct net_if *iface = net_if_get_default();

    int ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params));
    if (ret == 0) {
        LOG_INF("Wi-Fi connection initiated.");
    } else {
        LOG_ERR("Wi-Fi connection failed: %d", ret);
    }
}

void main(void) {
    LOG_INF("ESP32 Zephyr Weather Receiver");

    uart_dev = DEVICE_DT_GET(UART_NODE);
    if (!device_is_ready(uart_dev)) {
        LOG_ERR("UART device not ready");
        return;
    }

    uart_irq_callback_user_data_set(uart_dev, uart_cb, NULL);
    uart_irq_rx_enable(uart_dev);

    connect_to_wifi();

    while (1) {
        k_sleep(K_SECONDS(5));
    }
}

/**
 * ESP32 Weather Station - Ricevitore dati da STM32 e invio al database
 * Versione Zephyr RTOS
 */

 #include <zephyr/kernel.h>
 #include <zephyr/device.h>
 #include <zephyr/drivers/uart.h>
 #include <zephyr/net/net_if.h>
 #include <zephyr/net/wifi_mgmt.h>
 #include <zephyr/net/socket.h>
 #include <zephyr/logging/log.h>
 #include <zephyr/data/json.h>
 #include <string.h>
 #include <stdlib.h>
 #include <errno.h>  // Aggiunto per la variabile errno
 
 LOG_MODULE_REGISTER(esp32_weather, LOG_LEVEL_INF);
 
 /* Configurazione WiFi */
 #define WIFI_SSID "luca"
 #define WIFI_PSK "luca2303"
 
 /* Configurazione server */
 #define SERVER_URL "192.168.116.5"
 #define SERVER_PORT 3000
 #define API_ENDPOINT "/api/data"
 
 /* Configurazione UART */
 #define UART_DEVICE_NODE DT_NODELABEL(uart0)  // Modificato da uart1 a uart0
 
 /* Dimensioni del buffer */
 #define RECV_BUFFER_SIZE 256
 #define JSON_BUFFER_SIZE 512
 
 /* Struttura dati sensore */
 struct sensor_data {
     float temperature;
     float pressure;
     float humidity;
     float rain_probability;
 };
 
 /* Thread stacks */
 #define WIFI_STACK_SIZE 2048
 #define UART_STACK_SIZE 2048
 #define HTTP_STACK_SIZE 2048
 
 K_THREAD_STACK_DEFINE(wifi_stack, WIFI_STACK_SIZE);
 K_THREAD_STACK_DEFINE(uart_stack, UART_STACK_SIZE);
 K_THREAD_STACK_DEFINE(http_stack, HTTP_STACK_SIZE);
 
 /* Thread data */
 struct k_thread wifi_thread_data;
 struct k_thread uart_thread_data;
 struct k_thread http_thread_data;
 
 /* FIFO per comunicazione tra thread */
 K_FIFO_DEFINE(sensor_data_fifo);
 
 /* Struttura per elemento FIFO */
 struct sensor_data_item {
     void *fifo_reserved;
     struct sensor_data data;
 };
 
 /* Variabili globali */
 static const struct device *uart_dev;
 static struct net_if *iface;  // Variabile non utilizzata ma mantenuta
 static bool wifi_connected = false;
 static char rx_buf[RECV_BUFFER_SIZE];
 static int rx_buf_pos = 0;
 
 /* Prototipi delle funzioni */
 static void wifi_connect_thread(void *p1, void *p2, void *p3);
 static void uart_rx_thread(void *p1, void *p2, void *p3);
 static void http_client_thread(void *p1, void *p2, void *p3);
 static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                   uint32_t mgmt_event,
                                   struct net_if *iface);
 static void uart_rx_handler(const struct device *dev, void *user_data);
 static void parse_json_data(char *json_string, struct sensor_data *data);
 static void send_to_database(struct sensor_data *data);
 
 /* Callback per eventi WiFi */
 static struct net_mgmt_event_callback wifi_cb;
 
 void main(void)
 {
     int ret;
 
     /* Inizializza UART */
     uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);
     if (!device_is_ready(uart_dev)) {
         LOG_ERR("UART device not ready");
         return;
     }
 
     /* Configura UART */
     ret = uart_irq_callback_set(uart_dev, uart_rx_handler);
     if (ret < 0) {
         LOG_ERR("Cannot set UART callback (err: %d)", ret);
         return;
     }
 
     /* Abilita interrupt UART */
     uart_irq_rx_enable(uart_dev);
 
     LOG_INF("ESP32 Weather Station - Zephyr Edition");
 
     /* Inizializza e avvia i thread */
     k_tid_t wifi_tid = k_thread_create(&wifi_thread_data, wifi_stack,
                                     K_THREAD_STACK_SIZEOF(wifi_stack),
                                     wifi_connect_thread, NULL, NULL, NULL,
                                     K_PRIO_PREEMPT(7), 0, K_NO_WAIT);
     k_thread_name_set(wifi_tid, "wifi_thread");
 
     k_tid_t uart_tid = k_thread_create(&uart_thread_data, uart_stack,
                                     K_THREAD_STACK_SIZEOF(uart_stack),
                                     uart_rx_thread, NULL, NULL, NULL,
                                     K_PRIO_PREEMPT(8), 0, K_NO_WAIT);
     k_thread_name_set(uart_tid, "uart_thread");
 
     k_tid_t http_tid = k_thread_create(&http_thread_data, http_stack,
                                     K_THREAD_STACK_SIZEOF(http_stack),
                                     http_client_thread, NULL, NULL, NULL,
                                     K_PRIO_PREEMPT(9), 0, K_NO_WAIT);
     k_thread_name_set(http_tid, "http_thread");
 }
 
 /* Thread per la connessione WiFi */
 static void wifi_connect_thread(void *p1, void *p2, void *p3)
 {
     struct net_if *iface;
     struct wifi_connect_req_params wifi_params = {0};
     int ret;
 
     /* Registra callback per eventi WiFi */
     net_mgmt_init_event_callback(&wifi_cb, wifi_mgmt_event_handler,
                               NET_EVENT_WIFI_CONNECT_RESULT);
     net_mgmt_add_event_callback(&wifi_cb);
 
     /* Ottieni interfaccia WiFi */
     iface = net_if_get_default();
     if (!iface) {
         LOG_ERR("Cannot get default network interface");
         return;
     }
 
     /* Configura parametri WiFi */
     wifi_params.ssid = WIFI_SSID;
     wifi_params.ssid_length = strlen(WIFI_SSID);
     wifi_params.psk = WIFI_PSK;
     wifi_params.psk_length = strlen(WIFI_PSK);
     wifi_params.channel = WIFI_CHANNEL_ANY;
     wifi_params.security = WIFI_SECURITY_TYPE_PSK;
 
     /* Connetti al WiFi */
     LOG_INF("Connecting to WiFi: %s", WIFI_SSID);
     ret = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &wifi_params, sizeof(wifi_params));
     if (ret) {
         LOG_ERR("Failed to connect to WiFi: %d", ret);
         return;
     }
 
     /* Attendi connessione WiFi */
     while (!wifi_connected) {
         k_sleep(K_MSEC(100));
     }
 
     LOG_INF("WiFi connected");
 }
 
 /* Thread per la ricezione UART */
 static void uart_rx_thread(void *p1, void *p2, void *p3)
 {
     while (1) {
         /* Il thread resta in esecuzione ma la ricezione dati è gestita 
            dall'interrupt handler */
         k_sleep(K_SECONDS(1));
     }
 }
 
 /* Thread per l'invio HTTP */
 static void http_client_thread(void *p1, void *p2, void *p3)
 {
     struct sensor_data_item *rx_data;
 
     while (1) {
         /* Attendi dati dalla FIFO */
         rx_data = k_fifo_get(&sensor_data_fifo, K_FOREVER);
         
         if (rx_data) {
             /* Invia i dati al database */
             send_to_database(&rx_data->data);
             
             /* Libera la memoria */
             k_free(rx_data);
         }
     }
 }
 
 /* Handler per eventi WiFi */
 static void wifi_mgmt_event_handler(struct net_mgmt_event_callback *cb,
                                   uint32_t mgmt_event,
                                   struct net_if *iface)
 {
     if (mgmt_event == NET_EVENT_WIFI_CONNECT_RESULT) {
         wifi_connected = true;
     }
 }
 
 /* Handler per ricezione UART */
 static void uart_rx_handler(const struct device *dev, void *user_data)
 {
     uint8_t c;
 
     if (!uart_irq_update(dev)) {
         return;
     }
 
     if (!uart_irq_rx_ready(dev)) {
         return;
     }
 
     /* Leggi carattere */
     while (uart_fifo_read(dev, &c, 1) == 1) {
         LOG_DBG("RX: %c", c);
         
         /* Aggiungi al buffer */
         if (rx_buf_pos < RECV_BUFFER_SIZE - 1) {
             rx_buf[rx_buf_pos++] = c;
             
             /* Se troviamo un newline, i dati sono completi */
             if (c == '\n') {
                 rx_buf[rx_buf_pos] = '\0';  /* Termina la stringa */
                 LOG_INF("Received data: %s", rx_buf);
                 
                 /* Alloca memoria per i dati del sensore */
                 struct sensor_data_item *data_item = k_malloc(sizeof(struct sensor_data_item));
                 if (!data_item) {
                     LOG_ERR("Failed to allocate memory for sensor data");
                     rx_buf_pos = 0;
                     return;
                 }
                 
                 /* Analizza i dati JSON */
                 parse_json_data(rx_buf, &data_item->data);
                 
                 /* Invia i dati alla FIFO */
                 k_fifo_put(&sensor_data_fifo, data_item);
                 
                 /* Resetta il buffer */
                 rx_buf_pos = 0;
             }
         } else {
             /* Buffer overflow */
             LOG_ERR("Receive buffer overflow");
             rx_buf_pos = 0;
         }
     }
 }
 
 /* Parser JSON */
 static void parse_json_data(char *json_string, struct sensor_data *data)
 {
     struct json_obj_descr sensor_descr[] = {
         JSON_OBJ_DESCR_PRIM(struct sensor_data, temperature, JSON_TOK_FLOAT),
         JSON_OBJ_DESCR_PRIM(struct sensor_data, pressure, JSON_TOK_FLOAT),
         JSON_OBJ_DESCR_PRIM(struct sensor_data, humidity, JSON_TOK_FLOAT),
         JSON_OBJ_DESCR_PRIM(struct sensor_data, rain_probability, JSON_TOK_FLOAT),
     };
 
     int ret = json_obj_parse(json_string, strlen(json_string), 
                           sensor_descr, ARRAY_SIZE(sensor_descr), data);
                           
     if (ret < 0) {
         LOG_ERR("JSON parsing failed: %d", ret);
         return;
     }
 
     LOG_INF("Temperatura: %.2f°C", (double)data->temperature);
     LOG_INF("Pressione: %.2f hPa", (double)data->pressure);
     LOG_INF("Umidità: %.2f%%", (double)data->humidity);
     LOG_INF("Prob. pioggia: %.2f%%", (double)data->rain_probability);
 }
 
 /* Invia dati al database */
 static void send_to_database(struct sensor_data *data)
 {
     int sock, ret;
     struct sockaddr_in addr;
     char json_buffer[JSON_BUFFER_SIZE];
     char http_buffer[JSON_BUFFER_SIZE + 256];
     
     /* Verifica connessione WiFi */
     if (!wifi_connected) {
         LOG_ERR("WiFi not connected, cannot send data");
         return;
     }
     
     /* Formatta JSON */
     snprintf(json_buffer, sizeof(json_buffer),
            "{\"temperature\":%.2f,\"pressure\":%.2f,\"humidity\":%.2f,\"rain_probability\":%.2f,\"timestamp\":%llu}",
            (double)data->temperature, (double)data->pressure, (double)data->humidity, 
            (double)data->rain_probability, (unsigned long long)k_uptime_get());
 
     /* Crea socket */
     sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
     if (sock < 0) {
         LOG_ERR("Failed to create socket: %d", errno);
         return;
     }
 
     /* Configura endpoint */
     memset(&addr, 0, sizeof(addr));
     addr.sin_family = AF_INET;
     addr.sin_port = htons(SERVER_PORT);
     zsock_inet_pton(AF_INET, SERVER_URL, &addr.sin_addr);
 
     /* Connetti al server */
     ret = zsock_connect(sock, (struct sockaddr *)&addr, sizeof(addr));
     if (ret < 0) {
         LOG_ERR("Failed to connect to server: %d", errno);
         zsock_close(sock);
         return;
     }
 
     /* Crea richiesta HTTP */
     snprintf(http_buffer, sizeof(http_buffer),
            "POST %s HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "Content-Type: application/json\r\n"
            "Content-Length: %zu\r\n"
            "\r\n"
            "%s",
            API_ENDPOINT, SERVER_URL, SERVER_PORT, strlen(json_buffer), json_buffer);
 
     /* Invia dati */
     ret = zsock_send(sock, http_buffer, strlen(http_buffer), 0);
     if (ret < 0) {
         LOG_ERR("Failed to send data: %d", errno);
         zsock_close(sock);
         return;
     }
 
     LOG_INF("Data sent to server (%d bytes)", ret);
 
     /* Ricevi risposta (opzionale) */
     ret = zsock_recv(sock, http_buffer, sizeof(http_buffer) - 1, 0);
     if (ret > 0) {
         http_buffer[ret] = '\0';
         LOG_INF("Server response: %s", http_buffer);
     }
 
     /* Chiudi socket */
     zsock_close(sock);
 }
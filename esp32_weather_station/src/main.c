/**
 * Zephyr Weather Station - Integrazione con ThingSpeak
 * 
 * Riceve dati meteorologici via UART, li decodifica come JSON
 * e li invia a ThingSpeak via HTTP GET.
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/net/http/client.h>
#include <zephyr/logging/log.h>
#include <zephyr/data/json.h>
#include <sys/errno.h>  /* Per errno */
#include "secrets.h"

/* Configurazione logging */
LOG_MODULE_REGISTER(weather_station, LOG_LEVEL_INF);

/* Definizioni delle funzioni socket di Zephyr */
#ifndef CONFIG_NET_SOCKETS_POSIX_NAMES
#include <zephyr/posix/unistd.h>
extern int zsock_socket(int family, int type, int proto);
extern int zsock_connect(int sock, const struct sockaddr *addr, socklen_t addrlen);
extern int zsock_close(int sock);
#define socket zsock_socket
#define connect zsock_connect
#define close zsock_close
#endif

/* Configurazione WiFi - Usa i valori dal tuo secrets.h */
#define WIFI_SSID WIFI_ID               
#define WIFI_PSK WIFI_PASSWORD          

/* Configurazione server e API */
#define THINGSPEAK_API_KEY THING_SPEAK_API   
#define SERVER_ADDRESS SERVER_THINGSPEAK_API              // QUESTO DA RIVEDERE 

/* UART */
#define UART_DEVICE_NAME "UART_0"  // Nome del device UART disponibile
#define UART_BUFFER_SIZE 256

/* Networking */
#define HTTP_TIMEOUT_MS 10000

/* Thread e sincronizzazione */
#define STACK_SIZE 2048
#define THREAD_PRIORITY 5
K_MSGQ_DEFINE(uart_msgq, UART_BUFFER_SIZE, 10, 4);

/* Struttura dati meteo */
struct weather_data {
    double temperature;
    double pressure;
    double humidity;
    double rain_probability;
};

/* JSON parsing descr */
static const struct json_obj_descr weather_data_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct weather_data, temperature, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct weather_data, pressure, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct weather_data, humidity, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct weather_data, rain_probability, JSON_TOK_NUMBER)
};

/* Globali */
static const struct device *uart_dev;
static char rx_buf[UART_BUFFER_SIZE];
static int rx_buf_pos;
static bool connected;

/* Callback per la ricezione UART */
static void uart_cb(const struct device *dev, void *user_data)
{
    uint8_t c;

    if (!uart_irq_update(dev)) {
        return;
    }

    if (uart_irq_rx_ready(dev)) {
        while (uart_fifo_read(dev, &c, 1) == 1) {
            rx_buf[rx_buf_pos++] = c;

            if (c == '\n' || rx_buf_pos == UART_BUFFER_SIZE - 1) {
                rx_buf[rx_buf_pos] = '\0';
                
                /* Invia il buffer alla coda messaggi */
                if (k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT) != 0) {
                    LOG_ERR("Impossibile accodare messaggio UART");
                }
                
                /* Reset del buffer */
                rx_buf_pos = 0;
            }
        }
    }
}

/* Inizializzazione dell'UART */
static int uart_init(void)
{
    uart_dev = device_get_binding(UART_DEVICE_NAME);
    if (!uart_dev) {
        LOG_ERR("UART device non trovato");
        return -ENODEV;
    }

    uart_irq_callback_set(uart_dev, uart_cb);
    uart_irq_rx_enable(uart_dev);

    rx_buf_pos = 0;
    return 0;
}

/* Connessione WiFi */
static int wifi_connect(void)
{
    struct net_if *iface = net_if_get_default();
    if (!iface) {
        LOG_ERR("Interfaccia di rete non trovata");
        return -ENODEV;
    }

    struct wifi_connect_req_params params = {
        .ssid = WIFI_SSID,
        .ssid_length = strlen(WIFI_SSID),
        .psk = WIFI_PSK,
        .psk_length = strlen(WIFI_PSK),
        .channel = WIFI_CHANNEL_ANY,
        .security = WIFI_SECURITY_TYPE_PSK,
    };

    LOG_INF("Connessione al WiFi %s...", WIFI_SSID);
    if (net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params))) {
        LOG_ERR("Connessione WiFi fallita");
        return -ENOEXEC;
    }

    connected = true;
    LOG_INF("WiFi connesso");
    return 0;
}

/* Parsing JSON */
static int parse_json(char *json_string, struct weather_data *data)
{
    int ret;

    ret = json_obj_parse(json_string, strlen(json_string), 
                        weather_data_descr, 
                        ARRAY_SIZE(weather_data_descr),
                        data);

    if (ret < 0) {
        LOG_ERR("Errore di parsing JSON: %d", ret);
        return -EINVAL;
    }

    if (ret < ARRAY_SIZE(weather_data_descr)) {
        LOG_WRN("Alcuni campi JSON non sono stati analizzati, ret = %d", ret);
    }

    return 0;
}

/* Callback per risposta HTTP */
static void http_response_cb(struct http_response *rsp,
                            enum http_final_call final_data,
                            void *user_data)
{
    if (final_data == HTTP_DATA_FINAL) {
        LOG_INF("Risposta server: %d", rsp->http_status_code);
        /* Non possiamo accedere direttamente al contenuto della risposta in questo modo */
    }
}

/* Parse URL to extract host, port and path */
static int parse_url(const char *url, char *host, size_t host_len, 
                    uint16_t *port, char *path, size_t path_len)
{
    /* Skip http:// or https:// */
    const char *start = url;
    if (strncmp(url, "http://", 7) == 0) {
        start = url + 7;
        *port = 80;
    } else if (strncmp(url, "https://", 8) == 0) {
        start = url + 8;
        *port = 443;
    } else {
        *port = 80;  /* Default to HTTP port */
    }

    /* Find path part (after first '/' after host) */
    const char *path_start = strchr(start, '/');
    if (path_start == NULL) {
        /* No path, use root */
        strncpy(path, "/", path_len);
        path_start = start + strlen(start);
    } else {
        strncpy(path, path_start, path_len);
    }

    /* Extract host */
    size_t host_part_len = path_start - start;
    if (host_part_len >= host_len) {
        return -EINVAL;  /* Host buffer too small */
    }
    
    strncpy(host, start, host_part_len);
    host[host_part_len] = '\0';

    /* Check for port specification */
    char *port_start = strchr(host, ':');
    if (port_start != NULL) {
        *port_start = '\0';  /* Terminate host name before port */
        *port = (uint16_t)strtol(port_start + 1, NULL, 10);
    }

    return 0;
}

/* Invia dati al server specificato in SERVER_URL */
static int send_data_to_server(struct weather_data *data)
{
    struct http_request req;
    int sock;
    int ret;
    char url_path[128];
    char host[64];
    uint16_t port;
    char payload[256];
    struct sockaddr_in addr;

    /* Verifica connessione WiFi */
    if (!connected) {
        LOG_INF("WiFi disconnesso. Tentativo di riconnessione...");
        if (wifi_connect() != 0) {
            LOG_ERR("Impossibile riconnettersi al WiFi");
            return -ENETUNREACH;
        }
    }

    /* Parse dell'URL del server */
    ret = parse_url(SERVER_ADDRESS, host, sizeof(host), &port, url_path, sizeof(url_path));
    if (ret < 0) {
        LOG_ERR("Errore nel parsing dell'URL: %d", ret);
        return ret;
    }

    LOG_INF("Connessione a host: %s, porta: %d, path: %s", host, port, url_path);

    /* Costruisci il payload JSON */
    snprintf(payload, sizeof(payload),
             "{\"temperature\":%.2f,\"pressure\":%.2f,\"humidity\":%.2f,\"rain\":%d}",
             data->temperature,
             data->pressure,
             data->humidity,
             (int)data->rain_probability);

    LOG_INF("Payload: %s", payload);

    /* Risolvi il DNS e prepara l'indirizzo */
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    /* Se l'indirizzo è numerico, lo converte direttamente */
    ret = net_addr_pton(AF_INET, "192.168.63.121", &addr.sin_addr);
    if (ret < 0) {
        LOG_ERR("Conversione indirizzo IP fallita: %d", ret);
        return -EINVAL;
    }

    /* Crea il socket */
#ifdef CONFIG_NET_SOCKETS_POSIX_NAMES
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
    if (sock < 0) {
        LOG_ERR("Creazione socket fallita: %d", errno);
        return -errno;
    }

    /* Connettiti al server */
#ifdef CONFIG_NET_SOCKETS_POSIX_NAMES
    ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
#else
    ret = zsock_connect(sock, (struct sockaddr *)&addr, sizeof(addr));
#endif
    if (ret < 0) {
        LOG_ERR("Connessione al server fallita: %d", errno);
#ifdef CONFIG_NET_SOCKETS_POSIX_NAMES
        close(sock);
#else
        zsock_close(sock);
#endif
        return -errno;
    }

    /* Prepara la richiesta HTTP */
    memset(&req, 0, sizeof(req));
    req.method = HTTP_POST;
    req.url = url_path;
    req.host = host;
    req.protocol = "HTTP/1.1";
    req.payload = payload;
    req.payload_len = strlen(payload);
    req.header_fields = "Content-Type: application/json\r\n";
    req.response = http_response_cb;
    
    /* Invia la richiesta */
    ret = http_client_req(sock, &req, HTTP_TIMEOUT_MS, NULL);
    
    if (ret >= 0) {
        LOG_INF("Richiesta HTTP inviata con successo");
    } else {
        LOG_ERR("Errore nella richiesta HTTP: %d", ret);
    }

#ifdef CONFIG_NET_SOCKETS_POSIX_NAMES
    close(sock);
#else
    zsock_close(sock);
#endif
    return ret;
}

/* Invia dati a ThingSpeak */
static int send_to_thingspeak(struct weather_data *data)
{
    struct http_request req;
    int sock;
    int ret;
    char url[256];

    /* Verifica connessione WiFi */
    if (!connected) {
        LOG_INF("WiFi disconnesso. Tentativo di riconnessione...");
        if (wifi_connect() != 0) {
            LOG_ERR("Impossibile riconnettersi al WiFi");
            return -ENETUNREACH;
        }
    }

    /* Costruisci l'URL con i parametri */
    snprintf(url, sizeof(url),
        "/update?api_key=%s&field1=%.2f&field2=%.2f&field3=%.2f&field4=%d",
        THINGSPEAK_API_KEY,
        data->temperature,
        data->pressure,
        data->humidity,
        (int)data->rain_probability);

    LOG_INF("URL ThingSpeak: %s", url);

    /* Connetti al server ThingSpeak */
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    
    /* Risolvi DNS (in un'applicazione reale dovresti usare getaddrinfo) */
    ret = net_addr_pton(AF_INET, "184.106.153.149", &addr.sin_addr); /* IP di api.thingspeak.com */
    if (ret < 0) {
        LOG_ERR("Conversione indirizzo IP fallita: %d", ret);
        return -EINVAL;
    }
    
#ifdef CONFIG_NET_SOCKETS_POSIX_NAMES
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#endif
    if (sock < 0) {
        LOG_ERR("Creazione socket fallita: %d", errno);
        return -errno;
    }

#ifdef CONFIG_NET_SOCKETS_POSIX_NAMES
    ret = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
#else
    ret = zsock_connect(sock, (struct sockaddr *)&addr, sizeof(addr));
#endif
    if (ret < 0) {
        LOG_ERR("Connessione al server ThingSpeak fallita: %d", errno);
#ifdef CONFIG_NET_SOCKETS_POSIX_NAMES
        close(sock);
#else
        zsock_close(sock);
#endif
        return -errno;
    }

    /* Prepara la richiesta HTTP */
    memset(&req, 0, sizeof(req));
    req.method = HTTP_GET;
    req.url = url;
    req.host = "api.thingspeak.com";
    req.protocol = "HTTP/1.1";
    req.payload = NULL;
    req.payload_len = 0;
    req.response = http_response_cb;
    
    /* Invia la richiesta */
    ret = http_client_req(sock, &req, HTTP_TIMEOUT_MS, NULL);
    
    if (ret >= 0) {
        LOG_INF("Richiesta ThingSpeak inviata con successo");
    } else {
        LOG_ERR("Errore ThingSpeak: %d", ret);
    }

#ifdef CONFIG_NET_SOCKETS_POSIX_NAMES
    close(sock);
#else
    zsock_close(sock);
#endif
    return ret;
}

/* Thread principale */
void main_thread(void *p1, void *p2, void *p3)
{
    char uart_buf[UART_BUFFER_SIZE];
    struct weather_data data;
    int ret;

    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    LOG_INF("Zephyr Weather Station");

    /* Inizializza UART */
    if (uart_init() != 0) {
        LOG_ERR("Errore inizializzazione UART");
        return;
    }

    /* Connessione WiFi */
    if (wifi_connect() != 0) {
        LOG_ERR("Errore connessione WiFi");
        return;
    }

    while (1) {
        /* Aspetta messaggio dalla coda UART */
        if (k_msgq_get(&uart_msgq, &uart_buf, K_FOREVER) == 0) {
            LOG_INF("Dati ricevuti: %s", uart_buf);

            ret = parse_json(uart_buf, &data);
            if (ret == 0) {
                LOG_INF("Temperatura: %.2f°C", data.temperature);
                LOG_INF("Pressione: %.2f hPa", data.pressure);
                LOG_INF("Umidità: %.2f%%", data.humidity);
                LOG_INF("Pioggia prevista: %d", (int)data.rain_probability);

                /* Invia dati a ThingSpeak */
                ret = send_to_thingspeak(&data);
                if (ret < 0) {
                    LOG_ERR("Errore invio dati a ThingSpeak: %d", ret);
                }

                /* Invia dati al server personalizzato */
                ret = send_data_to_server(&data);
                if (ret < 0) {
                    LOG_ERR("Errore invio dati al server: %d", ret);
                }
            } else {
                LOG_ERR("Errore parsing JSON: %d", ret);
            }
        }
    }
}

/* Definizione thread principale */
K_THREAD_DEFINE(main_id, STACK_SIZE, main_thread, NULL, NULL, NULL,
                THREAD_PRIORITY, 0, 0);

/* Funzione main - necessaria ma può restare vuota, il thread principale è già avviato */
int main(void)
{
    /* Il thread principale è già stato avviato dal kernel */
    return 0;
}
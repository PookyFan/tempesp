#include "espressif/esp_common.h"
#include "esplibs/libnet80211.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"

#include "lwip/netdb.h"
#include "lwip/api.h"

#include "drivers.h"
#include "utils.h"

//Config file must define WIFI_SSID, WIFI_PASS and SERVER_HOSTNAME
#include "../config.h"

#define APP_VER "0.0.1"

static StackType_t app_task_stack[4096]; //Multiple of 32-bit words
static StaticTask_t app_task_tcb;

void run_app_task(void *pvParameters)
{
    netif_set_hostname(netif_default, "testmyesp");
    sdk_wifi_station_connect();
    do
    {
        printf("Waiting for IP...\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        //todo: timeout
    } while(sdk_wifi_station_get_connect_status() != STATION_GOT_IP);

    printf("Doing DNS lookup for " SERVER_HOSTNAME "\n");
    const struct addrinfo hints = {
        .ai_family = AF_UNSPEC,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    int err = getaddrinfo(SERVER_HOSTNAME, NULL, &hints, &res);
    if(err != 0 || res == NULL)
    {
        printf("Something went wrong - err=%d res=%p\n", err, res);
    }
    else
    {
        struct sockaddr *sa = res->ai_addr;
        if (sa->sa_family == AF_INET)
        {
            printf("DNS lookup succeeded. IP=%s\n\n", inet_ntoa(((struct sockaddr_in *)sa)->sin_addr));
        }
        freeaddrinfo(res);
    }

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    //todo: things to do:
    // - check for temperature
    // - check for humidity (optional)
    // - send data from sensors with identification data (station name, FW app version, whatelse)
    // - wait for OTA instruction, if it arrives - download and write new FW to the inactive slot
    // - if we can drive central heating - wait for switch instruction and perform it if it arrives
    // - fall into deep sleep if no further actions are required (todo: do we need to restart after OTA instead?)
    sdk_wifi_station_stop();
    sdk_system_deep_sleep(10 S_IN_US);
    printf("Falling into deep sleep for 10 seconds...\n");
    while(true)
        vTaskDelay(33 / portTICK_PERIOD_MS);
}

void user_init()
{
    uart_set_baud(0, 115200);
    printf("\nTempesp app version: " APP_VER "\nSDK version: %s\n", sdk_system_get_sdk_version());

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
    sdk_wifi_station_set_auto_connect(false);

    xTaskCreateStatic(run_app_task, "app", ARRAY_SIZE(app_task_stack), NULL, 1, app_task_stack, &app_task_tcb);
}

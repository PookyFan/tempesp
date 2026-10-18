#include <string.h>
#include "espressif/esp_common.h"
#include "esplibs/libnet80211.h"
#include "esp/uart.h"
#include "FreeRTOS.h"
#include "task.h"

#include "data.h"
#include "drivers.h"
#include "networking.h"
#include "utils.h"

//Config file must define WIFI_SSID, WIFI_PASS, INSTANCE_ID, and SERVER_HOSTNAME
#include "../config.h"

#define APP_VER_MAJ   0
#define APP_VER_MIN   1
#define APP_VER_PATCH 0

static StackType_t app_task_stack[4096]; //Multiple of 32-bit words
static StaticTask_t app_task_tcb;

//Fill common fields of report and checksum once other fields are filled
static void fill_report_common(struct report_t* report)
{
    memcpy(report->marker, "tesp", sizeof(report->marker));
    report->version_major = APP_VER_MAJ;
    report->version_minor = APP_VER_MIN;
    report->instance_id = INSTANCE_ID;
    report->seq_nr = 1;
    report->checksum = 0;

    int i;
    uint8_t checksum = 0xFF;
    uint8_t* ptr = (uint8_t*)report;
    for(i = 0; i < sizeof(*report); ++i)
        checksum ^= ptr[i];

    report->checksum = checksum;
}

static void step_report_seq_nr(struct report_t* report)
{
    report->checksum ^= report->seq_nr;
    ++report->seq_nr;
    report->checksum ^= report->seq_nr;
}

static void report_to_server(struct report_t* report)
{
    int retries = 5;
    do
    {
        if(!send_report(report))
            return;
        
        printf("Sent report with seq=%d, awaiting response...\n", report->seq_nr);
        if(receive_server_response())
            break;
        
        printf("Could not receive server response!\n");
        step_report_seq_nr(report);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    } while (--retries);

    if(retries == 0)
        printf("Failed to communicate with server!\n");
    else
        printf("Communication with server succeeded!\n");
}

static void run_app_task(void* pvParameters)
{
    printf("Initializing sensor...\n");
    bool result = initialize_sensor();
    printf("Initializing sensor %s!\n", result ? "succeeded" : "failed");

    struct report_t report;
    if( !(report.measurements_present = result && get_measurements(&report.measurements)) )
    {
        printf("Failed to get measurements!\n");
    }

    netif_set_hostname(netif_default, "testmyesp");
    printf("Enabling wifi...\n");
    sdk_wifi_station_connect();
    do
    {
        printf("Waiting for IP...\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        //todo: timeout
    } while(sdk_wifi_station_get_connect_status() != STATION_GOT_IP);

    printf("Preparing to send report to %s on port %d...\n", SERVER_HOSTNAME, SERVER_PORT);
    if(init_udp_socket() && init_server_addr(SERVER_HOSTNAME, SERVER_PORT))
    {
        fill_report_common(&report);
        printf("Communicating with server...\n");
        report_to_server(&report);
    }

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    //todo: things to do:
    // - wait for OTA instruction, if it arrives - download and write new FW to the inactive slot
    // - if we can drive central heating - wait for switch instruction and perform it if it arrives
    // - fall into deep sleep if no further actions are required (todo: do we need to restart after OTA instead?)
    printf("Disabling wifi...\n");
    sdk_wifi_station_stop();
    printf("Falling into deep sleep for 10 seconds...\n");
    sdk_system_deep_sleep(10 S_IN_US);
    while(true)
        vTaskDelay(33 / portTICK_PERIOD_MS);
}

void user_init()
{
    uart_set_baud(0, 115200);
    printf("\nTempesp app version: %d.%d.%d\nSDK version: %s\n",
           APP_VER_MAJ, APP_VER_MIN, APP_VER_PATCH, sdk_system_get_sdk_version());

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);
    sdk_wifi_station_set_auto_connect(false);

    xTaskCreateStatic(run_app_task, "app", ARRAY_SIZE(app_task_stack), NULL, 1, app_task_stack, &app_task_tcb);
}

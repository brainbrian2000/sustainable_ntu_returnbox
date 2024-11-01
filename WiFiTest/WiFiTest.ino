#include <string.h>
#include <stdlib.h>
#include "esp_wifi.h"
#include "esp_eap_client.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_ping.h"
#include "ping/ping_sock.h"
#include <WiFiClient.h>
#include <ESPping.h>
/* The examples use simple WiFi configuration that you can set via
   project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"

   You can choose EAP method via project configuration according to the
   configuration of AP.
*/
// #define EXAMPLE_WIFI_SSID "ntu_peap"
#define EXAMPLE_EAP_METHOD CONFIG_EXAMPLE_EAP_METHOD_PEAP


#define EXAMPLE_WIFI_SSID "RiceballFan_peap"
#define EXAMPLE_EAP_ID "brainbrian2000"
#define EXAMPLE_EAP_USERNAME "brainbrian2000"
#define EXAMPLE_EAP_PASSWORD "brainbrian2000"
// #define EXAMPLE_EAP_ID "b08209023"
// #define EXAMPLE_EAP_USERNAME "b08209023"
// #define EXAMPLE_EAP_PASSWORD "Brian168"

// #define EXAMPLE_ESP_WIFI_SSID "SCplusNTU"
// #define EXAMPLE_ESP_WIFI_PASS "smartcampus206"
#define CONFIG_ESP_WIFI_AUTH_WPA2_PSK 1
#define EXAMPLE_ESP_WIFI_SSID "Riceball_Fan"
#define EXAMPLE_ESP_WIFI_PASS "brainbrian2000"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5
#define DEFAULT_SCAN_LIST_SIZE 20
static int s_retry_num = 0;
// #define CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK 1
/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;
static esp_netif_t *sta_netif = NULL;

/* esp netif object representing the WIFI station */

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

static const char *TAG = "example";

/* CA cert, taken from ca.pem
   Client cert, taken from client.crt
   Client key, taken from client.key

   The PEM, CRT and KEY file were provided by the person or organization
   who configured the AP with wifi enterprise.

   To embed it in the app binary, the PEM, CRT and KEY file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
#if defined(CONFIG_EXAMPLE_VALIDATE_SERVER_CERT) || \
    defined(CONFIG_EXAMPLE_WPA3_ENTERPRISE) || \
    defined(CONFIG_EXAMPLE_WPA3_192BIT_ENTERPRISE) || \
    defined(CONFIG_ESP_WIFI_EAP_TLS1_3)
#define SERVER_CERT_VALIDATION_ENABLED
#endif

#ifdef SERVER_CERT_VALIDATION_ENABLED
extern uint8_t ca_pem_start[] asm("_binary_ca_pem_start");
extern uint8_t ca_pem_end[]   asm("_binary_ca_pem_end");
#endif /* SERVER_CERT_VALIDATION_ENABLED */

#ifdef CONFIG_EXAMPLE_EAP_METHOD_TLS
extern uint8_t client_crt_start[] asm("_binary_client_crt_start");
extern uint8_t client_crt_end[]   asm("_binary_client_crt_end");
extern uint8_t client_key_start[] asm("_binary_client_key_start");
extern uint8_t client_key_end[]   asm("_binary_client_key_end");
#endif /* CONFIG_EXAMPLE_EAP_METHOD_TLS */

#if defined CONFIG_EXAMPLE_EAP_METHOD_TTLS
esp_eap_ttls_phase2_types TTLS_PHASE2_METHOD = CONFIG_EXAMPLE_EAP_METHOD_TTLS_PHASE_2;
#endif /* CONFIG_EXAMPLE_EAP_METHOD_TTLS */

// static void event_handler(void* arg, esp_event_base_t event_base,
//                                 int32_t event_id, void* event_data)
// {
//     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
//         esp_wifi_connect();
//     } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
//         esp_wifi_connect();
//         xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
//     } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
//         xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
//     }
// }
static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        Serial.println("Connecting to the AP");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            Serial.println("retry to connect to the AP");
        } else {
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            esp_wifi_stop();
        }
        Serial.println("connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        // ESP_LOGI(sta_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        Serial.printf("got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
}
#define CONFIG_EXAMPLE_EAP_METHOD_PEAP
static void initialise_wifi(void)
{
#ifdef SERVER_CERT_VALIDATION_ENABLED
    unsigned int ca_pem_bytes = ca_pem_end - ca_pem_start;
#endif /* SERVER_CERT_VALIDATION_ENABLED */

#ifdef CONFIG_EXAMPLE_EAP_METHOD_TLS
    unsigned int client_crt_bytes = client_crt_end - client_crt_start;
    unsigned int client_key_bytes = client_key_end - client_key_start;
#endif /* CONFIG_EXAMPLE_EAP_METHOD_TLS */

    // ESP_ERROR_CHECK(esp_netif_init());
    // wifi_event_group = xEventGroupCreate();
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    // sta_netif = esp_netif_create_default_wifi_sta();
    // assert(sta_netif);
    Serial.println("wifi init");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_WIFI_SSID,
#if defined (CONFIG_EXAMPLE_WPA3_192BIT_ENTERPRISE) || defined (CONFIG_EXAMPLE_WPA3_ENTERPRISE)
            .pmf_cfg = {
                .required = true
            },
#endif
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_eap_client_set_identity((uint8_t *)EXAMPLE_EAP_ID, strlen(EXAMPLE_EAP_ID)) );
    Serial.println("esp_eap_client_set_identity");
#ifdef SERVER_CERT_VALIDATION_ENABLED
    ESP_ERROR_CHECK(esp_eap_client_set_ca_cert(ca_pem_start, ca_pem_bytes) );
#endif /* SERVER_CERT_VALIDATION_ENABLED */

#ifdef CONFIG_EXAMPLE_EAP_METHOD_TLS
    ESP_ERROR_CHECK(esp_eap_client_set_certificate_and_key(client_crt_start, client_crt_bytes,
                                      client_key_start, client_key_bytes, NULL, 0) );
#endif /* CONFIG_EXAMPLE_EAP_METHOD_TLS */

    // ESP_ERROR_CHECK(esp_eap_client_set_username((uint8_t *)EXAMPLE_EAP_USERNAME, strlen(EXAMPLE_EAP_USERNAME)) );
    // ESP_ERROR_CHECK(esp_eap_client_set_password((uint8_t *)EXAMPLE_EAP_PASSWORD, strlen(EXAMPLE_EAP_PASSWORD)) );
#if defined (CONFIG_EXAMPLE_EAP_METHOD_PEAP) || \
    defined (CONFIG_EXAMPLE_EAP_METHOD_TTLS)
    ESP_ERROR_CHECK(esp_eap_client_set_username((uint8_t *)EXAMPLE_EAP_USERNAME, strlen(EXAMPLE_EAP_USERNAME)) );
    ESP_ERROR_CHECK(esp_eap_client_set_password((uint8_t *)EXAMPLE_EAP_PASSWORD, strlen(EXAMPLE_EAP_PASSWORD)) );
#endif /* CONFIG_EXAMPLE_EAP_METHOD_PEAP || CONFIG_EXAMPLE_EAP_METHOD_TTLS */

#if defined CONFIG_EXAMPLE_EAP_METHOD_TTLS
    ESP_ERROR_CHECK(esp_eap_client_set_ttls_phase2_method(TTLS_PHASE2_METHOD) );
#endif /* CONFIG_EXAMPLE_EAP_METHOD_TTLS */

#if defined (CONFIG_EXAMPLE_WPA3_192BIT_ENTERPRISE)
    ESP_LOGI(TAG, "Enabling 192 bit certification");
    ESP_ERROR_CHECK(esp_eap_client_set_suiteb_192bit_certification(true));
#endif
#ifdef CONFIG_EXAMPLE_USE_DEFAULT_CERT_BUNDLE
    ESP_ERROR_CHECK(esp_eap_client_use_default_cert_bundle(true));
#endif
    ESP_ERROR_CHECK(esp_wifi_sta_enterprise_enable());
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
    Serial.println("wifi start");

}





#ifdef CONFIG_EXAMPLE_USE_SCAN_CHANNEL_BITMAP
#define USE_CHANNEL_BTIMAP 1
#define CHANNEL_LIST_SIZE 3
static uint8_t channel_list[CHANNEL_LIST_SIZE] = {1, 6, 11};
#endif /*CONFIG_EXAMPLE_USE_SCAN_CHANNEL_BITMAP*/

static const char *TAG_scan = "scan";

static void print_auth_mode(int authmode)
{
    switch (authmode) {
        case WIFI_AUTH_OPEN:
            Serial.println("Authmode \tWIFI_AUTH_OPEN");
            break;
        case WIFI_AUTH_WEP:
            Serial.println("Authmode \tWIFI_AUTH_WEP");
            break;
        case WIFI_AUTH_WPA_PSK:
            Serial.println("Authmode \tWIFI_AUTH_WPA_PSK");
            break;
        case WIFI_AUTH_WPA2_PSK:
            Serial.println("Authmode \tWIFI_AUTH_WPA2_PSK");
            break;
        case WIFI_AUTH_WPA_WPA2_PSK:
            Serial.println("Authmode \tWIFI_AUTH_WPA_WPA2_PSK");
            break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
            Serial.println("Authmode \tWIFI_AUTH_WPA2_ENTERPRISE");
            break;
        case WIFI_AUTH_WPA3_PSK:
            Serial.println("Authmode \tWIFI_AUTH_WPA3_PSK");
            break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
            Serial.println("Authmode \tWIFI_AUTH_WPA2_WPA3_PSK");
            break;
        case WIFI_AUTH_WAPI_PSK:
            Serial.println("Authmode \tWIFI_AUTH_WAPI_PSK");
            break;
        default:
            Serial.println("Authmode \tUnknown");
            break;
    }
}

static void print_cipher_type(int pairwise_cipher, int group_cipher)
{
 
    Serial.print("Pairwise Cipher: ");
    Serial.print(pairwise_cipher == WIFI_CIPHER_TYPE_WEP40 ? "WEP40" :
                   pairwise_cipher == WIFI_CIPHER_TYPE_WEP104 ? "WEP104" :
                   pairwise_cipher == WIFI_CIPHER_TYPE_TKIP ? "TKIP" :
                   pairwise_cipher == WIFI_CIPHER_TYPE_CCMP ? "CCMP" :
                   pairwise_cipher == WIFI_CIPHER_TYPE_TKIP_CCMP ? "TKIP_CCMP" :
                   "Unknown");
    Serial.print("\t");
    Serial.print("Group Cipher: ");
    Serial.println(group_cipher == WIFI_CIPHER_TYPE_WEP40 ? "WEP40" :
                   group_cipher == WIFI_CIPHER_TYPE_WEP104 ? "WEP104" :
                   group_cipher == WIFI_CIPHER_TYPE_TKIP ? "TKIP" :
                   group_cipher == WIFI_CIPHER_TYPE_CCMP ? "CCMP" :
                   group_cipher == WIFI_CIPHER_TYPE_TKIP_CCMP ? "TKIP_CCMP" :
                   "Unknown");
}

#ifdef USE_CHANNEL_BTIMAP
static void array_2_channel_bitmap(const uint8_t channel_list[], const uint8_t channel_list_size, wifi_scan_config_t *scan_config) {

    for(uint8_t i = 0; i < channel_list_size; i++) {
        uint8_t channel = channel_list[i];
        scan_config->channel_bitmap.ghz_2_channels |= (1 << channel);
    }
}
#endif /*USE_CHANNEL_BTIMAP*/


/* Initialize Wi-Fi as sta and set scan method */
static void wifi_scan(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));
    Serial.println("Starting WiFi scan...");

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // 開始掃描，使用默認的掃描設置
    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));

    Serial.println("WiFi scan completed.");

    // 獲取掃描結果
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));

    Serial.printf("Total APs scanned: %u, APs stored in list: %u\n", ap_count, number);

    for (int i = 0; i < number; i++) {
        Serial.printf("SSID: %18s ", ap_info[i].ssid);
        Serial.printf("RSSI: %4d ", ap_info[i].rssi);
        print_auth_mode(ap_info[i].authmode);
        // if (ap_info[i].authmode != WIFI_AUTH_WEP) {
        //     print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
        // }
        // Serial.printf("Channel: %d\n", ap_info[i].primary);
        // Serial.println();
    }

    // 停止 WiFi，避免持續消耗功率
    ESP_ERROR_CHECK(esp_wifi_stop());
}


// #define CONFIG_ESP_WIFI_PW_ID "123456"
#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
    #define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
    #define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
    #define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
    #define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
    #define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
    #define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
    #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
    #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
    #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
    #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
    #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
    #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
    #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
    #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *sta_TAG = "wifi station";



static void sta_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        Serial.println("Connecting to the AP");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            // ESP_LOGI(sta_TAG, "retry to connect to the AP");
            Serial.println("retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            // xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            esp_wifi_stop();
        }
        // ESP_LOGI(sta_TAG,"connect to the AP fail");
        Serial.println("connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        // ESP_LOGI(sta_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        Serial.printf("got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    // esp_netif_init();
    // esp_event_loop_create_default();
    // esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // 註冊 Wi-Fi 事件處理器
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &sta_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_event_handler, NULL, &instance_got_ip);

    wifi_config_t wifi_config = {};
    // strncpy((char *) wifi_config.sta.ssid, EXAMPLE_ESP_WIFI_SSID, sizeof(wifi_config.sta.ssid));
    // strncpy((char *) wifi_config.sta.password, EXAMPLE_ESP_WIFI_PASS, sizeof(wifi_config.sta.password));
    strcpy((char *)wifi_config.sta.ssid, EXAMPLE_ESP_WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, EXAMPLE_ESP_WIFI_PASS);
    // strcpy((char *)wifi_config.sta.sae_h2e_identifier, EXAMPLE_H2E_IDENTIFIER);
    wifi_config.sta.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );
    // wifi_config.sta.sae_pwe_h2e = ESP_WIFI_SAE_MODE;
    // 設置 Wi-Fi 連接參數
    s_wifi_event_group = xEventGroupCreate();


    Serial.println("wifi_init_sta finished.");
    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        // Serial.printf("connected to ap SSID:%s password:%s\n",
        //          EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        Serial.println("connected to ap");
    } else if (bits & WIFI_FAIL_BIT) {
        // Serial.printf("Failed to connect to SSID:%s, password:%s\n",
        //          EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
        Serial.println("Failed to connect to SSID");
    } else {
        // ESP_LOGE(sta_TAG, "UNEXPECTED EVENT");
        Serial.println("UNEXPECTED EVENT");
    }
}



static void wifi_enterprise_example_task(void *pvParameters)
{
    esp_netif_ip_info_t ip;
    memset(&ip, 0, sizeof(esp_netif_ip_info_t));
    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    while (1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        // Serial.println("Waiting for IP ...");
        //check wifi is connect or not
        esp_netif_get_ip_info(sta_netif, &ip);
        if((uint16_t)(((const uint8_t*)(&(&ip.ip)->addr))[0])!=(uint16_t)0){
            // Serial.printf("WIFI_CONNECTED_BIT: %hd \n", WIFI_CONNECTED_BIT);
            // Serial.println((uint16_t)(((const uint8_t*)(&(&ip.ip)->addr))[0]));
            if (esp_netif_get_ip_info(sta_netif, &ip) == ESP_OK) {
                Serial.println("~~~~~~~~~~~");
                Serial.print("IP         : ");
                Serial.printf("%d.%d.%d.%d\n", IP2STR(&ip.ip));
                Serial.print("MASK       : ");
                Serial.printf("%d.%d.%d.%d\n", IP2STR(&ip.netmask));
                Serial.print("GW         : ");
                Serial.printf("%d.%d.%d.%d\n", IP2STR(&ip.gw));
                uint8_t mac[6];
                ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));  // WIFI_IF_STA 用於 Wi-Fi STA 模式

                vTaskDelete(NULL);
            } else {
                Serial.println("Failed to get IP information");
            }
        }else{
            // Serial.println("No Connection");
            uint8_t mac[6];
            ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));  // WIFI_IF_STA 用於 Wi-Fi STA 模式
            Serial.print("MAC Address: ");
            Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
            Serial.println("No Connection");
        }
    }
}
bool PingTest(){
    // bool success = Ping.ping("www.google.com", 3);
    int ip[][4] = {
        // {192,168,1,105},
        // {192,168,1,1},
        {8,8,8,8},
        // {218,35,162,254},
        // {192,168,1,102},
        {168,95,1,1}
    };

    // IPAddress ip(8,8,8,8);
    // IPAddress ip(192,168,1,105);
    bool success;
    for(int i = 0; i <2; i++){
        success = Ping.ping(IPAddress(ip[i][0], ip[i][1], ip[i][2], ip[i][3]), 3);
        if(!success){
            Serial.println("Ping failed");
            // return false;
            for(int j = 0; j <4;j++){
                Serial.print(ip[i][j]);
                Serial.print(' ');
            }
            Serial.println("");
        }else{
            Serial.println("Ping succesful.");
            
            for(int j = 0; j <4;j++){
                Serial.print(ip[i][j]);
                Serial.print(' ');
            }
            Serial.println("");
        }
    }
    return true;
} 



void setup(void)
{
    Serial.begin(115200);
    Serial.println("wifi init");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_scan();
    initialise_wifi();
    // WiFi.mode(WIFI_STA);
    // wifi_init_sta();
    // init_ping_test();
    xTaskCreate(&wifi_enterprise_example_task, "wifi_enterprise_example_task", 4096, NULL, 5, NULL);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    PingTest();
}
void loop(){

};
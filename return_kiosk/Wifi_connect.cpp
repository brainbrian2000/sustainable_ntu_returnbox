#ifndef _WIFI_CONNECT_CPP
#define _WIFI_CONNECT_CPP
    #if 1
        // #include "ca_pem.h"
        #include <esp_wifi.h>
        #include <esp_err.h>
        #include <esp_eap_client.h>
        #include <stdlib.h>
        #include <ESPping.h>
        #include <esp_netif_types.h>
        #include "esp_log.h"
        #include "esp_event.h"
        #include "esp_netif.h"
        #include "nvs_flash.h"
        #define PRINT_WIFI_LOG 1
        #define WIFI_CONNECTED_BIT BIT0
        #define WIFI_FAIL_BIT      BIT1
        #define DEFAULT_SCAN_LIST_SIZE 30
        #define EXAMPLE_ESP_MAXIMUM_RETRY 15
        #define EXAMPLE_EAP_METHOD CONFIG_EXAMPLE_EAP_METHOD_PEAP
        #define CONFIG_ESP_WIFI_AUTH_WPA2_PSK 1
        #define CONFIG_EXAMPLE_EAP_METHOD_PEAP
        static bool SetForceWiFiOff = false;
        static bool auto_close_wifi = false;
        // #define SERVER_CERT_VALIDATION_ENABLED
        static int s_retry_num = 0;
        static bool STATE_initial;
        // static esp_netif_t *sta_netif;
        static EventGroupHandle_t wifi_event_group;
        class WiFi_connection {
            private:
                char peap_ssid_wpa2[32] = "eduroam";          // 替換為您的 PEAP SSID
                char peap_username_wpa2[64] = "r13247001@eduroam.ntu.edu.tw";   // 替換為您的 PEAP 用戶名
                char peap_password_wpa2[64] = "RiceballFan2000";   // 替換為您的 PEAP 密碼

                char ssid_wpa2[32] = "Riceball_Fan";               // 替換為您的 WPA2 SSID
                char password_wpa2[64] = "brainbrian2000";        // 替換為您的 WPA2 密碼

                char default_ssid[32] = "DISCONNECTED";
                char* current_ssid = default_ssid;
                int type = 0;
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
                esp_event_handler_instance_t instance_any_id;
                esp_event_handler_instance_t instance_got_ip;
                bool connect_wpa2() {
                    esp_err_t ret = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);
                    if (ret == ESP_OK) {
                        Serial.println("WiFi event handler unregistered successfully.");
                    } else {
                        Serial.printf("Failed to unregister WiFi event handler: %s\n", esp_err_to_name(ret));
                    }
                    ret = esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
                    if (ret == ESP_OK) {
                        Serial.println("IP event handler unregistered successfully.");
                    } else {
                        Serial.printf("Failed to unregister IP event handler: %s\n", esp_err_to_name(ret));
                    }
                    Serial.println("wifi init-WPA2");
                    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                    esp_wifi_init(&cfg);

                    // 註冊 Wi-Fi 事件處理器
                    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFi_event_handler, NULL, &instance_any_id);
                    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFi_event_handler, NULL, &instance_got_ip);

                    wifi_config_t wifi_config = {};
                    strcpy((char *)wifi_config.sta.ssid, ssid_wpa2);
                    strcpy((char *)wifi_config.sta.password, password_wpa2);
                    // strcpy((char *)wifi_config.sta.)
                    // strcpy((char *)wifi_config.sta.sae_h2e_identifier, EXAMPLE_H2E_IDENTIFIER);
                    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

                    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
                    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
                    ESP_ERROR_CHECK(esp_wifi_start() );
                    // wifi_config.sta.sae_pwe_h2e = ESP_WIFI_SAE_MODE;
                    // 設置 Wi-Fi 連接參數
                    wifi_event_group = xEventGroupCreate();


                    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
                    * number of re-tries (WIFI_FAIL_BIT). The bits are set by WiFi_event_handler() (see above) */
                    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                            pdFALSE,
                            pdFALSE,
                            portMAX_DELAY);

                    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
                    * happened. */
                    if (bits & WIFI_CONNECTED_BIT) {
                        // Serial.printf("connected to ap SSID:%s password:%s\n",
                        //          EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
                        obtain_time();
                        Serial.println("connected to ap");
                    } else if (bits & WIFI_FAIL_BIT) {
                        // Serial.printf("Failed to connect to SSID:%s, password:%s\n",
                        //          EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
                        Serial.println("Failed to connect to SSID");
                    } else {
                        // ESP_LOGE(sta_TAG, "UNEXPECTED EVENT");
                        Serial.println("UNEXPECTED EVENT");
                    }
                    return true;
                }
                uint8_t best_bssid[6];
                bool find_strongest_bssid(const char* target_ssid, uint8_t* best_bssid) {
                    // uint16_t number = DEFAULT_SCAN_LIST_SIZE;
                    // wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
                    // uint16_t ap_count = 0;
                    // memset(ap_info, 0, sizeof(ap_info));
                    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

                    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
                    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
                    uint16_t ap_count = 0;
                    memset(ap_info, 0, sizeof(ap_info));
                    Serial.println("Starting WiFi scan...");

                    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
                    ESP_ERROR_CHECK(esp_wifi_start());

                    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));
                    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
                    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));


                    int best_rssi = -100;  // 初始值為非常弱的信號
                    bool found = false;

                    for (int i = 0; i < ap_count; i++) {
                        if (strcmp((char *)ap_info[i].ssid, target_ssid) == 0) {
                            if (ap_info[i].rssi > best_rssi) {
                                best_rssi = ap_info[i].rssi;
                                memcpy(best_bssid, ap_info[i].bssid, 6);
                                found = true;
                            }
                        }
                    }
                    return found;
                }
                bool connect_peap() {
                    esp_err_t ret = esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id);
                    if (ret == ESP_OK) {
                        Serial.println("WiFi event handler unregistered successfully.");
                    } else {
                        Serial.printf("Failed to unregister WiFi event handler: %s\n", esp_err_to_name(ret));
                    }
                    ret = esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip);
                    if (ret == ESP_OK) {
                        Serial.println("IP event handler unregistered successfully.");
                    } else {
                        Serial.printf("Failed to unregister IP event handler: %s\n", esp_err_to_name(ret));
                    }

                    // uint8_t best_bssid[6];
                    // if (!find_strongest_bssid(peap_ssid_wpa2, best_bssid)) {
                    //     Serial.println("No AP with matching SSID found.");
                    //     return false;
                    // }

                    #ifdef SERVER_CERT_VALIDATION_ENABLED
                        // extern const uint8_t ca_pem_start[] asm("_binary_ca_pem_start");
                        // extern const uint8_t ca_pem_end[] asm("_binary_ca_pem_end");
                        // unsigned int ca_pem_bytes = ca_pem_end - ca_pem_start;
                    #endif /* SERVER_CERT_VALIDATION_ENABLED */

                    #ifdef CONFIG_EXAMPLE_EAP_METHOD_TLS
                        // unsigned int client_crt_bytes = client_crt_end - client_crt_start;
                        // unsigned int client_key_bytes = client_key_end - client_key_start;
                    #endif /* CONFIG_EXAMPLE_EAP_METHOD_TLS */
                        Serial.println("wifi init-PEAP");
                        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
                        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WiFi_event_handler, instance_any_id));
                        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WiFi_event_handler, instance_got_ip));
                        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
                        // ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_phase2_method(ESP_PEAP_PHASE2_MSCHAPV2));

                        wifi_config_t wifi_config = {};
                        strcpy((char * )wifi_config.sta.ssid, (const char *)peap_ssid_wpa2);
                        memcpy(wifi_config.sta.bssid, best_bssid, 6);
                        wifi_config.sta.bssid_set = 1;
                        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_ENTERPRISE;

                        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
                        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
                        // ESP_ERROR_CHECK(esp_eap_client_set_identity((uint8_t *)peap_ssid_wpa2, strlen(peap_ssid_wpa2)) );
                    #ifdef SERVER_CERT_VALIDATION_ENABLED
                        // ESP_ERROR_CHECK(esp_eap_client_set_ca_cert(ca_pem, ca_pem_len) );
                    #endif /* SERVER_CERT_VALIDATION_ENABLED */

                    #ifdef CONFIG_EXAMPLE_EAP_METHOD_TLS
                        // ESP_ERROR_CHECK(esp_eap_client_set_certificate_and_key(client_crt_start, client_crt_bytes,
                        //                                 client_key_start, client_key_bytes, NULL, 0) );
                    #endif /* CONFIG_EXAMPLE_EAP_METHOD_TLS */

                    #if defined (CONFIG_EXAMPLE_EAP_METHOD_PEAP) || defined (CONFIG_EXAMPLE_EAP_METHOD_TTLS)
                        ESP_ERROR_CHECK(esp_eap_client_set_identity((uint8_t *)peap_username_wpa2, strlen(peap_username_wpa2)) );
                        Serial.printf("esp_eap_client_set_identity %s",peap_username_wpa2);
                        ESP_ERROR_CHECK(esp_eap_client_set_username((uint8_t *)peap_username_wpa2, strlen(peap_username_wpa2)) );
                        // ESP_ERROR_CHECK(esp_eap)
                        Serial.printf(", password : %s\n",peap_password_wpa2);
                        ESP_ERROR_CHECK(esp_eap_client_set_password((uint8_t *)peap_password_wpa2, strlen(peap_password_wpa2)) );
                        ESP_ERROR_CHECK(esp_eap_client_set_disable_time_check(true));
                        // ESP_ERROR_CHECK(esp_eap_client_set_new_password((uint8_t *)peap_password_wpa2, strlen(peap_password_wpa2)) );
                        // ESP_ERROR_CHECK(esp_eap_client_set_identity((uint8_t *)peap_ssid_wpa2, strlen(peap_ssid_wpa2)) );
                        // ESP_ERROR_CHECK(esp_eap_client_set)
                    #endif /* CONFIG_EXAMPLE_EAP_METHOD_PEAP || CONFIG_EXAMPLE_EAP_METHOD_TTLS */

                    #if defined CONFIG_EXAMPLE_EAP_METHOD_TTLS
                        // ESP_ERROR_CHECK(esp_eap_client_set_ttls_phase2_method(TTLS_PHASE2_METHOD) );
                    #endif /* CONFIG_EXAMPLE_EAP_METHOD_TTLS */

                    #if defined (CONFIG_EXAMPLE_WPA3_192BIT_ENTERPRISE)
                        // ESP_LOGI(TAG, "Enabling 192 bit certification");
                        // ESP_ERROR_CHECK(esp_eap_client_set_suiteb_192bit_certification(true));
                    #endif
                    #ifdef CONFIG_EXAMPLE_USE_DEFAULT_CERT_BUNDLE
                        // ESP_ERROR_CHECK(esp_eap_client_use_default_cert_bundle(true));
                    #endif
                    ESP_ERROR_CHECK(esp_wifi_sta_enterprise_enable());
                    ESP_ERROR_CHECK(esp_wifi_start());
                    ESP_ERROR_CHECK(esp_wifi_connect());
                    Serial.println("wifi start");
                    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
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
                    return true;
                }
                void scanAP() {
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

                bool scanAP_check(const char* ssid) {
                    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

                    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
                    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
                    uint16_t ap_count = 0;
                    memset(ap_info, 0, sizeof(ap_info));
                    Serial.println("Starting WiFi scan...");

                    // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
                    ESP_ERROR_CHECK(esp_wifi_start());

                    // 開始掃描，使用默認的掃描設置
                    ESP_ERROR_CHECK(esp_wifi_scan_start(NULL, true));

                    Serial.println("WiFi scan completed.");

                    // 獲取掃描結果
                    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
                    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));

                    Serial.printf("Total APs scanned: %u, APs stored in list: %u\n", ap_count, number);

                    for (int i = 0; i < number; i++) {
                        if(strcmp((const char *)ap_info[i].ssid, ssid) == 0) {
                            Serial.printf("SSID: %18s ", ap_info[i].ssid);
                            Serial.printf("RSSI: %4d ", ap_info[i].rssi);
                            print_auth_mode(ap_info[i].authmode);
                            int best_rssi = -100;  // 初始值為非常弱的信號
                            bool found = false;

                            for (int i = 0; i < ap_count; i++) {
                                if (strcmp((char *)ap_info[i].ssid, peap_ssid_wpa2) == 0) {
                                    if (ap_info[i].rssi > best_rssi) {
                                        best_rssi = ap_info[i].rssi;
                                        memcpy(best_bssid, ap_info[i].bssid, 6);
                                        found = true;
                                    }
                                }
                            }
                            ESP_ERROR_CHECK(esp_wifi_stop());
                            // return found;
                            return true;
                        }
                    }
                    ESP_ERROR_CHECK(esp_wifi_stop());
                    return false;
                    // 停止 WiFi，避免持續消耗功率

                }

            public:
                static void WiFi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
                        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
                            if(!SetForceWiFiOff){
                                esp_wifi_connect();
                            }
                            Serial.println("Connecting to the AP");
                        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
                            if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
                                if(!SetForceWiFiOff){
                                    esp_wifi_connect();
                                }
                                s_retry_num++;
                                Serial.println("retry to connect to the AP");
                                wifi_event_sta_disconnected_t* event = (wifi_event_sta_disconnected_t*) event_data;
                                Serial.printf("Disconnected from AP, reason: %d\n", event->reason);
                            } else {
                                xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
                                xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
                                if(SetForceWiFiOff | auto_close_wifi){
                                  esp_wifi_stop();
                                }
                            }
                            Serial.println("connect to the AP fail");
                        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
                            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                            Serial.printf("got ip:" IPSTR, IP2STR(&event->ip_info.ip));
                            s_retry_num = 0;
                            xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
                        }
                    }
                WiFi_connection() {
                    STATE_initial = false;
                }
                ~WiFi_connection() {
                    // 清理資源（如有需要）
                }

                void changeWPA2(const char* ssid, const char* password) {
                    strncpy(ssid_wpa2, ssid, sizeof(ssid_wpa2) - 1);
                    strncpy(password_wpa2, password, sizeof(password_wpa2) - 1);
                }

                void changePEAP(const char* ssid, const char* userid, const char* password) {
                    strncpy(peap_ssid_wpa2, ssid, sizeof(peap_ssid_wpa2) - 1);
                    strncpy(peap_username_wpa2, userid, sizeof(peap_username_wpa2) - 1);
                    // strncpy(peap_)
                    strncpy(peap_password_wpa2, password, sizeof(peap_password_wpa2) - 1);
                }
                /**
                 * @brief Connection WiFi by Selection type
                 * @param connection_type 0 for only scan, 1 for simple WPA2, 2 for peap
                 */
                void turn_on_WiFi(int connection_type=0, int try_time = 1) {
                    ESP_ERROR_CHECK(nvs_flash_erase());
                    ESP_ERROR_CHECK(nvs_flash_init());

                    s_retry_num = 0;
                    // SetForceWiFiOff = true;
                    // esp_wifi_stop();
                    esp_wifi_disconnect();
                    // SetForceWiFiOff = false;

                    if(STATE_initial == false ){
                        STATE_initial = true;
                        ESP_ERROR_CHECK(nvs_flash_init());
                        ESP_ERROR_CHECK(esp_netif_init());
                        wifi_event_group = xEventGroupCreate();
                        ESP_ERROR_CHECK(esp_event_loop_create_default());
                        esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
                        esp_netif_set_hostname(sta_netif, "IndoorAirSensorv1");
                        assert(sta_netif);
                    }


                    type = connection_type;
                    int try_counter = 0;

                    if (connection_type == 0) {
                        scanAP();
                        return;
                    } else if (connection_type == 1) {
                        if (scanAP_check(ssid_wpa2)) {
                            // while (!connect_wpa2() && try_counter < try_time) {
                            //     try_counter++;
                            // }
                            connect_wpa2();
                            printWiFiConfig();
                            // PingTest();
                            return;
                        }
                    } else if (connection_type == 2) {
                        if (scanAP_check(peap_ssid_wpa2)) {
                            // while (!connect_peap() && try_counter < try_time) {
                            //     try_counter++;
                            // }
                            connect_peap();
                            printWiFiConfig();
                            // PingTest();
                            return;
                        }
                    }
                }

                void turn_off_WiFi() {
                    SetForceWiFiOff = true;
                    esp_wifi_stop();
                }

                bool wifi_connect_status() {
                    esp_netif_ip_info_t ip;
                    memset(&ip, 0, sizeof(esp_netif_ip_info_t));
                    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
                    // vTaskDelay(2000 / portTICK_PERIOD_MS);
                    // Serial.println("Waiting for IP ...");
                    //check wifi is connect or not
                    esp_netif_get_ip_info(sta_netif, &ip);
                    if((uint16_t)(((const uint8_t*)(&(&ip.ip)->addr))[0])!=(uint16_t)0){
                        return true;
                    }    
                    return false;
                }

                uint8_t mac[6];
                void printWiFiConfig() {
                    esp_netif_ip_info_t ip;
                    memset(&ip, 0, sizeof(esp_netif_ip_info_t));
                    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
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
                            Serial.print("MAC Address: ");
                            Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

                        } else {
                            Serial.println("Failed to get IP information");
                        }
                    }else{
                        // Serial.println("No Connection");
                        ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));  // WIFI_IF_STA 用於 Wi-Fi STA 模式
                        Serial.print("MAC Address: ");
                        Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                        Serial.println("No Internet Connection");
                    }
                }
                uint8_t* getMACaddress() {
                    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));  // WIFI_IF_STA 用於 Wi-Fi STA 模式
                    return mac;
                }
                void connect_check() {
                    esp_netif_ip_info_t ip;
                    memset(&ip, 0, sizeof(esp_netif_ip_info_t));
                    esp_netif_t *sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
                    // vTaskDelay(2000 / portTICK_PERIOD_MS);
                    // Serial.println("Waiting for IP ...");
                    //check wifi is connect or not
                    esp_netif_get_ip_info(sta_netif, &ip);
                    while ((uint16_t)(((const uint8_t*)(&(&ip.ip)->addr))[0])!=(uint16_t)0) {
                        Serial.println("====[RESETING WIFI]====");
                        turn_on_WiFi(type, 1);
                    }
                }

                bool PingTest() {
                    IPAddress ips[] = {
                        IPAddress(8, 8, 8, 8),
                        IPAddress(168, 95, 1, 1),
                        // IPAddress(192, 168, 50, 1)
                    };
                    if(!wifi_connect_status()){
                      return false;
                    }
                    bool success = false;
                    for (const auto& ip : ips) {
                        success = Ping.ping(ip, 3);
                        if (success) {
                            Serial.println("Ping successful to " + ip.toString());
                        } else {
                            Serial.println("Ping failed to " + ip.toString());
                        }
                    }
                    return success;
                }
                void setCloseWiFi(bool signal){
                    auto_close_wifi = signal;
                }
                void obtain_time() {
                    const char* ntpServer = "time.stdtime.gov.tw";
                    const long gmtOffset_sec = 8 * 3600; // UTC+8
                    const int daylightOffset_sec = 0;

                    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

                    struct tm timeinfo;
                    if(!getLocalTime(&timeinfo)){
                        Serial.println("Failed to obtain time");
                    }
                    Serial.println(&timeinfo, "Current time: %Y-%m-%d %H:%M:%S");
                    return;
                }
        };

    // #endif
    //     #endif
    #endif
#endif
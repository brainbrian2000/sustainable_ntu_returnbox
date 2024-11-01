#ifndef _WIFI_CONNECT_CPP
#define _WIFI_CONNECT_CPP
    #if 1
        #include <WiFi.h>
        #include "esp_wifi.h"
        #include <esp_err.h>
        #include <esp_wpa.h>
        #include <esp_eap_client.h>
        #include <stdlib.h>
        #include <ESPping.h>
        #include <esp_netif_types.h>
        #include "esp_log.h"
        #include "esp_event.h"
        #include "esp_netif.h"
        #include "nvs_flash.h"
        #define PRINT_WIFI_LOG 1
        static EventGroupHandle_t wifi_event_group;
        static esp_netif_t *sta_netif = NULL;
        static const int CONNECTED_BIT = BIT0;
        static void wifi_inside_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
    {
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        }
    }

        // #include <dns.h>
        /** 
         * Default Using in NTU and Using eduroam or SCplusNTU 
         * wifi in IPCS G206 Room.
         * Also can set by function by setting ssid, username and passwd.
         *  
        */
        // WiFiClass Wifi;
        class WiFi_connection{
            private:
                bool connect_peap(){
                    // wifi_init_sta();
                    WiFi.disconnect(true);
                    WiFi.setHostname("ESP32_IndoorAirSensor");
                    
                    WiFi.mode(WIFI_STA);
                    // WiFi.begin(peap_ssid_wpa2,WPA2_AUTH_PEAP,peap_id_wpa2,peap_username_wpa2,peap_password_wpa2);
                    // WiFi.begin(peap_ssid_wpa2,WPA2_AUTH_PEAP,peap_username_wpa2,peap_password_wpa2);
                        // ESP_ERROR_CHECK(esp_netif_init());
                        // wifi_event_group = xEventGroupCreate();
                        // ESP_ERROR_CHECK(esp_event_loop_create_default());
                        // sta_netif = esp_netif_create_default_wifi_sta();
                        // assert(sta_netif);
                        // // ESP_ERROR_CHECK(esp_wifi_init());
                        // esp_wifi_set_mode(WIFI_MODE_STA);
                        // // ESP_ERROR_CHECK()
                        // wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
                        // ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
                        // ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_inside_event_handler, NULL));
                        // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_inside_event_handler, NULL));
                        // // ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
                        // ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
                        // wifi_config_t wifi_config = {
                        //     .sta = {
                        //         .ssid = peap_ssid_wpa2,
                        //     },
                        // };
                        // wifi_config_t config = WPA2_CONFIG_INIT_DEFAULT();
                        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                        // ESP_ERROR_CHECK(esp_wifi_deinit());
                        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
                        Serial.println("WiFi init");
                        // wifi_config.ssids = peap_ssid_wpa2;
                        // ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
                        // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
                        // ESP_ERROR_CHECK(esp_wifi_sta_enterprise_enable());
                        ESP_ERROR_CHECK( esp_eap_client_set_identity((uint8_t *)peap_id_wpa2, strlen(peap_id_wpa2)) );
                        ESP_ERROR_CHECK( esp_eap_client_set_username((uint8_t *)peap_username_wpa2, strlen(peap_username_wpa2)) );
                        ESP_ERROR_CHECK( esp_eap_client_set_password((uint8_t *)peap_password_wpa2, strlen(peap_password_wpa2)) );
                        Serial.println("Set identity, username, password");
                        // ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable(&config));
                        // esp_eap_fast_config eap_fast_config = {};
                        // eap_fast_config.fast_provisioning = 2;
                        // eap_fast_config.fast_max_pac_list_len = 0;
                        // eap_fast_config.fast_pac_format_binary = false;
                        // ESP_ERROR_CHECK(esp_eap_client_set_fast_params(eap_fast_config));
                        // ESP_ERROR_CHECK(esp_wifi_start());
                        // ESP_ERROR_CHECK(esp_wifi_connect());
                    WiFi.begin(peap_ssid_wpa2, WPA2_AUTH_PEAP, peap_id_wpa2, peap_username_wpa2, peap_password_wpa2);
                    // wifi_init_sta();
                    
                    // using arduino wifi lib to connect to peap


                    
                    int counter = 0;
                    bool return_status = false;
                    // esp_wifi_connect();
                    current_ssid = peap_ssid_wpa2;
                    Serial.println("Try connecting to wifi [peap]");
                    while (WiFi.status() != WL_CONNECTED && counter < 30) {
                        delay(1500);
        
                        Serial.println("Connecting "+String(current_ssid));
                        counter++;
                    }
                    if(WiFi.status() == WL_CONNECTED){
                        return_status = true;
                        Serial.println("");
                        Serial.println("WiFi connected");

                    }else{
                        return_status = false;
                        current_ssid = default_ssid;
                    }
                    return return_status;
                }
                bool connect_wpa2(){
                    WiFi.disconnect();
                    WiFi.setHostname("ESP32_IndoorAirSensor");
                    WiFi.mode(WIFI_STA);
                    // WiFi.config(IPAddress(192,168,1,102),IPAddress(192,168,1,1),IPAddress(255,255,255,0),IPAddress(8,8,8,8),IPAddress(8,8,8,8));
                    WiFi.begin(ssid_wpa2,password_wpa2);
                    int counter = 0;
                    bool return_status = false;
                    current_ssid = ssid_wpa2;
                    while (WiFi.status() != WL_CONNECTED && counter < 30) {
                        delay(1500);
                        Serial.println("Connecting "+String(current_ssid));
                        counter++;
                    }
                    if(WiFi.status() == WL_CONNECTED){
                        return_status = true;
                        Serial.println("");
                        Serial.println("WiFi connected");
                    }else{
                        return_status = false;
                        current_ssid = default_ssid;
                    }
                    return return_status;
                }
                void scanAP(void) {
                    int n = WiFi.scanNetworks();
                    delay(500);
                    Serial.println("scan Wi-Fi done");
                    if (n == 0)
                        Serial.println("no Wi-Fi networks found");
                    else
                    {
                        Serial.print(n);
                        Serial.println(" Wi-Fi networks found:");
                        for (int i = 0; i < n; ++i)
                        {
                        Serial.print(i + 1);
                        Serial.print(": ");
                        //印出SSID
                        Serial.print(WiFi.SSID(i));
                        Serial.print(" (");
                        //印出RSSI強度
                        Serial.print(WiFi.RSSI(i));
                        Serial.print(") ");
                        //印出加密模式
                        Serial.println(WiFi.encryptionType(i),HEX);
                        delay(10);
                        }
                    }
                }       
                bool scanAP_check(char * ssid) {
                    int n = WiFi.scanNetworks();
                    bool check = false;
                    delay(500);

                    Serial.println("scan Wi-Fi done");
                    if (n == 0)
                        Serial.println("no Wi-Fi networks found");
                    else
                    {
                        for (int i = 0; i < n; ++i)
                        {
                        if(WiFi.SSID(i).equals(String(ssid))){
                            check = true;
                        }
                        if(check){
                            Serial.println("Find WiFi name: "+String(ssid));
                            Serial.print("dB : "+String(WiFi.RSSI(i))+" Type: ");
                            Serial.println(WiFi.encryptionType(i),HEX);
                            return check;
                        }
                        }
                    }
                    return check;
                }     
                static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
                {
                    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
                        esp_wifi_connect();
                    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
                        esp_wifi_connect();
                        ESP_LOGI(TAG, "retry to connect to the AP");
                    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
                        // ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
                        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
                    }
                }

                void wifi_init_sta()
                {
                    // 初始化 WiFi 配置
                    esp_err_t ret = nvs_flash_init();
                    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                        ESP_ERROR_CHECK(nvs_flash_erase());
                        ret = nvs_flash_init();
                    }
                    ESP_ERROR_CHECK(ret);

                    ESP_ERROR_CHECK(esp_netif_init());
                    wifi_event_group = xEventGroupCreate();
                    // ESP_ERROR_CHECK(esp_event_loop_create_default());
                    // esp_netif_create_default_wifi_sta();
                    esp_netif_destroy(sta_netif);
                    sta_netif = NULL;
                    sta_netif = esp_netif_create_default_wifi_sta();
                    assert(sta_netif);

                    if (sta_netif == NULL) {
                        ESP_LOGE(TAG, "Failed to create default WiFi STA");
                        return;
                    }
                    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                    esp_wifi_init(&cfg);

                    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
                    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
                    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

                    wifi_config_t wifi_config = {};
                    // wifi_config.sta.ssid = peap_ssid_wpa2,               // WiFi SSID
                    memcpy(wifi_config.sta.ssid,peap_ssid_wpa2,strlen(peap_ssid_wpa2));
                    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_ENTERPRISE;  // 認證模式
                    esp_wifi_set_mode(WIFI_MODE_STA);
                    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

                    // ESP_ERROR_CHECK(esp_wpa2_init());
                    // 配置PEAP EAP參數
                    ESP_ERROR_CHECK( esp_eap_client_set_identity((uint8_t *)peap_id_wpa2, strlen(peap_id_wpa2)) );
                    ESP_ERROR_CHECK( esp_eap_client_set_username((uint8_t *)peap_username_wpa2, strlen(peap_username_wpa2)) );
                    ESP_ERROR_CHECK( esp_eap_client_set_password((uint8_t *)peap_password_wpa2, strlen(peap_password_wpa2)) );
                    ESP_ERROR_CHECK(esp_eap_client_set_ca_cert(NULL, 0));
                    ESP_ERROR_CHECK(esp_eap_client_set_certificate_and_key(NULL, 0, NULL, 0, NULL, 0));
                    // ESP_ERROR_CHECK(esp_wpa2_ent_enable());
                    
                    // esp_eap_fast_config eap_fast_config = {
                    //     .fast_provisioning = 2,
                    //     .fast_max_pac_list_len = 0,
                    //     .fast_pac_format_binary = false,
                    // };
                    // ESP_ERROR_CHECK(esp_eap_client_set_fast_params(eap_fast_config));
                    // 開啟 WPA2 企業模式
                    ESP_ERROR_CHECK(esp_eap_client_use_default_cert_bundle(true));
                    ESP_ERROR_CHECK(esp_wifi_sta_enterprise_enable());
                    ESP_ERROR_CHECK(esp_wifi_start());
                    // esp_wifi_start();
                }         
            
            public:
                WiFi_connection(){
                    ESP_ERROR_CHECK(nvs_flash_init());
                    ESP_ERROR_CHECK(esp_netif_init());
                    wifi_event_group = xEventGroupCreate();
                    ESP_ERROR_CHECK(esp_event_loop_create_default());
                };
                ~WiFi_connection(){};
                char peap_ssid_wpa2[32] = "eduroam";
                char peap_id_wpa2[32] = "b08209023@eduroam.ntu.edu.tw";
                char peap_username_wpa2[32] = "b08209023@eduroam.ntu.edu.tw";
                char peap_password_wpa2[32] = "Brianipcs206";
                // char peap_ssid_wpa2[32] = "ntu_peap";
                // char peap_id_wpa2[32] = "b08209023";
                // char peap_username_wpa2[32] = "b08209023";
                // char ssid_wpa2[31] = "ASAP_A1F";
                // char password_wpa2[32] = "0233663926";
                // char ssid_wpa2[31] = "SCplusNTU";
                // char password_wpa2[32] = "smartcampus206";
                char ssid_wpa2[31] = "Riceball_Fan";
                char password_wpa2[32] = "brainbrian2000";
                char default_ssid[32]="DISCONNECTED";
                char *current_ssid=default_ssid;
                int type=0;
                void changeWPA2(char const *ssid, char const *password){
                    strcpy(ssid_wpa2,ssid);
                    strcpy(password_wpa2,password);
                    return;
                };
                void changePEAP(char const *ssid, char const *userid,char const *username, char const *password){
                    strcpy(peap_username_wpa2,username);
                    strcpy(peap_id_wpa2,userid);
                    strcpy(peap_password_wpa2,password);
                    strcpy(peap_ssid_wpa2,ssid);
                    return;
                }

                bool wifi_connect_status(){
                    if(WiFi.status() == WL_CONNECTED){
                        return true;
                    }else{
                        return false;
                    }
                }

                void printWiFiConfig(){
                    #if PRINT_WIFI_LOG
                        Serial.print("Local IP     :");
                        Serial.println((WiFi.localIP()));
                        Serial.print("Mac Address  :");Serial.println(String(WiFi.macAddress()));
                        Serial.print("Subnet Mask  :");Serial.println(WiFi.subnetMask());
                        Serial.print("Gateway IP   :");Serial.println(WiFi.gatewayIP());
                        Serial.print("DNS IP       :");Serial.println(WiFi.dnsIP());
                        Serial.print("Broadcast IP :");Serial.println(WiFi.broadcastIP());
                        Serial.print("Subnet CIDR  :");Serial.println(WiFi.subnetCIDR());
                    #endif
                }
                /**
                 * @brief Connection WiFi by Selection type
                 * @param connection_type 0 for only scan,
                 *                        1 for simple WPA2, 
                 *                        2 for peap 
                 * 
                 */
                void turn_on_WiFi(int connection_type,int try_time=1){
                    // Serial.println(WiFi.config(WiFi.localIP(),WiFi.gatewayIP(),WiFi.subnetMask(),IPAddress(8,8,8,8),IPAddress(8,8,8,8)));
                    type = connection_type;
                    int try_counter = 0;
                    if(connection_type == 0){
                        scanAP();
                        return;
                    }else if (connection_type == 1){
                        if(scanAP_check(ssid_wpa2)){
                            while(!connect_wpa2()&&try_counter < try_time){
                                try_counter++;        
                            }
                            printWiFiConfig();
                            PingTest();
                            return;
                        }
                    }else if (connection_type == 2){
                        if(scanAP_check(peap_ssid_wpa2)){
                            while(!connect_peap()&& try_counter < try_time){
                                try_counter++;        
                            // }
                            printWiFiConfig();
                            PingTest();
                            return;
                            }
                        }
                    }
                }
                void turn_off_WiFi(){
                    type = 0;
                    WiFi.disconnect(true);
                    WiFi.mode(WIFI_OFF);
                }
                void connect_check(){
                    while (WiFi.status() != WL_CONNECTED) {
                        Serial.println("====[RESETING WIFI]====");
                        turn_on_WiFi(type,1);
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
                    // const char *timeserver = "time.stdtime.gov.tw";
                    // // const char *timeserver = "pool.ntp.org";
                    // success = Ping.ping(timeserver, 3);
                    // if(!success){
                    //     Serial.println("Ping failed");
                    //     Serial.print(timeserver);
                    //     Serial.println("");
                    // }else{
                    //     Serial.println("Ping succesful.");
                    //     Serial.print(timeserver);
                    //     Serial.println("");
                    // }

                    
                    return true;
                } 



        };
//         #ifndef _WIFI_CONNECT_CPP
// #define _WIFI_CONNECT_CPP
    #else

        // #include <WiFi.h>
        // #include <esp_wifi.h>
        // #include <esp_err.h>
        // #include <esp_event.h>
        // #include <esp_netif.h>
        // #include <esp_eap_client.h>
        // #include <nvs_flash.h>
        // #include <ESPping.h>
        // #include "esp_log.h"
        #include <WiFi.h>
        #include <esp_wifi.h>
        #include <esp_err.h>
        // #include <esp_wpa.h>
        #include <esp_eap_client.h>
        #include <stdlib.h>
        #include <ESPping.h>
        #include <esp_netif_types.h>
        #include "esp_log.h"
        #include "esp_event.h"
        #include "esp_netif.h"
        #include "nvs_flash.h"
        #define PRINT_WIFI_LOG 1

        class WiFi_connection {
        public:
            WiFi_connection() {
                // 初始化 NVS
                esp_err_t ret = nvs_flash_init();
                if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                    ESP_ERROR_CHECK(nvs_flash_erase());
                    ret = nvs_flash_init();
                }
                ESP_ERROR_CHECK(ret);

                // 初始化事件循環
                ESP_ERROR_CHECK(esp_event_loop_create_default());

                // 初始化網絡接口
                ESP_ERROR_CHECK(esp_netif_init());
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
                strncpy(peap_password_wpa2, password, sizeof(peap_password_wpa2) - 1);
            }

            void turn_on_WiFi(int connection_type, int try_time = 1) {
                type = connection_type;
                int try_counter = 0;

                if (connection_type == 0) {
                    scanAP();
                    return;
                } else if (connection_type == 1) {
                    if (scanAP_check(ssid_wpa2)) {
                        while (!connect_wpa2() && try_counter < try_time) {
                            try_counter++;
                        }
                        printWiFiConfig();
                        PingTest();
                        return;
                    }
                } else if (connection_type == 2) {
                    if (scanAP_check(peap_ssid_wpa2)) {
                        while (!connect_peap() && try_counter < try_time) {
                            try_counter++;
                        }
                        printWiFiConfig();
                        PingTest();
                        return;
                    }
                }
            }

            void turn_off_WiFi() {
                type = 0;
                WiFi.disconnect(true);
                WiFi.mode(WIFI_OFF);
            }

            bool wifi_connect_status() {
                return WiFi.status() == WL_CONNECTED;
            }

            void printWiFiConfig() {
                #if PRINT_WIFI_LOG
                    Serial.print("Local IP     : ");
                    Serial.println(WiFi.localIP());
                    Serial.print("Mac Address  : ");
                    Serial.println(WiFi.macAddress());
                    Serial.print("Subnet Mask  : ");
                    Serial.println(WiFi.subnetMask());
                    Serial.print("Gateway IP   : ");
                    Serial.println(WiFi.gatewayIP());
                    Serial.print("DNS IP       : ");
                    Serial.println(WiFi.dnsIP());
                #endif
            }

            void connect_check() {
                while (WiFi.status() != WL_CONNECTED) {
                    Serial.println("====[RESETING WIFI]====");
                    turn_on_WiFi(type, 1);
                }
            }

            bool PingTest() {
                IPAddress ips[] = {
                    IPAddress(8, 8, 8, 8),
                    IPAddress(168, 95, 1, 1)
                };

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

        private:
            char peap_ssid_wpa2[32] = "YOUR_PEAP_SSID";          // 替換為您的 PEAP SSID
            char peap_username_wpa2[64] = "YOUR_PEAP_USERNAME";   // 替換為您的 PEAP 用戶名
            char peap_password_wpa2[64] = "YOUR_PEAP_PASSWORD";   // 替換為您的 PEAP 密碼

            char ssid_wpa2[32] = "YOUR_WPA2_SSID";               // 替換為您的 WPA2 SSID
            char password_wpa2[64] = "YOUR_WPA2_PASSWORD";        // 替換為您的 WPA2 密碼

            char default_ssid[32] = "DISCONNECTED";
            char* current_ssid = default_ssid;
            int type = 0;

            bool connect_wpa2() {
                WiFi.disconnect();
                WiFi.mode(WIFI_STA);
                WiFi.begin(ssid_wpa2, password_wpa2);
                current_ssid = ssid_wpa2;

                int counter = 0;
                Serial.println("Trying to connect to WPA2 WiFi...");
                while (WiFi.status() != WL_CONNECTED && counter < 30) {
                    delay(1000);
                    Serial.print(".");
                    counter++;
                }

                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println("\nWiFi connected");
                    return true;
                } else {
                    Serial.println("\nFailed to connect to WiFi");
                    current_ssid = default_ssid;
                    return false;
                }
            }

            bool connect_peap() {
                WiFi.disconnect();
                WiFi.mode(WIFI_STA);

                esp_netif_t* netif = esp_netif_create_default_wifi_sta();
                assert(netif);

                wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
                ESP_ERROR_CHECK(esp_wifi_init(&cfg));

                ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
                ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));

                wifi_config_t wifi_config = {};
                strncpy((char*)wifi_config.sta.ssid, peap_ssid_wpa2, sizeof(wifi_config.sta.ssid));

                ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
                ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

                // 配置 WPA2 Enterprise PEAP 參數
                ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_identity((uint8_t*)peap_username_wpa2, strlen(peap_username_wpa2)));
                ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_username((uint8_t*)peap_username_wpa2, strlen(peap_username_wpa2)));
                ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_password((uint8_t*)peap_password_wpa2, strlen(peap_password_wpa2)));
                ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_enable());

                ESP_ERROR_CHECK(esp_wifi_start());

                current_ssid = peap_ssid_wpa2;

                int counter = 0;
                Serial.println("Trying to connect to PEAP WiFi...");
                while (WiFi.status() != WL_CONNECTED && counter < 30) {
                    delay(1000);
                    Serial.print(".");
                    counter++;
                }

                if (WiFi.status() == WL_CONNECTED) {
                    Serial.println("\nWiFi connected");
                    return true;
                } else {
                    Serial.println("\nFailed to connect to WiFi");
                    current_ssid = default_ssid;
                    return false;
                }
            }

            void scanAP() {
                int n = WiFi.scanNetworks();
                delay(500);
                Serial.println("Scan Wi-Fi done");
                if (n == 0) {
                    Serial.println("No Wi-Fi networks found");
                } else {
                    Serial.print(n);
                    Serial.println(" Wi-Fi networks found:");
                    for (int i = 0; i < n; ++i) {
                        Serial.print(i + 1);
                        Serial.print(": ");
                        Serial.print(WiFi.SSID(i));
                        Serial.print(" (");
                        Serial.print(WiFi.RSSI(i));
                        Serial.print(") ");
                        Serial.println(WiFi.encryptionType(i), HEX);
                        delay(10);
                    }
                }
            }

            bool scanAP_check(const char* ssid) {
                int n = WiFi.scanNetworks();
                delay(500);
                Serial.println("Scan Wi-Fi done");
                if (n == 0) {
                    Serial.println("No Wi-Fi networks found");
                    return false;
                } else {
                    for (int i = 0; i < n; ++i) {
                        if (WiFi.SSID(i).equals(String(ssid))) {
                            Serial.println("Found WiFi: " + String(ssid));
                            return true;
                        }
                    }
                    return false;
                }
            }

            static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
                if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
                    esp_wifi_connect();
                } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
                    esp_wifi_connect();
                    Serial.println("Retrying to connect to the AP");
                } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
                    ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
                    Serial.print("Got IP: ");
                    Serial.println(IPAddress(event->ip_info.ip.addr));
                }
            }
        };

    #endif
    //     #endif
    // #endif
#endif
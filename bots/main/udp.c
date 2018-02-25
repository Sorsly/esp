#include <string.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include "udp.h"


const int CONNECTED_BIT = BIT0;
const int STARTED_BIT = BIT1;


static const char *TAG = "MAIN";
ip4_addr_t ip;
char ip_str[30];
bool connected_to_ap = false;

void send_thread(resp rsp,commands cmd) {

    int socket_fd;
    struct sockaddr_in sa;

    int sent_data; char data_buffer[80];

    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if ( socket_fd < 0 ) {
        printf("socket call failed");
        exit(0);

    }

    memset(&sa, 0, sizeof(struct sockaddr_in));
    inet_pton(AF_INET, CCCPIP, &(sa.sin_addr.s_addr));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(cmd.portAssign);
    strcpy(data_buffer,"BOT STATE");

    ESP_LOGI(TAG,"SENDING");
    ESP_LOGI(TAG,"Received packet from %s:%d\n", inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
    sent_data = sendto(socket_fd, data_buffer,sizeof("Hello World"),0,(struct sockaddr*)&sa,sizeof(sa));
    if(sent_data < 0){
	    printf("send failed\n");
	    close(socket_fd);
	    exit(2);
    }

    close(socket_fd);
}

commands parsecommands(char * raw){
    commands cmd;
    cmd.sheepf = raw[0];
    cmd.duty_cycle1= raw[1];
    cmd.tOn1= raw[2];
    cmd.duty_cycle2= raw[3];
    cmd.tOn2= raw[4];
    cmd.servoAngle= raw[5];
    cmd.portAssign= (uint16_t)(raw[6] | (raw[7] << 8));
    return cmd;
}
commands receive_thread() {

    int socket_fd;
    struct sockaddr_in sa,ra;

    int recv_data; char data_buffer[80];

    socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if ( socket_fd < 0 )
    {

        printf("socket call failed");
        exit(0);

    }

    memset(&sa, 0, sizeof(struct sockaddr_in));
    ra.sin_family = AF_INET;
    ra.sin_addr.s_addr = htonl(INADDR_ANY);
    ra.sin_port = htons(RECEIVER_PORT_NUM);

    ESP_LOGD(TAG,"SOCKET PREPARED");

    if (bind(socket_fd, (struct sockaddr *)&ra, sizeof(struct sockaddr_in)) == -1){
    close(socket_fd);
    exit(1);
    }
    ESP_LOGD(TAG,"RECIVEING SOCKET READY");

    recv_data = recv(socket_fd,data_buffer,sizeof(data_buffer),0);
    if(recv_data > 0)
    {

        data_buffer[recv_data] = '\0';

    }
    close(socket_fd);
    return parsecommands(data_buffer);

}



static esp_err_t cust_wifi_event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_START");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_GOT_IP");
            ESP_LOGI(TAG, "********************************************");
            ESP_LOGI(TAG, "* We are now connected to AP")
            ESP_LOGI(TAG, "* - Our IP address is: %s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            ESP_LOGI(TAG, "********************************************");
            ip = event->event_info.got_ip.ip_info.ip;
            inet_ntop(AF_INET,&ip,ip_str,30);
            connected_to_ap = true;
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            ESP_LOGI(TAG, "SYSTEM_EVENT_STA_DISCONNECTED");
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        default:
            break;
    }
    return ESP_OK;
}


void init_wifi(void)
{
    tcpip_adapter_init();
    ESP_ERROR_CHECK( esp_event_loop_init(cust_wifi_event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .scan_method = WIFI_FAST_SCAN,
            .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
            .threshold.rssi = CONFIG_FAST_SCAN_MINIMUM_SIGNAL,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );

}




resp move(commands cmd){
    resp accumulatedstate;
	ESP_LOGI(TAG,"DOING THINGS TO ACHIEVE DESIRED STATE");
    ESP_LOGI(TAG,"DYCLE1 %04X",cmd.sheepf);
    return accumulatedstate;
}


void app_main() {

    commands nextCommands;
    resp state;
    //init nvs_flash
    esp_err_t nvsret = nvs_flash_init();

    if (nvsret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        nvsret = nvs_flash_init();
    }
    init_wifi();
    while(!connected_to_ap){}
    while(true){
            nextCommands = receive_thread();
            state = move(nextCommands);
            send_thread(state,nextCommands);
    }
}

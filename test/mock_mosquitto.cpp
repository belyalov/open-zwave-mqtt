
#include <mosquitto.h>

int
mosquitto_subscribe(struct mosquitto *mosq, int *mid, const char *sub, int qos)
{
    return 0;
}

int
mosquitto_publish(struct mosquitto *mosq, int *mid, const char* topic,
        int payloadlen, const void* payload, int qos, bool retain)
{
    return 0;
}

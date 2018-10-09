
#include "mock_mosquitto.h"

using namespace std;

vector<string> subscribe_history;
vector<pair<string, string> > publish_history;

int
mosquitto_lib_init()
{
    return 0;
}

struct mosquitto*
mosquitto_new(const char *id, bool clean_session, void *userdata)
{
    return (struct mosquitto*)&subscribe_history;
}

int
mosquitto_connect(struct mosquitto *mosq, const char *host, int port, int keepalive)
{
    return 0;
}

int
mosquitto_username_pw_set(struct mosquitto *mosq, const char *user, const char *passwd)
{
    return 0;
}

void mosquitto_message_callback_set(struct mosquitto *mosq,
        void (*on_message)(struct mosquitto *, void *, const struct mosquitto_message *))
{
}

int mosquitto_reconnect(struct mosquitto *mosq)
{
    return 0;
}

int
mosquitto_loop(struct mosquitto *mosq, int timeout, int max_packets)
{
    return 0;
}

int
mosquitto_subscribe(struct mosquitto *mosq, int *mid, const char *sub, int qos)
{
    subscribe_history.push_back(sub);

    return 0;
}

int
mosquitto_publish(struct mosquitto *mosq, int *mid, const char* topic,
        int payloadlen, const void* payload, int qos, bool retain)
{
    publish_history.push_back(make_pair(topic, string((const char*)payload, payloadlen)));

    return 0;
}

const vector<string>
mock_mosquitto_subscribe_history()
{
    return subscribe_history;
}

const vector<pair<string, string> >
mock_mosquitto_publish_history()
{
    return publish_history;
}

void
mock_mosquitto_cleanup()
{
    subscribe_history.clear();
    publish_history.clear();
}

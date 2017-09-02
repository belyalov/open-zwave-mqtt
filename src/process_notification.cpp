
#include <openzwave/Manager.h>
#include "process_notification.h"
#include "node_value.h"
#include "mqtt.h"
#include "options.h"

using namespace OpenZWave;

uint32_t home_id;
bool publishing = false;

void
process_notification(const Notification* n, void* ctx)
{
    options *opts = static_cast<options*> (ctx);

    switch(n->GetType()) {
        case Notification::Type_DriverReady:
        {
            printf("HomeID is %x\n", n->GetHomeId());
            break;
        }

        case Notification::Type_DriverFailed:
        {
            printf("Failed to initialize OpenZWave.\n");
            exit(1);
            break;
        }

        case Notification::Type_NodeAdded:
        {
            node_add(n->GetHomeId(), n->GetNodeId());
            break;
        }

        case Notification::Type_NodeRemoved:
        {
            node_remove_by_id(n->GetNodeId());
            break;
        }

        case Notification::Type_ValueAdded:
        {
            value_add(n->GetValueID());
            mqtt_subscribe(opts->mqtt_prefix, n->GetValueID());
            break;
        }

        case Notification::Type_ValueRemoved:
        {
            value_remove(n->GetValueID());
            break;
        }

        case Notification::Type_ValueRefreshed:
        {
            // We don't care about the same values ("refresh") today.
            break;
        }

        case Notification::Type_ValueChanged:
        {
            if (publishing) {
                mqtt_publish(opts->mqtt_prefix, n->GetValueID());
            }
            break;
        }

        case Notification::Type_NodeEvent:
        {
            // Should we take care of this one?
            break;
        }

        case Notification::Type_NodeQueriesComplete:
        {
            printf("Driver ready. Start publishing changed values to MQTT\n");
            publishing = true;
            break;
        }

        case Notification::Type_PollingEnabled:
        {
            // Log here
            break;
        }

        case Notification::Type_PollingDisabled:
        {
            // As well as here
            break;
        }

        default:
            break;
    }
}


#include <openzwave/Manager.h>
#include "process_notification.h"
#include "node_value.h"
#include "mqtt.h"

using namespace OpenZWave;

void
process_notification(const Notification* n, void* ctx)
{
    // Node1 is always controller, so, ignore it
    // if (n->GetNodeId() == 1) {
    //     return;
    // }

    switch(n->GetType()) {
        case Notification::Type_DriverReady:
        {
            // home_id = n->GetHomeId();
            // printf("HomeID is %x\n", home_id);
            break;
        }

        case Notification::Type_DriverFailed:
        {
            printf("Failed to initialize driver.\n");
            exit(1);
            break;
        }

        case Notification::Type_NodeAdded:
        {
            // printf("nid=%d hid=%x\n", n->GetNodeId(), n->GetHomeId());
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
            // value_add(n->GetValueID());
            // mqtt_subscribe("", n->GetValueID());
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
            mqtt_publish("", n->GetValueID());
            break;
        }

        case Notification::Type_NodeEvent:
        {
            // Should we take care of this one?
            break;
        }

        case Notification::Type_NodeQueriesComplete:
        {
            // Log some message?
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

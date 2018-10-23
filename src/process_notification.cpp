
#include <openzwave/Manager.h>
#include <openzwave/platform/Log.h>
#include "process_notification.h"
#include "node_value.h"
#include "mqtt.h"
#include "polling.h"
#include "options.h"

using namespace OpenZWave;

uint32_t home_id = 0;
bool publishing = false;

void
process_notification(const Notification* n, void* ctx)
{
    uint32_t hid = n->GetHomeId();
    uint8_t nid = n->GetNodeId();
    options *opts = static_cast<options*> (ctx);

    switch(n->GetType()) {
        case Notification::Type_DriverReady:
        {
            break;
        }

        case Notification::Type_DriverFailed:
        {
            exit(1);
            break;
        }

        case Notification::Type_NodeAdded:
        {
            node_add(hid, nid);
            break;
        }

        case Notification::Type_NodeRemoved:
        {
            node_remove_by_id(nid);
            break;
        }

        case Notification::Type_ValueAdded:
        {
            value_add(n->GetValueID());
            mqtt_subscribe(opts, n->GetValueID());
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
                // check / disable value polling
                polling_disable(n->GetValueID());
                mqtt_publish(opts, n->GetValueID());
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
            Log::Write(LogLevel_Info, nid, "Driver ready. Start publishing changed values to MQTT");
            home_id = hid;
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

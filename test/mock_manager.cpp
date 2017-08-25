
#include <set>
#include <openzwave/Manager.h>
#include "mock_manager.h"

using namespace OpenZWave;

Manager* Manager::s_instance = NULL;
map<uint8_t, string> node_new_names;
set<uint64_t> readonly_values;
map<uint64_t, string> value_labels;

// Blank constructor / destructor
Manager::Manager(): m_notificationMutex((Mutex*) new int)
{
}

Manager::~Manager()
{
}

// Create singletone manager
Manager*
Manager::Create()
{
    if (s_instance == NULL) {
        s_instance = new Manager();
    }
    return s_instance;
}

void
Manager::Destroy()
{
    if (s_instance != NULL) {
        delete s_instance;
        s_instance = NULL;
    }
}

 // Node info mocks
void
Manager::SetNodeName(const uint32_t hid, const uint8_t nid, const string& name)
{
    node_new_names[nid] = name;
}

string
Manager::GetNodeName(const uint32_t hid, const uint8_t nid)
{
    if (node_new_names.find(nid) != node_new_names.end()) {
        return node_new_names[nid];
    }

    return "name_h" + to_string(hid) + "_n" + to_string(nid);
}

string
Manager::GetNodeLocation(const uint32_t hid, const uint8_t nid)
{
    return "location_h" + to_string(hid) + "_n" + to_string(nid);
}

string
Manager::GetNodeManufacturerId(const uint32_t hid, const uint8_t nid)
{
    return "manufacturer_id_h" + to_string(hid) + "_n" + to_string(nid);
}

string
Manager::GetNodeProductId(const uint32_t hid, const uint8_t nid)
{
    return "product_id_h" + to_string(hid) + "_n" + to_string(nid);
}

string
Manager::GetNodeProductType(const uint32_t hid, const uint8_t nid)
{
    return "product_type_h" + to_string(hid) + "_n" + to_string(nid);
}

string
Manager::GetValueLabel(const OpenZWave::ValueID& v)
{
    if (value_labels.find(v.GetId()) != value_labels.end()) {
        return value_labels[v.GetId()];
    }
    return "label" + to_string(v.GetIndex());
}

bool
Manager::IsValueReadOnly(const OpenZWave::ValueID& v)
{
    return readonly_values.find(v.GetId()) != readonly_values.end();
}

bool
Manager::GetValueAsString(const ValueID& _id, string* o_value)
{
    o_value->assign("value" + to_string(_id.GetId()));

    return true;
}

void
mock_manager_set_value_readonly(const OpenZWave::ValueID& v)
{
    readonly_values.insert(v.GetId());
}

void
mock_manager_set_value_label(const OpenZWave::ValueID& v, const string& label)
{
    value_labels[v.GetId()] = label;
}

void
mock_manager_cleanup()
{
    node_new_names.clear();
    readonly_values.clear();
    value_labels.clear();

    Manager::Destroy();
}

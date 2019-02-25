
#include <algorithm>
#include <openzwave/Manager.h>
#include "node_value.h"
#include "command_classes.h"

using namespace std;

map<uint8_t, shared_ptr<node> > nodes_by_id;

// Nodes //

std::shared_ptr<const node>
node_add(const uint32_t home_id, const uint8_t node_id)
{
    // Check for already existing nodes
    if (nodes_by_id.find(node_id) != nodes_by_id.end()) {
        // LOG: warning, node already exists
        return nodes_by_id[node_id];
    }

    OpenZWave::Manager *mng = OpenZWave::Manager::Get();

    shared_ptr<node> n(new node(home_id, node_id));
    n->name = mng->GetNodeName(home_id, n->id);
    if (n->name == "") {
        // Unnamed node - rename it to node{ID}
        n->name = "node" + to_string(n->id);
        mng->SetNodeName(home_id, n->id, n->name);
    }
    n->location = mng->GetNodeLocation(home_id, n->id);
    n->manufacturer_id = mng->GetNodeManufacturerId(home_id, n->id);
    n->product_type = mng->GetNodeProductType(home_id, n->id);
    n->product_id = mng->GetNodeProductId(home_id, n->id);
    nodes_by_id[n->id] = n;

    return n;
}

std::shared_ptr<const node>
node_find_by_id(const uint8_t id)
{
    auto it = nodes_by_id.find(id);

    if (it != nodes_by_id.end()) {
        return it->second;
    } else {
        return NULL;
    }
}

void
node_remove_by_id(const uint8_t id)
{
    auto it = nodes_by_id.find(id);

    if (it != nodes_by_id.end()) {
        nodes_by_id.erase(id);
    }
}

void
node_remove_all()
{
    nodes_by_id.clear();
}

vector<shared_ptr<const node> >
node_get_all()
{
    vector<shared_ptr<const node> > ret;

    for (auto i = nodes_by_id.begin(); i != nodes_by_id.end(); ++i) {
        ret.push_back(i->second);
    }

    return ret;
}

// Values //

void
value_add(const OpenZWave::ValueID& v)
{
    auto it = nodes_by_id.find(v.GetNodeId());

    if (it == nodes_by_id.end()) {
        throw invalid_argument("Node not found");
    }

    it->second->values.push_back(v);
}

void
value_remove(const OpenZWave::ValueID& v)
{
    auto it = nodes_by_id.find(v.GetNodeId());

    if (it == nodes_by_id.end()) {
        throw invalid_argument("Node not found");
    }

    auto& vals = it->second->values;
    for (auto i = vals.begin(); i != vals.end(); ) {
        if (*i == v) {
            i = vals.erase(i);
        } else {
            ++i;
        }
    }
}

string
value_escape_label(const string& lbl)
{
    string res = lbl;

    transform(res.begin(), res.end(), res.begin(), ::tolower);
    std::replace(res.begin(), res.end(), ' ', '_');
    std::replace(res.begin(), res.end(), '/', '_');
    std::replace(res.begin(), res.end(), '+', '_');

    return res;
}

// Make string from OpenZwave value
pair<string, string>
value_make_paths(const string& prefix, const OpenZWave::ValueID& v)
{
    auto n = node_find_by_id(v.GetNodeId());
    if (!n) {
        throw invalid_argument("Node not found");
    }

    uint8_t cmd_class = v.GetCommandClassId();

    // prefix/node_location/node_name/command_class_name
    // prefix/node_id/command_class_id
    string name_path;
    string id_path;

    if (!prefix.empty()) {
        name_path += prefix + "/";
        id_path += prefix + "/";
    }
    if (!n->location.empty()) {
        name_path += n->location + "/";
    }
    name_path += n->name + "/" + command_class_str(cmd_class);
    id_path += to_string(n->id) + "/" + to_string(cmd_class);

    // Several command types support multi instances (e.g. 2-relay binary switch), so, add instance as well
    // Today we've found only 2 types that support it: SWITCH_MULTILEVEL and SWITCH_BINARY
    if (cmd_class == 0x25 || cmd_class == 0x26) {
        name_path += "/" + to_string(v.GetInstance());
        id_path += "/" + to_string(v.GetInstance());
    }
    name_path += "/" + value_escape_label(OpenZWave::Manager::Get()->GetValueLabel(v));
    id_path += "/" + to_string(v.GetIndex());

    return make_pair(name_path, id_path);
}

void
print_all_nodes()
{
    printf("\n");
    for (auto n = nodes_by_id.begin(); n != nodes_by_id.end(); ++n) {
        for (auto v = n->second->values.begin(); v != n->second->values.end(); v++) {
            string pref = "R";
            if (!OpenZWave::Manager::Get()->IsValueReadOnly(*v)) {
                pref += "/W";
            }
            auto names = value_make_paths("", *v);
            printf("(%s) %s (%s)\n", pref.c_str(), names.first.c_str(), names.second.c_str());
        }
    }

}

#ifndef NODE_VALUE_H
#define NODE_VALUE_H

#include <string>
#include <openzwave/Manager.h>


struct node {
    node(uint8_t _hid, uint32_t _id): home_id(_hid), id(_id) {};

    const uint32_t home_id;
    const uint8_t  id;

    std::string    name;
    std::string    location;
    std::string    manufacturer_id;
    std::string    product_type;
    std::string    product_id;

    std::vector<OpenZWave::ValueID> values;
};

std::shared_ptr<const node> node_add(const uint8_t node_id, const uint32_t home_id);
std::shared_ptr<const node> node_find_by_id(const uint8_t id);
void                        node_remove_by_id(const uint8_t id);
void                        node_remove_all();

std::vector<std::shared_ptr<const node> > node_get_all();

void                        value_add(const OpenZWave::ValueID& v);
void                        value_remove(const OpenZWave::ValueID& v);

std::string                 value_escape_label(const std::string&);

#endif

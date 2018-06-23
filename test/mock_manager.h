
#ifndef MOCK_MANAGER_H
#define MOCK_MANAGER_H

#include <openzwave/Manager.h>

void mock_manager_set_value_readonly(const OpenZWave::ValueID&);
void mock_manager_set_value_label(const OpenZWave::ValueID& v, const std::string& label);
const std::vector<std::pair<uint64_t, std::string> >
     mock_manager_get_value_set_history();
bool mock_manager_get_polling_state(const OpenZWave::ValueID& _id);

void mock_manager_cleanup();

#endif
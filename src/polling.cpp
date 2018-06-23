
#include <map>
#include <openzwave/Manager.h>
#include <openzwave/platform/Log.h>
#include "polling.h"

using namespace std;
using namespace OpenZWave;

map<ValueID, uint32_t> polling_values;


void polling_enable(const ValueID& v, uint32_t poll_count)
{
    auto it = polling_values.find(v);

    if (it == polling_values.end()) {
        // New item
        Manager::Get()->EnablePoll(v);
        Manager::Get()->SetPollIntensity(v, 3);
        polling_values[v] = 0;
        it = polling_values.find(v);
    }
    // Update poll count
    it->second = poll_count;
}

void polling_disable(const ValueID& v)
{
    auto it = polling_values.find(v);
    if (it == polling_values.end()) {
        // no such item
        return;
    }

    // Decrease counter and disable polling if needed
    it->second--;
    if (it->second == 0) {
        Manager::Get()->DisablePoll(it->first);
        polling_values.erase(it);
    }
}

// Unittests only
void polling_disable_all()
{
    polling_values.clear();
}

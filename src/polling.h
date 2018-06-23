
#ifndef POLLING_H
#define POLLING_H

#include <string>
#include <memory>
#include <openzwave/Manager.h>

// Enables polling on given value for at most poll_count
void polling_enable(const OpenZWave::ValueID& v, uint32_t poll_count);

// Check / disable polling on given value if it has been polled at least poll_count
void polling_disable(const OpenZWave::ValueID& v);

// unittests only
void polling_disable_all();

#endif


#include "ebus/event_bus.hpp"

// Declare the static instance since this can't be done in the header file
ebus::EventBus* ebus::EventBus::instance = nullptr;


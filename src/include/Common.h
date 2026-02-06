#pragma once
#include <cstdint>

using Key = int32_t;

struct RID {
    uint32_t pageID;
    uint16_t slotID;
};

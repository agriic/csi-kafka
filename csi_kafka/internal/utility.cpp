
#include "utility.h"

namespace csi
{
    namespace kafka
    {
        uint64_t change_endianess(uint64_t value)
        {
            uint64_t changed;
            uint8_t *data = (uint8_t *)&changed;

            data[0] = value >> 56;
            data[1] = value >> 48;
            data[2] = value >> 40;
            data[3] = value >> 32;
            data[4] = value >> 24;
            data[5] = value >> 16;
            data[6] = value >> 8;
            data[7] = value >> 0;
            
            return changed;
        }
    }
}


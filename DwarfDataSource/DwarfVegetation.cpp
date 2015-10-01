#ifdef BUILD_DLL

#include "DwarfFortress.h"
#include "DwarfVegetation.h"

namespace DwarfFortress {
    dword Vegetation::getGrowthTicksOffset(int32 x, int32 y) { // as of 40.10
        // it IS (y + 7) * y in the binary. Don't know if it's a typo by Toady or if it's intended.
        int32 seed = ((y + 3) * x + 5) * ((y + 7) * y + 100);
        for (dword i = 6; --i;) { // 5 times
            seed = (seed * 0xDD9CD + 0x5F59201) & 0x3FFFFFFF;
        }
        return ((dword) seed) % 2000;
    }

    dword Vegetation::getGrowthTicks(int32 x, int32 y) {
        return (df.ticks + getGrowthTicksOffset(x, y)) % ticks_per_year;
    }

    dword Vegetation::getGrowthTicks() {
        return getGrowthTicks(location.x, location.y);
    }
}

#endif

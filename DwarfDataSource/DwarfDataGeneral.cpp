#ifdef BUILD_DLL

#include "DwarfDataGeneral.h"

namespace DwarfFortress {
    String::operator char const *() const {
        return data();
    }

    char const * String::data() const {
        if (this == nullptr) {
            return "";
        } else if (len < 16) {
            return adata;
        } else {
            return pdata;
        }
    }

    dword String::length() const {
        if (this == nullptr) {
            return 0;
        } else {
            return len;
        }
    }
}

#endif

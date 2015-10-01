#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_DATA_GENERAL_H__
#define __DORFY_VISION__DWARF_DATA_GENERAL_H__

#include <functional>
#include "Types.h"

// make __LINE__ correctly expands to line number, rather than being pasted literally after _##addr##_
// I don't know why we need two macros for this, but it doesn't work with only one
#define ___concat(a, b, c) a##b##c
#define __concat(a, b, c) ___concat(a, b, c)

#define __at(addr, ...)                                             \
    struct {                                                        \
        byte __concat(z_dummy_##addr##_, __LINE__, _)[0x##addr];    \
        __VA_ARGS__;                                                \
    }

int32 constexpr invalid_location = -30000;
word constexpr invalid_temperature = 60001;
dword constexpr season_time_per_day = 120;
dword constexpr ticks_per_day = 1200;
dword constexpr days_per_month = 28;
dword constexpr months_per_season = 3;
dword constexpr seasons_per_year = 4;

dword constexpr tick_0_month = 3; // march
dword constexpr season_time_per_month = season_time_per_day * days_per_month;
dword constexpr ticks_per_month = ticks_per_day * days_per_month;
dword constexpr days_per_season = days_per_month * months_per_season;
dword constexpr months_per_year = months_per_season * seasons_per_year;

dword constexpr ticks_per_year = ticks_per_month * months_per_year;

namespace DwarfFortress {
    struct VTable {}; // placeholder

    template <typename T>
    struct Vector3 {
        T x, y, z;
    };

    struct String {
        union {
            char adata[16];
            char * pdata;
        };
        dword len, cap, unknown;

        operator char const *() const;
        char const * data() const;
        dword length() const;
    };

    template <typename T>
    struct Set {
        static_assert(sizeof(T) <= 4, "Forgot to use pointer?");
    private:
        T * head, * tail, * cap;
    public:
        typedef T const * Iterator;

        struct ReverseIterator {
            T * v;

            inline ReverseIterator(T * v) : v(v) {}

            inline ReverseIterator operator ++() {
                return --v;
            }

            inline bool operator <(ReverseIterator other) const {
                return v > other.v;
            }

            inline T operator *() const {
                return *v;
            }
        };

        inline bool empty() const {
            return size() == 0;
        }

        inline dword size() const {
            if (head < tail) {
                return ((dword) tail - (dword) head) / sizeof(T);
            } else {
                return 0;
            }
        }

        inline Iterator begin() const {
            return head;
        }

        inline Iterator end() const {
            return tail;
        }

        inline ReverseIterator rbegin() const {
//            if (head < tail) {
                return (T *) (((ptrword) tail) - sizeof(T)); // if use tail - 1, compiler will compile weird useless extra binary code
//            } else {
//                return 0;
//            }
        }

        inline ReverseIterator rend() const {
//            if (head < tail) {
                return (T *) (((ptrword) head) - sizeof(T)); // if use head - 1, compiler will compile weird useless extra binary code
//            } else {
//                return ((T *) 0) - 1;
//            }
        }

        T find(std::function<int(T const &)> comp) const {
            T * left = head, * right = tail, * center;
            while (left < right) {
                center = left + ((dword) right - (dword) left) / (sizeof(T) * 2);
                int cmp = comp(*center);
                if (cmp < 0) { // target < center
                    right = center;
                } else if (cmp > 0) { // target > center
                    left = center + 1;
                } else { // target == center
                    return *center;
                }
            }
            return 0;
        }

        inline T operator [](int32 v) const {
            if (v >= 0 && v < size()) {
                return *(head + v);
            } else {
                return 0;
            }
        }
    };
};

#endif

#endif

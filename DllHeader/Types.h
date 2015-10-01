#ifdef BUILD_DLL

#ifndef __DORFY_VISION__TYPES_H__
#define __DORFY_VISION__TYPES_H__

namespace __type_h_deprecated {
    template <int t> struct __select_signed_type;
    template <> struct __select_signed_type<1> {typedef signed char      type;};
    template <> struct __select_signed_type<2> {typedef signed short     type;};
    template <> struct __select_signed_type<3> {typedef signed int       type;};
    template <> struct __select_signed_type<4> {typedef signed long      type;};
    template <> struct __select_signed_type<5> {typedef signed long long type;};

    template <int t> struct __select_unsigned_type;
    template <> struct __select_unsigned_type<1> {typedef unsigned char      type;};
    template <> struct __select_unsigned_type<2> {typedef unsigned short     type;};
    template <> struct __select_unsigned_type<3> {typedef unsigned int       type;};
    template <> struct __select_unsigned_type<4> {typedef unsigned long      type;};
    template <> struct __select_unsigned_type<5> {typedef unsigned long long type;};

    template <int t> struct __select_float_type;
    template <> struct __select_float_type<1> {typedef float  type;};
    template <> struct __select_float_type<2> {typedef double type;};

    template <int size>
    struct __signed_type_of_size {
    private:
        static int constexpr index =
                sizeof(signed char     ) == size ? 1 :
                sizeof(signed short    ) == size ? 2 :
                sizeof(signed int      ) == size ? 3 :
                sizeof(signed long     ) == size ? 4 :
                sizeof(signed long long) == size ? 5 : 0;
        static_assert(index, "The compiler doesn't support signed integer of the given bit width");
    public:
        typedef typename __select_signed_type<index>::type type;
    };

    template <int size>
    struct __unsigned_type_of_size {
    private:
        static int constexpr index =
            sizeof(unsigned char     ) == size ? 1 :
            sizeof(unsigned short    ) == size ? 2 :
            sizeof(unsigned int      ) == size ? 3 :
            sizeof(unsigned long     ) == size ? 4 :
            sizeof(unsigned long long) == size ? 5 : 0;
        static_assert(index, "The compiler doesn't support unsigned integer of the given bit width");
    public:
        typedef typename __select_unsigned_type<index>::type type;
    };

    template <int size>
    struct __float_type_of_size {
    private:
        static int constexpr index =
            sizeof(float ) == size ? 1 :
            sizeof(double) == size ? 2 : 0;
        static_assert(index, "The compiler doesn't support float point type of the given bit width");
    public:
        typedef typename __select_float_type<index>::type type;
    };
}

typedef __type_h_deprecated::__signed_type_of_size<1>::type int8;
typedef __type_h_deprecated::__signed_type_of_size<2>::type int16;
typedef __type_h_deprecated::__signed_type_of_size<4>::type int32;
typedef __type_h_deprecated::__signed_type_of_size<8>::type int64;
typedef __type_h_deprecated::__unsigned_type_of_size<1>::type byte;
typedef __type_h_deprecated::__unsigned_type_of_size<2>::type word;
typedef __type_h_deprecated::__unsigned_type_of_size<4>::type dword;
typedef __type_h_deprecated::__unsigned_type_of_size<8>::type qword;
typedef __type_h_deprecated::__unsigned_type_of_size<sizeof(void *)>::type ptrword;
typedef __type_h_deprecated::__float_type_of_size<4>::type float32;
typedef __type_h_deprecated::__float_type_of_size<8>::type float64;

#endif

#endif

#ifndef MAHJONG_CPP_UTILS
#define MAHJONG_CPP_UTILS

template <typename T> inline bool check_exclusive(T x)
{
    return !x || !(x & (x - 1));
}

#endif /* MAHJONG_CPP_UTILS */

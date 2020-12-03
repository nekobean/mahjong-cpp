#ifndef MAHJONG_CPP_UNNECESSARYTILESELECTOR
#define MAHJONG_CPP_UNNECESSARYTILESELECTOR

#include <vector>

#include "types/types.hpp"

namespace mahjong {

class UnnecessaryTileSelector {
public:
    static std::vector<int> select(const Hand &hand, int type);
    static std::vector<int> select_normal(const Hand &hand);
    static std::vector<int> select_tiitoi(const Hand &hand);
    static std::vector<int> select_kokusi(const Hand &hand);
};

} // namespace mahjong

#endif /* MAHJONG_CPP_UNNECESSARYTILESELECTOR */

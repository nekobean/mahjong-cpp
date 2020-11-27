#ifndef MAHJONG_CPP_REQUIREDTILESELECTOR
#define MAHJONG_CPP_REQUIREDTILESELECTOR

#include <vector>

#include "types/types.hpp"

namespace mahjong {

class RequiredTileSelector {
public:
    static std::vector<int> select_normal(const Hand &hand);
    static std::vector<int> select_normal2(const Hand &hand);
};

} // namespace mahjong

#endif /* MAHJONG_CPP_REQUIREDTILESELECTOR */

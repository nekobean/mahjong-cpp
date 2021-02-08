#ifndef MAHJONG_CPP_REQUIREDTILESELECTOR
#define MAHJONG_CPP_REQUIREDTILESELECTOR

#include <vector>

#include "types/types.hpp"

namespace mahjong
{

class RequiredTileSelector
{
  public:
    static std::vector<int> select(const Hand &hand, int type);
    static std::vector<int> select_normal(const Hand &hand);
    static std::vector<int> select_tiitoi(const Hand &hand);
    static std::vector<int> select_kokusi(const Hand &hand);
};

} // namespace mahjong

#endif /* MAHJONG_CPP_REQUIREDTILESELECTOR */

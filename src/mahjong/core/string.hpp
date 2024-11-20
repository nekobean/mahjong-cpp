#ifndef MAHJONG_CPP_STRING
#define MAHJONG_CPP_STRING

#include <string>
#include <vector>

#include "mahjong/types/types.hpp"

namespace mahjong
{

std::string to_mpsz(const Hand &hand);
std::string to_mpsz(const std::vector<int> &tiles);
Hand from_mpsz(const std::string &tiles);
std::string to_string(const Block &block);
std::string to_string(const Meld &meld);
std::string to_string(const Round &round);
std::string to_string(const Player &player);
std::string to_string(const Result &result);

} // namespace mahjong

#endif /* MAHJONG_CPP_STRING */

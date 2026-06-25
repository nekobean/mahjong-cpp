#ifndef MAHJONG_CPP_TOOLS_TENHOU_XML_UTILS
#define MAHJONG_CPP_TOOLS_TENHOU_XML_UTILS

#include <filesystem>
#include <string>
#include <vector>

#include <rapidxml.hpp>

namespace mahjong::tools::tenhou::detail
{

std::vector<char> read_xml_file(const std::filesystem::path &path);
std::string attr_string(const rapidxml::xml_node<> &node, const char *name);
std::vector<std::string> attr_strings(const rapidxml::xml_node<> &node,
                                      const char *name);
int attr_int(const rapidxml::xml_node<> &node, const char *name);
std::vector<int> attr_ints(const rapidxml::xml_node<> &node, const char *name);
double attr_double(const rapidxml::xml_node<> &node, const char *name);
std::vector<double> attr_doubles(const rapidxml::xml_node<> &node, const char *name);
std::vector<std::vector<int>> attr_hands(const rapidxml::xml_node<> &node);

} // namespace mahjong::tools::tenhou::detail

#endif // MAHJONG_CPP_TOOLS_TENHOU_XML_UTILS

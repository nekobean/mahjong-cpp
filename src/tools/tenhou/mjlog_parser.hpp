#ifndef MAHJONG_CPP_TOOLS_TENHOU_MJLOG_PARSER
#define MAHJONG_CPP_TOOLS_TENHOU_MJLOG_PARSER

#include <filesystem>

#include "mjlog_types.hpp"

namespace mahjong::tools::tenhou
{

/**
 * @brief Parses a Tenhou mjlog XML file into a generic event sequence.
 * @param path Path to an mjlog XML file. Gzip-compressed files are also supported.
 * @return Parsed mjlog.
 */
Mjlog parse_mjlog_file(const std::filesystem::path &path);

} // namespace mahjong::tools::tenhou

#endif // MAHJONG_CPP_TOOLS_TENHOU_MJLOG_PARSER

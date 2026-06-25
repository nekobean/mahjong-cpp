#ifndef MAHJONG_CPP_TOOLS_TENHOU_MJLOG_EVENT_FACTORY
#define MAHJONG_CPP_TOOLS_TENHOU_MJLOG_EVENT_FACTORY

#include <string>

#include <rapidxml.hpp>

#include "mjlog_types.hpp"

namespace mahjong::tools::tenhou::detail
{

MjlogShuffleEvent make_shuffle_event(const rapidxml::xml_node<> &node);
MjlogGoEvent make_go_event(const rapidxml::xml_node<> &node);
MjlogUnEvent make_un_event(const rapidxml::xml_node<> &node);
MjlogTaikyokuEvent make_taikyoku_event(const rapidxml::xml_node<> &node);
MjlogInitEvent make_init_event(const rapidxml::xml_node<> &node);
MjlogDrawEvent make_draw_event(const std::string &name);
MjlogDiscardEvent make_discard_event(const std::string &name);
MjlogMeldEvent make_meld_event(const rapidxml::xml_node<> &node);
MjlogDoraEvent make_dora_event(const rapidxml::xml_node<> &node);
MjlogByeEvent make_bye_event(const rapidxml::xml_node<> &node);
MjlogReachEvent make_reach_event(const rapidxml::xml_node<> &node);
MjlogAgariEvent make_agari_event(const rapidxml::xml_node<> &node);
MjlogRyukyokuEvent make_ryukyoku_event(const rapidxml::xml_node<> &node);

} // namespace mahjong::tools::tenhou::detail

#endif // MAHJONG_CPP_TOOLS_TENHOU_MJLOG_EVENT_FACTORY

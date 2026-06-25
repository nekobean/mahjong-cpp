#include "mjlog_event_factory.hpp"

#include "xml_utils.hpp"

#include <string>
#include <string_view>

namespace mahjong::tools::tenhou::detail
{

MjlogShuffleEvent make_shuffle_event(const rapidxml::xml_node<> &node)
{
    return {
        attr_string(node, "seed"),
        attr_string(node, "ref"),
    };
}

MjlogGoEvent make_go_event(const rapidxml::xml_node<> &node)
{
    const std::string lobby = attr_string(node, "lobby");
    return {
        attr_int(node, "type"),
        lobby.empty() ? -1 : std::stoi(lobby),
    };
}

MjlogUnEvent make_un_event(const rapidxml::xml_node<> &node)
{
    MjlogUnEvent event;
    for (int i = 0; i < 4; ++i) {
        event.names.push_back(attr_string(node, ("n" + std::to_string(i)).c_str()));
    }
    event.dan = attr_ints(node, "dan");
    event.rate = attr_doubles(node, "rate");
    event.sx = attr_strings(node, "sx");
    return event;
}

MjlogTaikyokuEvent make_taikyoku_event(const rapidxml::xml_node<> &node)
{
    return {attr_int(node, "oya")};
}

MjlogInitEvent make_init_event(const rapidxml::xml_node<> &node)
{
    return {
        attr_ints(node, "seed"),
        attr_ints(node, "ten"),
        attr_int(node, "oya"),
        attr_hands(node),
    };
}

MjlogDrawEvent make_draw_event(const std::string &name)
{
    static constexpr std::string_view Tags = "TUVW";
    return {
        static_cast<int>(Tags.find(name.front())),
        std::stoi(name.substr(1)),
    };
}

MjlogDiscardEvent make_discard_event(const std::string &name)
{
    static constexpr std::string_view Tags = "DEFG";
    return {
        static_cast<int>(Tags.find(name.front())),
        std::stoi(name.substr(1)),
    };
}

MjlogMeldEvent make_meld_event(const rapidxml::xml_node<> &node)
{
    return {
        attr_int(node, "who"),
        attr_int(node, "m"),
    };
}

MjlogDoraEvent make_dora_event(const rapidxml::xml_node<> &node)
{
    return {attr_int(node, "hai")};
}

MjlogByeEvent make_bye_event(const rapidxml::xml_node<> &node)
{
    return {attr_int(node, "who")};
}

MjlogReachEvent make_reach_event(const rapidxml::xml_node<> &node)
{
    return {
        attr_int(node, "who"),
        attr_int(node, "step"),
        attr_ints(node, "ten"),
    };
}

MjlogAgariEvent make_agari_event(const rapidxml::xml_node<> &node)
{
    return {
        attr_ints(node, "ba"),         attr_ints(node, "hai"),
        attr_ints(node, "m"),          attr_int(node, "machi"),
        attr_ints(node, "ten"),        attr_ints(node, "yaku"),
        attr_ints(node, "yakuman"),    attr_ints(node, "doraHai"),
        attr_ints(node, "doraHaiUra"), attr_int(node, "who"),
        attr_int(node, "fromWho"),     attr_ints(node, "sc"),
        attr_ints(node, "owari"),
    };
}

MjlogRyukyokuEvent make_ryukyoku_event(const rapidxml::xml_node<> &node)
{
    return {
        attr_ints(node, "ba"),     attr_ints(node, "sc"),    attr_hands(node),
        attr_string(node, "type"), attr_ints(node, "owari"),
    };
}

} // namespace mahjong::tools::tenhou::detail

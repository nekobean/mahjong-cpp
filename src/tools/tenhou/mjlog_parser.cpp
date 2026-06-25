#include "mjlog_parser.hpp"
#include "mjlog_event_factory.hpp"
#include "xml_utils.hpp"

#include <algorithm>
#include <cctype>
#include <string>

#include <rapidxml.hpp>

namespace mahjong::tools::tenhou
{

namespace
{

bool is_tile_event_name(const std::string &name, const char *tags)
{
    return name.size() >= 2 && std::char_traits<char>::find(tags, 4, name.front()) &&
           std::all_of(name.begin() + 1, name.end(), [](const char c) {
               return std::isdigit(static_cast<unsigned char>(c));
           });
}

void parse_node(Mjlog &log, const rapidxml::xml_node<> &node)
{
    const std::string name{node.name(), node.name_size()};
    if (name == "SHUFFLE") {
        log.events.push_back(detail::make_shuffle_event(node));
    }
    else if (name == "GO") {
        log.events.push_back(detail::make_go_event(node));
    }
    else if (name == "UN") {
        log.events.push_back(detail::make_un_event(node));
    }
    else if (name == "TAIKYOKU") {
        log.events.push_back(detail::make_taikyoku_event(node));
    }
    else if (name == "INIT") {
        log.events.push_back(detail::make_init_event(node));
    }
    else if (is_tile_event_name(name, "TUVW")) {
        log.events.push_back(detail::make_draw_event(name));
    }
    else if (is_tile_event_name(name, "DEFG")) {
        log.events.push_back(detail::make_discard_event(name));
    }
    else if (name == "N") {
        log.events.push_back(detail::make_meld_event(node));
    }
    else if (name == "DORA") {
        log.events.push_back(detail::make_dora_event(node));
    }
    else if (name == "BYE") {
        log.events.push_back(detail::make_bye_event(node));
    }
    else if (name == "REACH") {
        log.events.push_back(detail::make_reach_event(node));
    }
    else if (name == "AGARI") {
        log.events.push_back(detail::make_agari_event(node));
    }
    else if (name == "RYUUKYOKU") {
        log.events.push_back(detail::make_ryukyoku_event(node));
    }
    else {
        log.events.push_back(MjlogUnknownEvent{name});
    }
}

} // namespace

Mjlog parse_mjlog_file(const std::filesystem::path &path)
{
    auto buffer = detail::read_xml_file(path);

    rapidxml::xml_document<> doc;
    doc.parse<rapidxml::parse_no_data_nodes>(buffer.data());

    Mjlog log;
    log.source_file = path.filename().string();

    auto *root = doc.first_node();
    log.ver = detail::attr_string(*root, "ver");
    for (auto *node = root->first_node(); node; node = node->next_sibling()) {
        parse_node(log, *node);
    }

    return log;
}

} // namespace mahjong::tools::tenhou

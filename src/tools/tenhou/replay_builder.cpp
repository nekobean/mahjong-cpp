#include "replay_builder.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <optional>
#include <tuple>
#include <variant>

namespace mahjong::tools::tenhou
{

namespace
{

RuleFlags to_rule_flags(const MjlogGoEvent &event)
{
    RuleFlags rules = RuleFlag::None;
    if ((event.type & 0x0002) == 0) {
        rules |= RuleFlag::RedDora;
    }
    if ((event.type & 0x0004) == 0) {
        rules |= RuleFlag::OpenTanyao;
    }
    return rules;
}

int to_game_mode(const MjlogGoEvent &event)
{
    return (event.type & 0x0010) != 0 ? GameMode::Sanma : GameMode::Yonma;
}

int to_game_length(const MjlogGoEvent &event)
{
    return (event.type & 0x0008) != 0 ? GameLength::Hanchan : GameLength::Tonpu;
}

GameSpeed to_game_speed(const MjlogGoEvent &event)
{
    return (event.type & 0x0040) != 0 ? GameSpeed::Fast : GameSpeed::Normal;
}

TableLevel to_table_level(const MjlogGoEvent &event)
{
    switch (event.type & 0x00A0) {
    case 0x0080:
        return TableLevel::Joukyu;
    case 0x0020:
        return TableLevel::Tokujou;
    case 0x00A0:
        return TableLevel::Houou;
    default:
        return TableLevel::Ippan;
    }
}

int to_tile(const int tile136)
{
    assert(0 <= tile136 && tile136 < 136);

    switch (tile136) {
    case 16:
        return Tile::RedManzu5;
    case 52:
        return Tile::RedPinzu5;
    case 88:
        return Tile::RedSouzu5;
    default:
        return tile136 / 4;
    }
}

Gender to_gender(const std::string &sx)
{
    if (sx == "M") {
        return Gender::Male;
    }
    if (sx == "F") {
        return Gender::Female;
    }
    if (sx == "C") {
        return Gender::Computer;
    }
    return Gender::Unknown;
}

int to_seat_wind(const int player_id, const int dealer, const int num_players)
{
    return Tile::East + ((player_id - dealer + num_players) % num_players);
}

std::vector<int> to_score_deltas(const std::vector<int> &sc)
{
    assert(sc.size() % 2 == 0);

    std::vector<int> ret;
    ret.reserve(sc.size() / 2);
    for (size_t i = 1; i < sc.size(); i += 2) {
        ret.push_back(sc[i] * 100);
    }
    return ret;
}

int to_score_limit(const MjlogAgariEvent &event, const bool is_yakuman)
{
    assert(event.ten.size() >= 3);

    if (is_yakuman) {
        switch (event.yakuman.size()) {
        case 1:
            return ScoreLimit::Yakuman;
        case 2:
            return ScoreLimit::DoubleYakuman;
        case 3:
            return ScoreLimit::TripleYakuman;
        case 4:
            return ScoreLimit::QuadrupleYakuman;
        case 5:
            return ScoreLimit::QuintupleYakuman;
        case 6:
            return ScoreLimit::SextupleYakuman;
        default:
            assert(false);
            return ScoreLimit::Null;
        }
    }

    switch (event.ten[2]) {
    case 0:
        return ScoreLimit::Null;
    case 1:
        return ScoreLimit::Mangan;
    case 2:
        return ScoreLimit::Haneman;
    case 3:
        return ScoreLimit::Baiman;
    case 4:
        return ScoreLimit::Sanbaiman;
    case 5:
        return ScoreLimit::CountedYakuman;
    default:
        return ScoreLimit::Null;
    }
}

int to_win_flag(const int yaku_id)
{
    switch (yaku_id) {
    case 1:
        return WinFlag::Riichi;
    case 2:
        return WinFlag::Ippatsu;
    case 3:
        return WinFlag::RobbingAKan;
    case 4:
        return WinFlag::AfterAKan;
    case 5:
        return WinFlag::UnderTheSea;
    case 6:
        return WinFlag::UnderTheRiver;
    case 21:
        return WinFlag::DoubleRiichi;
    case 37:
        return WinFlag::HeavenlyHand;
    case 38:
        return WinFlag::EarthlyHand;
    default:
        return WinFlag::None;
    }
}

int to_win_flags(const MjlogAgariEvent &event)
{
    int ret = WinFlag::None;
    if (!event.yakuman.empty()) {
        for (const int yaku_id : event.yakuman) {
            ret |= to_win_flag(yaku_id);
        }
        return ret;
    }

    assert(event.yaku.size() % 2 == 0);
    for (size_t i = 0; i + 1 < event.yaku.size(); i += 2) {
        ret |= to_win_flag(event.yaku[i]);
    }
    return ret;
}

int to_ryukyoku_type(const std::string &type)
{
    if (type == "yao9") {
        return RyukyokuType::NineTerminals;
    }
    if (type == "kaze4") {
        return RyukyokuType::FourWinds;
    }
    if (type == "reach4") {
        return RyukyokuType::FourRiichi;
    }
    if (type == "ron3") {
        return RyukyokuType::ThreeRon;
    }
    if (type == "nm") {
        return RyukyokuType::Null;
    }
    return RyukyokuType::Exhaustive;
}

YakuFlags to_yaku(const int yaku_id)
{
    switch (yaku_id) {
    case 0:
        return Yaku::Tsumo;
    case 1:
        return Yaku::Riichi;
    case 2:
        return Yaku::Ippatsu;
    case 3:
        return Yaku::RobbingAKan;
    case 4:
        return Yaku::AfterAKan;
    case 5:
        return Yaku::UnderTheSea;
    case 6:
        return Yaku::UnderTheRiver;
    case 7:
        return Yaku::Pinfu;
    case 8:
        return Yaku::Tanyao;
    case 9:
        return Yaku::PureDoubleSequence;
    case 10:
        return Yaku::SelfWindEast;
    case 11:
        return Yaku::SelfWindSouth;
    case 12:
        return Yaku::SelfWindWest;
    case 13:
        return Yaku::SelfWindNorth;
    case 14:
        return Yaku::RoundWindEast;
    case 15:
        return Yaku::RoundWindSouth;
    case 16:
        return Yaku::RoundWindWest;
    case 17:
        return Yaku::RoundWindNorth;
    case 18:
        return Yaku::WhiteDragon;
    case 19:
        return Yaku::GreenDragon;
    case 20:
        return Yaku::RedDragon;
    case 21:
        return Yaku::DoubleRiichi;
    case 22:
        return Yaku::SevenPairs;
    case 23:
        return Yaku::HalfOutsideHand;
    case 24:
        return Yaku::PureStraight;
    case 25:
        return Yaku::MixedTripleSequence;
    case 26:
        return Yaku::MixedTripleTriplets;
    case 27:
        return Yaku::ThreeKans;
    case 28:
        return Yaku::AllTriplets;
    case 29:
        return Yaku::ThreeConcealedTriplets;
    case 30:
        return Yaku::LittleThreeDragons;
    case 31:
        return Yaku::AllTerminalsAndHonors;
    case 32:
        return Yaku::TwicePureDoubleSequence;
    case 33:
        return Yaku::FullyOutsideHand;
    case 34:
        return Yaku::HalfFlush;
    case 35:
        return Yaku::FullFlush;
    case 52:
        return Yaku::Dora;
    case 53:
        return Yaku::UraDora;
    case 54:
        return Yaku::RedDora;
    default:
        return Yaku::None;
    }
}

YakuEntry to_yakuman(const int yaku_id)
{
    switch (yaku_id) {
    case 37:
        return {Yaku::HeavenlyHand, 1};
    case 38:
        return {Yaku::EarthlyHand, 1};
    case 39:
        return {Yaku::BigThreeDragons, 1};
    case 40:
        return {Yaku::FourConcealedTriplets, 1};
    case 41:
        return {Yaku::SingleWaitFourConcealedTriplets, 1};
    case 42:
        return {Yaku::AllHonors, 1};
    case 43:
        return {Yaku::AllGreen, 1};
    case 44:
        return {Yaku::AllTerminals, 1};
    case 45:
        return {Yaku::NineGates, 1};
    case 46:
        return {Yaku::TrueNineGates, 1};
    case 47:
        return {Yaku::ThirteenOrphans, 1};
    case 48:
        return {Yaku::ThirteenWaitThirteenOrphans, 1};
    case 49:
        return {Yaku::BigFourWinds, 1};
    case 50:
        return {Yaku::LittleFourWinds, 1};
    case 51:
        return {Yaku::FourKans, 1};
    default:
        return {Yaku::None, 0};
    }
}

void sort_yaku_list(std::vector<YakuEntry> &yaku_list)
{
    std::sort(yaku_list.begin(), yaku_list.end(),
              [](const auto &a, const auto &b) { return a.yaku < b.yaku; });
}

std::vector<YakuEntry> to_yaku_entries(const std::vector<int> &raw_yaku,
                                       const int nuki_count)
{
    assert(raw_yaku.size() % 2 == 0);

    std::vector<YakuEntry> ret;
    for (size_t i = 0; i + 1 < raw_yaku.size(); i += 2) {
        const int yaku_id = raw_yaku[i];
        int han = raw_yaku[i + 1];
        if (han == 0) {
            continue;
        }

        const YakuFlags yaku = to_yaku(yaku_id);

        if (yaku == Yaku::Dora && nuki_count > 0) {
            han -= nuki_count;
            if (han > 0) {
                ret.push_back({yaku, han});
            }
            ret.push_back({Yaku::NukiDora, nuki_count});
        }
        else {
            ret.push_back({yaku, han});
        }
    }

    if (nuki_count > 0 && std::none_of(ret.begin(), ret.end(), [](const auto &x) {
            return x.yaku == Yaku::NukiDora;
        })) {
        ret.push_back({Yaku::NukiDora, nuki_count});
    }

    sort_yaku_list(ret);
    return ret;
}

std::vector<YakuEntry> to_yakuman_entries(const std::vector<int> &raw_yakuman)
{
    std::vector<YakuEntry> ret;
    for (const int yaku_id : raw_yakuman) {
        ret.push_back(to_yakuman(yaku_id));
    }
    sort_yaku_list(ret);
    return ret;
}

int sum_han(const std::vector<YakuEntry> &yaku)
{
    int ret = 0;
    for (const auto &entry : yaku) {
        ret += entry.han;
    }
    return ret;
}

int to_meld_from_seat(const int value)
{
    assert(0 <= value && value < 4);

    switch (value) {
    case 0:
        return SeatType::Self;
    case 1:
        return SeatType::Shimocha;
    case 2:
        return SeatType::Toimen;
    case 3:
        return SeatType::Kamicha;
    default:
        return SeatType::Null;
    }
}

std::optional<Meld> decode_meld(const int value)
{
    constexpr int FromMask = 0x0003;
    constexpr int ChiMask = 0x0004;
    constexpr int PonMask = 0x000C;
    constexpr int PonValue = 0x0008;
    constexpr int KakanMask = 0x0014;
    constexpr int KakanValue = 0x0010;
    constexpr int NukiMask = 0x003C;
    constexpr int NukiValue = 0x0020;

    if ((value & NukiMask) == NukiValue) {
        return std::nullopt;
    }

    const int from = to_meld_from_seat(value & FromMask);

    if (value & ChiMask) {
        const int encoded = (value & 0xFC00) >> 10;
        const int called_tile_index = encoded % 3;
        const int sequence_base136 =
            (((encoded / 3) / 7) * 9 + ((encoded / 3) % 7)) * 4;
        const std::array<int, 3> tile_ids = {
            sequence_base136 + ((value & 0x0018) >> 3),
            sequence_base136 + 4 + ((value & 0x0060) >> 5),
            sequence_base136 + 8 + ((value & 0x0180) >> 7),
        };

        return Meld{
            MeldType::Chi,
            {to_tile(tile_ids[0]), to_tile(tile_ids[1]), to_tile(tile_ids[2])},
            to_tile(tile_ids[called_tile_index]),
            from,
        };
    }

    if ((value & PonMask) == PonValue) {
        const int encoded = (value & 0xFE00) >> 9;
        const int called_tile_index = encoded % 3;
        const int base = (encoded / 3) * 4;
        const int unused = base + ((value & 0x0060) >> 5);
        std::array<int, 3> tile_ids{};
        int index = 0;
        for (int i = 0; i < 4; ++i) {
            if (base + i != unused) {
                tile_ids[index++] = base + i;
            }
        }
        assert(index == 3);

        return Meld{
            MeldType::Pon,
            {to_tile(tile_ids[0]), to_tile(tile_ids[1]), to_tile(tile_ids[2])},
            to_tile(tile_ids[called_tile_index]),
            from,
        };
    }

    if ((value & KakanMask) == KakanValue) {
        const int encoded = (value & 0xFE00) >> 9;
        const int base = (encoded / 3) * 4;
        const int added = base + ((value & 0x0060) >> 5);

        return Meld{
            MeldType::Kakan,
            {to_tile(base), to_tile(base + 1), to_tile(base + 2), to_tile(base + 3)},
            to_tile(added),
            from,
        };
    }

    const int raw = (value & 0xFF00) >> 8;
    const int base = raw - raw % 4;
    const int type = (value & FromMask) == 0 ? MeldType::Ankan : MeldType::Daiminkan;

    return Meld{
        type,
        {to_tile(base), to_tile(base + 1), to_tile(base + 2), to_tile(base + 3)},
        type == MeldType::Ankan ? Tile::Null : to_tile(raw),
        from,
    };
}

std::tuple<std::vector<Meld>, int> decode_melds(const std::vector<int> &raw_melds)
{
    std::vector<Meld> melds;
    melds.reserve(raw_melds.size());
    int nuki_count = 0;

    for (const int raw_meld : raw_melds) {
        const auto meld = decode_meld(raw_meld);
        if (meld) {
            melds.push_back(*meld);
        }
        else {
            ++nuki_count;
        }
    }

    return {melds, nuki_count};
}

GameMeta make_meta(const Mjlog &log)
{
    return {
        log.source_file,
        log.ver,
    };
}

TableConfig make_table(const MjlogGoEvent &event)
{
    return {
        to_game_mode(event),  to_rule_flags(event),  to_game_length(event),
        to_game_speed(event), to_table_level(event),
    };
}

Hand make_hand(const std::vector<int> &tiles136)
{
    Hand hand{};
    for (const int tile136 : tiles136) {
        const int tile = to_tile(tile136);
        ++hand[tile];
    }
    return hand;
}

std::vector<PlayerProfile> make_players(const MjlogUnEvent &event)
{
    assert(event.names.size() == 4);
    assert(event.dan.size() == 4);
    assert(event.rate.size() == 4);
    assert(event.sx.size() == 4);

    const int num_players = event.names[3].empty() ? 3 : 4;

    std::vector<PlayerProfile> ret;
    ret.reserve(num_players);

    for (int i = 0; i < num_players; ++i) {
        PlayerProfile player;
        player.id = i;
        player.name = event.names[i];
        player.rank = static_cast<Rank>(event.dan[i]);
        player.rate = event.rate[i];
        player.gender = to_gender(event.sx[i]);
        ret.push_back(player);
    }

    return ret;
}

std::vector<PlayerState> make_player_states(const MjlogInitEvent &event,
                                            const int num_players)
{
    assert(num_players == 3 || num_players == 4);
    assert(0 <= event.oya && event.oya < num_players);
    assert(event.hands.size() >= static_cast<size_t>(num_players));
    assert(event.ten.size() >= static_cast<size_t>(num_players));

    std::vector<PlayerState> ret;
    ret.reserve(num_players);

    for (int i = 0; i < num_players; ++i) {
        PlayerState player;
        player.hand = make_hand(event.hands[i]);
        player.seat_wind = to_seat_wind(i, event.oya, num_players);
        player.nuki_count = 0;
        player.score = event.ten[i] * 100;
        ret.push_back(player);
    }

    return ret;
}

RoundRecord make_round_record(const MjlogInitEvent &event, const int num_players)
{
    assert(event.seed.size() == 6);

    const int round_id = event.seed[0];

    RoundRecord record;
    record.initial.round.round_wind = Tile::East + round_id / 4;
    record.initial.round.round_number = round_id % 4 + 1;
    record.initial.round.honba = event.seed[1];
    record.initial.round.dealer = event.oya;
    record.initial.table.kyotaku = event.seed[2];
    record.initial.table.dora_indicators = {to_tile(event.seed[5])};
    record.initial.players = make_player_states(event, num_players);
    record.last = record.initial;
    return record;
}

CallEvent make_call_event(const MjlogMeldEvent &event)
{
    const auto meld = decode_meld(event.m);
    assert(meld);
    return {event.who, *meld};
}

WinResult make_win_result(const RoundSnapshot &state, const MjlogAgariEvent &event)
{
    const bool is_yakuman = !event.yakuman.empty();
    const auto [melds, nuki_count] = decode_melds(event.m);
    assert(event.ten.size() >= 3);
    assert(event.sc.size() % 2 == 0);
    assert(event.who >= 0 && event.who < static_cast<int>(state.players.size()));
    assert(event.from_who >= 0 &&
           event.from_who < static_cast<int>(state.players.size()));

    WinResult result;
    result.result_round = state.round;
    result.result_table = state.table;
    result.player = state.players[event.who];
    result.player.hand = make_hand(event.hai);
    result.player.melds = melds;
    result.player.nuki_count = nuki_count;
    result.winner = event.who;
    result.loser =
        event.who == event.from_who ? std::nullopt : std::optional<int>{event.from_who};
    result.winning_tile = to_tile(event.machi);
    result.win_flags = to_win_flags(event);
    if (event.who == event.from_who) {
        result.win_flags |= WinFlag::Tsumo;
    }
    result.yaku = is_yakuman ? to_yakuman_entries(event.yakuman)
                             : to_yaku_entries(event.yaku, nuki_count);
    result.han = is_yakuman ? 0 : sum_han(result.yaku);
    result.fu = is_yakuman ? 0 : event.ten[0];
    result.score_limit = to_score_limit(event, is_yakuman);
    result.score_deltas = to_score_deltas(event.sc);
    result.pao = std::nullopt;
    return result;
}

RyukyokuResult make_ryukyoku_result(const RoundSnapshot &state,
                                    const MjlogRyukyokuEvent &event, const int type)
{
    assert(event.sc.size() % 2 == 0);
    return {state.round, state.table, type, to_score_deltas(event.sc)};
}

void add_tile(PlayerState &player, const int tile)
{
    assert(0 <= tile && tile < Tile::Length);
    ++player.hand[tile];
}

void remove_tile(PlayerState &player, const int tile)
{
    assert(0 <= tile && tile < Tile::Length);
    assert(player.hand[tile] > 0);
    --player.hand[tile];
}

void remove_meld_tiles(PlayerState &player, const Meld &meld)
{
    if (meld.type == MeldType::Ankan) {
        for (const int tile : meld.tiles) {
            remove_tile(player, tile);
        }
        return;
    }

    if (meld.type == MeldType::Kakan) {
        remove_tile(player, meld.discarded_tile);
        return;
    }

    bool skipped_called_tile = false;
    for (const int tile : meld.tiles) {
        if (!skipped_called_tile && tile == meld.discarded_tile) {
            skipped_called_tile = true;
            continue;
        }
        remove_tile(player, tile);
    }
}

void apply_call_event(RoundRecord &record, const CallEvent &event)
{
    assert(event.actor >= 0 && event.actor < static_cast<int>(record.last.players.size()));
    PlayerState &player = record.last.players[event.actor];
    remove_meld_tiles(player, event.meld);
    player.melds.push_back(event.meld);
}

void apply_nuki_event(RoundRecord &record, const MjlogMeldEvent &event)
{
    assert(event.who >= 0 && event.who < static_cast<int>(record.last.players.size()));
    PlayerState &player = record.last.players[event.who];
    remove_tile(player, Tile::North);
    ++player.nuki_count;
}

void apply_event(RoundRecord &record, const MjlogEvent &event)
{
    constexpr int NukiMask = 0x003C;
    constexpr int NukiValue = 0x0020;

    if (const auto *draw = std::get_if<MjlogDrawEvent>(&event)) {
        assert(draw->player >= 0 &&
               draw->player < static_cast<int>(record.last.players.size()));
        const int tile = to_tile(draw->tile136);
        add_tile(record.last.players[draw->player], tile);
        record.events.push_back(DrawEvent{draw->player, tile});
        return;
    }

    if (const auto *discard = std::get_if<MjlogDiscardEvent>(&event)) {
        assert(discard->player >= 0 &&
               discard->player < static_cast<int>(record.last.players.size()));
        const int tile = to_tile(discard->tile136);
        remove_tile(record.last.players[discard->player], tile);
        record.events.push_back(DiscardEvent{discard->player, tile, false});
        return;
    }

    if (const auto *meld = std::get_if<MjlogMeldEvent>(&event)) {
        if ((meld->m & NukiMask) == NukiValue) {
            apply_nuki_event(record, *meld);
            return;
        }
        const auto call = make_call_event(*meld);
        apply_call_event(record, call);
        record.events.push_back(call);
        return;
    }

    if (const auto *dora = std::get_if<MjlogDoraEvent>(&event)) {
        const int dora_indicator = to_tile(dora->hai);
        record.last.table.dora_indicators.push_back(dora_indicator);
        record.events.push_back(DoraOpenEvent{dora_indicator});
        return;
    }

    if (const auto *reach = std::get_if<MjlogReachEvent>(&event)) {
        if (reach->step == 2) {
            ++record.last.table.kyotaku;
            record.events.push_back(RiichiEvent{reach->who});
        }
        return;
    }

    if (const auto *agari = std::get_if<MjlogAgariEvent>(&event)) {
        const int win_tile = to_tile(agari->machi);
        assert(agari->who >= 0 &&
               agari->who < static_cast<int>(record.last.players.size()));
        assert(agari->from_who >= 0 &&
               agari->from_who < static_cast<int>(record.last.players.size()));
        if (agari->who == agari->from_who) {
            record.events.push_back(TsumoEvent{agari->who, win_tile});
        }
        else {
            record.events.push_back(RonEvent{agari->who, agari->from_who, win_tile});
        }
        record.results.push_back(make_win_result(record.last, *agari));
        return;
    }

    if (const auto *ryukyoku = std::get_if<MjlogRyukyokuEvent>(&event)) {
        const int type = to_ryukyoku_type(ryukyoku->type);
        record.events.push_back(RyukyokuEvent{type});
        record.results.push_back(make_ryukyoku_result(record.last, *ryukyoku, type));
    }
}

} // namespace

GameRecord build_replay(const Mjlog &log)
{
    GameRecord game;
    game.meta = make_meta(log);

    for (const auto &event : log.events) {
        if (const auto *go = std::get_if<MjlogGoEvent>(&event)) {
            game.table = make_table(*go);
            continue;
        }

        if (const auto *un = std::get_if<MjlogUnEvent>(&event)) {
            if (game.players.empty()) {
                game.players = make_players(*un);
            }
            continue;
        }

        if (const auto *init = std::get_if<MjlogInitEvent>(&event)) {
            assert(!game.players.empty());
            game.rounds.push_back(
                make_round_record(*init, static_cast<int>(game.players.size())));
            continue;
        }

        if (!game.rounds.empty()) {
            apply_event(game.rounds.back(), event);
        }
    }

    return game;
}

} // namespace mahjong::tools::tenhou

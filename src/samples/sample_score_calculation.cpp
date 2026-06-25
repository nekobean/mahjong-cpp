#include <iostream>

#include "mahjong/mahjong.hpp"

int main()
{
    using namespace mahjong;

    TableConfig table_config;
    table_config.rule_flags = RuleFlag::RedDora | RuleFlag::OpenTanyao;

    RoundState round_state;
    // Set round wind from Tile::East, Tile::South, Tile::West, Tile::North. (場風)
    round_state.round_wind = Tile::East;
    // Set kyoku from 1 to 4. (局)
    round_state.round_number = 1;
    // Set honba. (本場)
    round_state.honba = 0;

    TableState table_state;
    // Set kyotaku. (供託棒の数)
    // For double-ron or triple-ron rules, set this to 0 when calculating a
    // winner who does not receive honba sticks or kyotaku sticks.
    table_state.kyotaku = 1;

    // Set dora indicators. (表ドラ表示牌)
    table_state.dora_indicators = {Tile::North};
    // To pass dora tiles instead of indicator tiles, use TableState::set_dora().
    // table_state.set_dora({Tile::East}, table_config.game_mode);

    // Set uradora indicator tiles. (裏ドラ表示牌)
    table_state.uradora_indicators = {Tile::Pinzu9};
    // To pass uradora tiles instead of indicator tiles, use TableState::set_uradora().
    // table_state.set_uradora({Tile::Pinzu1}, table_config.game_mode);

    // Set player information.
    PlayerState player;
    player.hand = from_mpsz("11123m567p333z444z");
    // Set seat wind from Tile::East, Tile::South, Tile::West, Tile::North. (自風)
    player.seat_wind = Tile::East;
    // Set winning tile. (和了牌)
    const int win_tile = Tile::Manzu1;

    // Set winning flags. (和了フラグ)
    // WinFlag::Tsumo         : self-draw win (自摸和了)
    // WinFlag::Riichi        : riichi (立直)
    // WinFlag::Ippatsu       : ippatsu (一発)
    // WinFlag::RobbingAKan   : robbing a kan (槍槓)
    // WinFlag::AfterAKan     : after a kan (嶺上開花)
    // WinFlag::UnderTheSea   : under the sea (海底摸月)
    // WinFlag::UnderTheRiver : under the river (河底撈魚)
    // WinFlag::DoubleRiichi  : double riichi (ダブル立直)
    // WinFlag::NagashiMangan : nagashi mangan (流し満貫)
    // WinFlag::HeavenlyHand  : heavenly hand (天和)
    // WinFlag::EarthlyHand   : earthly hand (地和)
    // WinFlag::HandOfMan     : hand of man (人和)
    const int flag = WinFlag::Tsumo | WinFlag::Riichi;

    // Calculate score.
    const ScoreResult result = ScoreCalculator::calc(
        table_config, round_state, table_state, player, win_tile, flag);
    std::cout << to_string(result) << std::endl;
}

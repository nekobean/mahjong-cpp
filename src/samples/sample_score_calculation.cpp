#include <iostream>

#include "mahjong/mahjong.hpp"

int main(int argc, char *argv[])
{
    using namespace mahjong;

    // Set round infomation.
    /////////////////////////////////////////////////////
    Round round;
    // Set game rule. (ルール)
    round.rules = RuleFlag::RedDora | RuleFlag::OpenTanyao;
    // Set round wind from Tile::East, Tile::South, Tile::West, Tile::North. (場風)
    round.wind = Tile::East; // round wind
    // Set kyoku from 1 to 4. (局)
    round.kyoku = 1;
    // Set honba. (本場)
    round.honba = 0;
    // Set kyotaku. (供託棒の数)
    // ダブロン、トリロン有りのルールの場合、積み棒、供託棒を受け取らない和了者の
    // 計算時には0に設定してください。
    round.kyotaku = 1;
    // Set dora indicators. (表ドラ表示牌)
    round.dora_indicators = {Tile::North};
    // If specifying dora, use set_dora().
    // round.set_dora(Tile::East);
    // Set uradora indicators. (裏ドラ表示牌)
    round.uradora_indicators = {Tile::Pinzu9};
    // If specifying dora, use set_dora().
    // round.set_uradora({Tile::Pinzu1});

    // Set player information.
    /////////////////////////////////////////////////////

    // Create hand by mpsz notation or vector of tiles.
    Hand hand = from_mpsz("222567345p333s22z");
    // Hand hand = from_array({Tile::Manzu2, Tile::Manzu2, Tile::Manzu2, Tile::Manzu5,
    //                          Tile::Manzu6, Tile::Manzu7, Tile::Pinzu3, Tile::Pinzu4,
    //                          Tile::Pinzu5, Tile::Souzu3, Tile::Souzu3, Tile::Souzu3,
    //                          Tile::South, Tile::South});
    Player player;
    player.hand = hand;
    player.melds = {};
    // Set seat wind from Tile::East, Tile::South, Tile::West, Tile::North. (自風)
    player.wind = Tile::East;
    // Set win tile (和了牌)
    const int win_tile = Tile::South;
    // Set win flag (フラグ)
    // WinFlag::Tsumo           : Tsumo win (自摸和了)
    // WinFlag::Riichi          : Riich established (立直成立)
    // WinFlag::Ippatsu         : One-shot Win established (一発成立)
    // WinFlag::RobbingAKong    : Robbing a Kong established (搶槓成立)
    // WinFlag::AfterAKong      : After a Kong established (嶺上開花成立)
    // WinFlag::UnderTheSea     : Under the Sea established (海底撈月成立)
    // WinFlag::UnderTheRiver   : Under the River established (河底撈魚成立)
    // WinFlag::DoubleRiichi    : Double Riichi established (ダブル立直成立)
    // WinFlag::NagashiMangan   : Mangan at Draw established (流し満貫成立)
    // WinFlag::BlessingOfHeaven: Blessing of Heaven established (天和成立)
    // WinFlag::BlessingOfEarth : Blessing of Earth established (地和成立)
    // WinFlag::HandOfMan       : Hand of Man established (人和成立)
    const int flag = WinFlag::Tsumo | WinFlag::Riichi;

    // Calculate score.
    /////////////////////////////////////////////////////
    const Result result = ScoreCalculator::calc(round, player, win_tile, flag);
    // Result stores score, han, fu, yaku, etc... Sea also src/mahjong/types/result.hpp
    std::cout << to_string(result) << std::endl;
}

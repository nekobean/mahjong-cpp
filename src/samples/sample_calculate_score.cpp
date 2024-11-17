#include "mahjong/mahjong.hpp"

using namespace mahjong;

int main(int, char **)
{
    // calc() に手牌を指定した場合、通常手、七対子手、国士無双手の向聴数を計算し、
    // 向聴数が最小となる手の種類及び向聴数をタプルで返します。
    {
        ScoreCalculator score;

        // 場やルールの設定
        ////////////////////////////////////////////////////////////////////////////////

        // 場風牌を Tile::East, Tile::South, Tile::West, Tile::North から設定します。
        score.set_wind(Tile::East);
        // 自風牌を Tile::East, Tile::South, Tile::West, Tile::North から設定します。
        score.set_self_wind(Tile::East);
        // 積み棒の数を設定します。(例: 1本場なら1)
        score.set_bonus_stick(0);
        // 供託棒 (立直棒) の数を設定します。
        score.set_deposit_stick(0);
        // ※ ダブロン、トリロン有りのルールの場合、積み棒、供託棒を受け取らない和了者の精算時には0に設定してください。

        // ドラの一覧 (表示牌ではない) を設定します。
        score.set_dora_tiles({Tile::North});
        // 裏ドラがある場合は、裏ドラの一覧 (表示牌ではない) を設定します。
        score.set_uradora_tiles({Tile::Pinzu9});
        // ルールを設定します。デフォルトは赤ドラ有り、喰い断有りのありありルールです。
        // な変更したい場合は以下のように設定します。
        // score.set_rule(RuleFlag::RedDora, false);  // 赤ドラなし
        // score.set_rule(RuleFlag::OpenTanyao, false); // 喰い断なし

        // 手牌、和了牌、フラグの設定
        ////////////////////////////////////////////////////////////////////////////////

        // 手牌
        MeldedBlock block(MeldType::AddedKong,
                          {Tile::East, Tile::East, Tile::East, Tile::East});
        Hand hand({Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, //
                   Tile::Pinzu3, Tile::Pinzu4, Tile::Pinzu5, //
                   Tile::Souzu1, Tile::Souzu2, Tile::Souzu3, //
                   Tile::Souzu4, Tile::Souzu4},
                  {block});
        // 和了牌
        int win_tile = Tile::Manzu1;

        // フラグ (自摸和了、立直など手牌に関係ない点数計算に必要なフラグを指定します。)
        // WinFlag::Tsumo         自摸和了 (門前かどうかに関わらず、自摸和了の場合は指定)
        // WinFlag::Riichi        立直成立
        // WinFlag::Ippatsu       一発成立
        // WinFlag::RobbingAKong      搶槓成立
        // WinFlag::AfterAKong 嶺上開花成立
        // WinFlag::HaiteiRaoyue   海底撈月成立
        // WinFlag::UnderTheRiver     河底撈魚成立
        // WinFlag::DoubleRiichi  ダブル立直成立
        // WinFlag::NagasiMangan 流し満貫成立
        // WinFlag::Tenho        天和成立
        // WinFlag::Tiho         地和成立
        // WinFlag::Renho        人和成立
        int flag = WinFlag::Tsumo | WinFlag::AfterAKong;

        // 点数計算の実行
        ////////////////////////////////////////////////////////////////////////////////

        Result ret = score.calc(hand, win_tile, flag);

        // 結果は Result 構造体に格納されています。to_string() で文字列として出力できます。
        // また、各メンバ変数を参照することで、飜、符、成立役といった情報を個別に取得できます。
        // 詳しくは types/result.hpp を参照してください。
        std::cout << ret.to_string() << std::endl;
    }
}

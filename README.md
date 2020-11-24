# mahjong-cpp

## 概要

日本のリーチ麻雀のプログラム置き場

Miscellaneous programs about Japanese Mahjong

## 内容

* [x] 向聴数計算
* [x] 点数計算
* [ ] 有効牌計算
* [ ] 期待値計算

## ビルドに必要なライブラリ

* [Boost C++ Libraries](https://www.boost.org/) >= 1.61
* [CMake](https://cmake.org/) >= 3.1.1

## 使い方

### 向聴数計算

```cpp
#include "mahjong/mahjong.hpp"

using namespace mahjong;

int main(int, char **)
{
    Hand hand({Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu2,
                Tile::AkaManzu5, Tile::Manzu6, Tile::Manzu7, Tile::Manzu8,
                Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu2,
                Tile::Pinzu2});

    auto [syanten_type, syanten] = SyantenCalculator::calc(hand);

    std::cout << fmt::format("手牌: {}, 向聴数の種類: {}, 向聴数: {}",
                                hand.to_string(), SyantenType::Name[syanten_type],
                                syanten)
                << std::endl;
}
```

### 点数計算

```cpp
#include "mahjong/mahjong.hpp"

using namespace mahjong;

int main(int, char **)
{
    ScoreCalculator score;

    // 場やルールの設定
    score.set_bakaze(Tile::Ton);  // 場風牌
    score.set_zikaze(Tile::Ton);  // 自風牌
    score.set_num_tumibo(0);  // 積み棒の数
    score.set_num_kyotakubo(0);  // 供託棒の数
    score.set_dora_tiles({Tile::Pe});  // ドラの一覧 (表示牌ではない)
    score.set_uradora_tiles({Tile::Pinzu9});  // 裏ドラの一覧 (表示牌ではない)

    // 手牌、和了牌、フラグの設定
    // 手牌
    MeldedBlock block(MeldType::Kakan, {Tile::Ton, Tile::Ton, Tile::Ton, Tile::Ton});
    Hand hand({Tile::Manzu1, Tile::Manzu2, Tile::Manzu3, Tile::Pinzu3, Tile::Pinzu4,
                Tile::Pinzu5, Tile::Sozu1, Tile::Sozu2, Tile::Sozu3, Tile::Sozu4, Tile::Sozu4},
                {block});
    int win_tile = Tile::Manzu1;  // 和了牌
    int flag = HandFlag::Tumo | HandFlag::Rinsyankaiho;  // フラグ

    // 点数計算
    Result ret = score.calc(hand, win_tile, flag);
    std::cout << ret.to_string() << std::endl;
}
```

## ベンチマーク

* Core™ i9-9900K 3.6 GHz

### 向聴数計算

|      | N=40000 | 1手あたりの平均計算時間  |
|------|--------------------|-------------|
| 一般手  | 1.87ms              | 46ns |
| 七対子手 | 45ms               | 5.9ns |
| 国士手  | 23ms               | 3.4ns |

### 点数計算

|      | N=410831 | 1手あたりの平均計算時間  |
|------|--------------------|-------------|
| 一般手  | 128ms              | 311ns |

## テスト

### 向聴数計算

4万手のテストケースで一般手、七対子手、国士無双手の向聴数が一致することを確認

### 点数計算

天鳳の41万手のテストケースで飜、符、役、点数が一致することを確認

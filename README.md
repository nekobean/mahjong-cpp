# mahjong-cpp

## About

日本のリーチ麻雀のルールで、点数や期待値計算を行う C++ ライブラリです。

Miscellaneous programs about Japanese Mahjong

## 麻雀何切るシミュレーター

このライブラリを使った期待値計算機能を Web アプリにしたものを以下に公開しています。

[麻雀何切るシミュレーター](https://pystyle.info/apps/mahjong-nanikiru-simulator/)

![麻雀何切るシミュレーター](docs/mahjong-nanikiru-simulator.png)

- アプリの紹介: [麻雀 - 期待値計算ツール 何切るシミュレーター](https://pystyle.info/mahjong-nanikiru-simulator/)
- 期待値計算の詳細: [麻雀における期待値の計算方法](https://pystyle.info/mahjong-expected-value-in-mahjong/)

## Features

🚧This program is under development. Currently the following features have been implemented.🚧

- [x] Syanten Number Calculation (向聴数計算)
- [x] Score Calculation (点数計算)
- [x] Required Tile Selection (有効牌列挙)
- [x] Unnecessary Tile Selection (不要牌列挙)
- [x] Expected Score Calculation (期待値計算)

## Requirements

- C++17 (See [C++ compiler support - cppreference.com](https://en.cppreference.com/w/cpp/compiler_support))
- [Boost C++ Libraries](https://www.boost.org/) >= 1.66
- [CMake](https://cmake.org/) >= 3.5

## How to build

### Linux

Clone repogitory and build program.

```bash
git clone https://github.com/nekobean/mahjong-cpp.git
cd mahjong-cpp
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Run sample program.

```bash
cd build/src/samples
./sample_calculate_expexted_value
./sample_calculate_score
./sample_calculate_syanten
./sample_required_tile_selector
./sample_unnecessary_tile_selector
```

### Build on Docker container

Build and run container.

```bash
docker build . --tag mahjong_cpp_build
docker run -it mahjong_cpp_build
```

Build program on the created container.

```bash
git clone https://github.com/nekobean/mahjong-cpp.git
cd mahjong-cpp
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## Usage

- [Syanten Number Calculation (向聴数計算)](src/samples/sample_shanten_number_calculation.cpp)
- [Score Calculation (点数計算)](src/samples/sample_score_calculation.cpp)
- [Necessary Tile Calculation (有効牌計算)](src/samples/sample_necessary_tile_calculation.cpp)
- [Unnecessary Tile Selection (不要牌選択)](src/samples/sample_necessary_tile_calculation.cpp)

### Shanten number calculation

```cpp
#include <iostream>

#include "mahjong/mahjong.hpp"

int main(int argc, char *argv[])
{
    using namespace mahjong;

    // Create hand by mpsz notation or vector of tiles.
    Hand hand = from_mpsz("222567m34p33667s");
    // Hand hand = from_array({Tile::Manzu2, Tile::Manzu2, Tile::Manzu2, Tile::Manzu5,
    //                          Tile::Manzu6, Tile::Manzu7, Tile::Pinzu3, Tile::Pinzu4,
    //                          Tile::Souzu3, Tile::Souzu3, Tile::Souzu6, Tile::Souzu6,
    //                          Tile::Souzu7});
    // number of melds.
    int num_melds = 0;
    // Calculate minimum shanten number of regular hand, Seven Pairs and Thirteen Orphans.
    auto [shanten_type, shanten] =
        ShantenCalculator::calc(hand, num_melds, ShantenFlag::All);
    std::cout << "shanten type: " << ShantenFlag::Name.at(shanten_type)
              << std::endl;
    std::cout << "shanten: " << shanten << std::endl;
}
```

### Neccesary tile calculation

```cpp
#include <iostream>

#include "mahjong/mahjong.hpp"

int main(int argc, char *argv[])
{
    using namespace mahjong;

    // Create hand by mpsz notation or vector of tiles.
    Hand hand = from_mpsz("222567m34p33667s");
    // number of melds.
    int num_melds = 0;

    // Calculate necessary tiles.
    auto [shanten_type, shanten, tiles] =
        NecessaryTileCalculator::select(hand, num_melds, ShantenFlag::All);

    std::cout << "shanten: " << shanten << std::endl;
    for (auto tile : tiles) {
        std::cout << Tile::Name.at(tile) + " ";
    }
    std::cout << std::endl;
}
```

### Unnecessary tile calculation

```cpp
#include <iostream>

#include "mahjong/mahjong.hpp"

int main(int argc, char *argv[])
{
    using namespace mahjong;

    // Create hand by mpsz notation or vector of tiles.
    Hand hand = from_mpsz("222567m34p33667s1z");
    // number of melds.
    int num_melds = 0;

    // Calculate unnecessary tiles.
    auto [shanten_type, shanten, tiles] =
        UnnecessaryTileCalculator::select(hand, num_melds, ShantenFlag::All);

    std::cout << "shanten: " << shanten << std::endl;
    for (auto tile : tiles) {
        std::cout << Tile::Name.at(tile) + " ";
    }
    std::cout << std::endl;
}

```

### Score Calculation

```cpp
#include <iostream>

#include "mahjong/mahjong.hpp"

int main(int argc, char *argv[])
{
    using namespace mahjong;

    // Set round infomation.
    Round round;
    round.rules = RuleFlag::RedDora | RuleFlag::OpenTanyao;
    round.wind = Tile::East;
    round.kyoku = 1;
    round.honba = 0;
    round.kyotaku = 1;
    round.dora_tiles = {Tile::North};
    round.uradora_tiles = {Tile::Pinzu9};

    // Set player information
    Hand hand = from_mpsz("222567345p333s22z");
    Player player;
    player.hand = hand;
    player.melds = {};
    player.wind = Tile::East;
    const int win_tile = Tile::South;
    const int flag = WinFlag::Tsumo | WinFlag::Riichi;

    // Calculate score.
    const Result result = ScoreCalculator::calc(round, player, win_tile, flag);
    std::cout << to_string(result) << std::endl;
}
```

```output
[入力]
手牌: 222345567p333s22z
副露牌:
自風: 1z
自摸
[結果]
面子構成: [222p 暗刻子][345p 暗順子][567p 暗順子][333s 暗刻子][22z 暗対子]
待ち: 単騎待ち
役:
 門前清自摸和 1翻
 立直 1翻
40符2翻
和了者の獲得点数: 4900点, 子の支払い点数: 1300点
```

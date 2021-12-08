# mahjong-cpp

## About (概要)

日本のリーチ麻雀のルールで、点数や期待値計算を行う C++ ライブラリです。

Miscellaneous programs about Japanese Mahjong

## 麻雀何切るシミュレーター

このライブラリを使った期待値計算機能を Web アプリにしたものを以下に公開しています。

[麻雀何切るシミュレーター](https://pystyle.info/apps/mahjong-nanikiru-simulator/)

![麻雀何切るシミュレーター](docs/mahjong-nanikiru-simulator.png)

* アプリの紹介: [麻雀 - 期待値計算ツール 何切るシミュレーター](https://pystyle.info/mahjong-nanikiru-simulator/)
* 期待値計算の詳細: [麻雀における期待値の計算方法](https://pystyle.info/mahjong-expected-value-in-mahjong/)

## Features (機能)

🚧This program is under development. Currently the following features have been implemented.🚧

* [x] 向聴数計算 (Syanten Number Calculation)
* [x] 点数計算 (Score Calculation)
* [x] 有効牌列挙 (Required Tile Selection)
* [x] 不要牌列挙 (Unnecessary Tile Selection)
* [x] 期待値計算 (Expected Score Calculation)
  * [x] 向聴戻し考慮
  * [x] 手変わり考慮
  * [x] ダブル立直、一発、海底撈月考慮
  * [x] 裏ドラ考慮 (平和形と仮定した場合の近似)
  * [x] 副露している手牌に対応
  * [x] 赤ドラ対応

## Requirements (依存ライブラリ)

* C++17 (See [C++ compiler support - cppreference.com](https://en.cppreference.com/w/cpp/compiler_support))
  * e.x. Microsoft Visual Studio Community 2019 Version 16.7.2
  * e.x. gcc 9.3.0
* [Boost C++ Libraries](https://www.boost.org/) >= 1.66
* [CMake](https://cmake.org/) >= 3.1.1

## How to build (ビルド方法)

Clone repogitory and build program.

```bash
git clone https://github.com/nekobean/mahjong-cpp.git
cd mahjong-cpp
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Run sample program.

```
cd build/src/samples
./sample_calculate_expexted_value
./sample_calculate_score
./sample_calculate_syanten
./sample_required_tile_selector
./sample_unnecessary_tile_selector
```

## Usage (使い方)

* [向聴数計算 (Syanten Number Calculation)](src/samples/sample_calculate_syanten.cpp)
* [点数計算 (Score Calculation)](src/samples/sample_calculate_score.cpp)
* [有効牌選択 (Required Tile Selection)](src/samples/sample_required_tile_selector.cpp)
* [不要牌選択 (Unnecessary Tile Selection)](src/samples/sample_unnecessary_tile_selector.cpp)
* [期待値計算 (Expected Value Calculation)](src/samples/sample_calculate_expexted_value.cpp)

### 点数計算の例

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

```output
[結果]
手牌: 123m 345p 12344s [東東東東, 加槓], 和了牌: 一萬, 自摸
面子構成:
  [東東東東, 明槓子]
  [一萬二萬三萬, 暗順子]
  [三筒四筒五筒, 暗順子]
  [一索二索三索, 暗順子]
  [四索四索, 暗対子]
待ち: 両面待ち
役:
 嶺上開花 1翻
 自風 (東) 1翻
 場風 (東) 1翻
40符3翻
```

### 期待値計算の例

```
手牌: 123349m3688p1245s, 向聴数: 2, 巡目: 1
[打 二索] 有効牌: 17種60枚, 聴牌確率: 71.65%, 和了確率: 23.30%, 期待値: 1509.20  (向聴戻し)
[打 一索] 有効牌: 20種70枚, 聴牌確率: 70.21%, 和了確率: 22.67%, 期待値: 1475.87  (向聴戻し)
[打 九萬] 有効牌:  4種15枚, 聴牌確率: 64.34%, 和了確率: 20.50%, 期待値: 1379.59
[打 三筒] 有効牌:  4種15枚, 聴牌確率: 64.06%, 和了確率: 20.05%, 期待値: 1250.20
[打 六筒] 有効牌:  4種15枚, 聴牌確率: 62.63%, 和了確率: 19.09%, 期待値: 1294.03
[打 八筒] 有効牌: 16種50枚, 聴牌確率: 59.84%, 和了確率: 15.47%, 期待値:  939.12  (向聴戻し)
[打 四索] 有効牌: 18種64枚, 聴牌確率: 59.93%, 和了確率: 15.29%, 期待値: 1049.23  (向聴戻し)
[打 五索] 有効牌: 19種66枚, 聴牌確率: 58.87%, 和了確率: 14.79%, 期待値: 1047.75  (向聴戻し)
[打 一萬] 有効牌: 18種62枚, 聴牌確率: 52.48%, 和了確率: 13.61%, 期待値:  873.10  (向聴戻し)
[打 三萬] 有効牌: 19種66枚, 聴牌確率: 51.87%, 和了確率: 13.13%, 期待値:  908.70  (向聴戻し)
[打 四萬] 有効牌: 18種62枚, 聴牌確率: 50.94%, 和了確率: 12.80%, 期待値:  931.94  (向聴戻し)
[打 二萬] 有効牌:  4種15枚, 聴牌確率: 24.66%, 和了確率:  4.25%, 期待値:  278.19  (向聴戻し)
計算時間: 33542us
```

## Benchmark (ベンチマーク)

* Core™ i9-9900K 3.6 GHz

### 向聴数計算

|          | N=40000 | 1手あたりの平均計算時間 |
| -------- | ------- | ----------------------- |
| 一般手   | 576 us  | 14.4ns                  |
| 七対子手 | 240 us  | 6.0ns                   |
| 国士手   | 147 us  | 3.7ns                   |

* 検証: 40000パターンのテストケースがパスすることを確認

### 点数計算

|        | N=410831 | 1手あたりの平均計算時間 |
| ------ | -------- | ----------------------- |
| 一般手 | 128ms    | 311ns                   |

* 検証: 実践の牌譜から取得した1486960パターンのテストケースがパスすることを確認

### 不要牌選択

|          | N=100000 | 1手あたりの平均計算時間 |
| -------- | -------- | ----------------------- |
| 一般手   | 33 ms    | 330ns                   |
| 七対子手 | 21 ms    | 210ns                   |
| 国士手   | 12 ms    | 120ns                   |

### 有効牌選択

|          | N=100000 | 1手あたりの平均計算時間 |
| -------- | -------- | ----------------------- |
| 一般手   | 62 ms    | 620ns                   |
| 七対子手 | 35 ms    | 350ns                   |
| 国士手   | 13 ms    | 130ns                   |

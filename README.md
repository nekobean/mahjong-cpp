# mahjong-cpp

## 概要

麻雀関係のプログラム置き場

Miscellaneous programs about Japanese Mahjong

## 内容

* [x] 向聴数計算
* [ ] 有効牌計算
* [ ] 期待値計算
* [ ] 牌譜解析スクリプト

## 依存ライブラリ

* [Boost C++ Libraries](https://www.boost.org/) >= 1.61

## 向聴数計算

* ハッシュテーブル、手牌のビット表記を利用して高速化

### 速度

|      | 計算時間 (N=4000000) | 1手あたりの計算計算  |
|------|--------------------|-------------|
| 一般手  | 183ms              | 0.000046ms |
| 七対子手 | 45ms               | 0.000011ms |
| 国士手  | 23ms               | 0.000005ms |

* Core™ i9-9900K 3.6 GHz
* 手牌を初期化した際のハッシュ計算は除く

### サンプル

src/main.cpp

```cpp
// 手牌入力
Tehai tehai({Tile::Manzu1, Tile::Manzu1, Tile::Manzu1, Tile::Manzu2, Tile::Manzu5, Tile::Manzu6,
             Tile::Manzu7, Tile::Manzu8, Tile::Manzu9, Tile::Pinzu1, Tile::Pinzu1, Tile::Pinzu2,
             Tile::Pinzu2});

int syanten = SyantenCalculator::calc(tehai);
std::cout << tehai << " " << syanten << std::endl;
```

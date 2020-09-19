#ifndef MAHJONG_CPP_FUURO
#define MAHJONG_CPP_FUURO

#include "types.hpp"

namespace mahjong
{
// const std::string FUURO_TYPE_NAME[] = {
//     "鳴いていない", "ポン", "赤入りポン", "チー", "赤入りチー", "暗槓", "明槓", "加槓",
// };

/**
 * @brief 副露の種類
 */
enum FuuroType {
    Null,    // NULL
    Pon,     //ポン
    AkaPon,  //赤牌を含むポン
    Chii,    //チー
    AkaChii, //赤牌を含むチー
    Ankan,   //暗槓
    Minkan,  //明槓
    Kakan,   //加槓
};

/**
 * @brief 副露ブロック
 */
struct FuuroBlock {
    FuuroType type; //副露タイプ
    int minhai;     //構成牌のうち最小の牌
    int nakihai;    //鳴いた牌
    int from;       //どこから鳴いたか

    //コンストラクタで初期化
    FuuroBlock()
    {
        type = FuuroType::Null;
        nakihai = 0;
        minhai = 0;
        from = -1;
    }

    //副露ブロックの判別

    //=!演算子
    bool operator!=(FuuroBlock &fuuro)
    {
        return !(this->type == fuuro.type && this->minhai == fuuro.minhai &&
                 this->nakihai == fuuro.nakihai && this->from == fuuro.from);
    }

    //==演算子
    bool operator==(FuuroBlock &fuuro)
    {
        return (this->type == fuuro.type && this->minhai == fuuro.minhai &&
                this->nakihai == fuuro.nakihai && this->from == fuuro.from);
    }
};

} // namespace mahjong

#endif /* MAHJONG_CPP_FUURO */

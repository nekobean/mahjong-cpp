#ifndef MAHJONG_CPP_MELDEDBLOCK
#define MAHJONG_CPP_MELDEDBLOCK

#include <map>
#include <string>
#include <vector>

#include <boost/operators.hpp>

#include "tile.hpp"

namespace mahjong
{

/**
 * @brief 副露の種類
 */
namespace MeldType
{

enum
{
    Null = -1,
    Pon,    /* ポン */
    Ti,     /* チー */
    Ankan,  /* 暗槓 */
    Minkan, /* 明槓 */
    Kakan,  /* 加槓 */
    Length,
};

static inline const std::map<int, std::string> Name = {{Null, "Null"},   {Pon, "ポン"},
                                                       {Ti, "チー"},     {Ankan, "暗槓"},
                                                       {Minkan, "明槓"}, {Kakan, "加槓"}};

} // namespace MeldType

/**
 * @brief プレイヤーの種類
 */
namespace PlayerType
{

enum
{
    Null = -1,
    Player0,
    Player1,
    Player2,
    Player3,
    Length,
};

static inline const std::map<int, std::string> Name = {{Null, "Null"},
                                                       {Player0, "プレイヤー1"},
                                                       {Player1, "プレイヤー2"},
                                                       {Player2, "プレイヤー3"},
                                                       {Player3, "プレイヤー4"}};

} // namespace PlayerType

/**
 * @brief 座席の種類
 */
namespace SeatType
{

enum
{
    Null = -1,
    Zitya,   /* 自家 */
    Kamitya, /* 上家 */
    Toimen,  /* 対面 */
    Simotya, /* 下家 */
    Length,
};

static inline const std::map<int, std::string> Name = {
    {Null, "Null"}, {Zitya, "自家"}, {Kamitya, "上家"}, {Toimen, "対面"}, {Simotya, "下家"}};

} // namespace SeatType

/**
 * @brief 副露ブロック
 */
struct MeldedBlock : private boost::equality_comparable<MeldedBlock, MeldedBlock>
{
    MeldedBlock() : type(MeldType::Null), discarded_tile(Tile::Null), from(PlayerType::Null) {}

    MeldedBlock(int type, std::vector<int> tiles)
        : type(type)
        , tiles(tiles)
        , discarded_tile(!tiles.empty() ? tiles.front() : Tile::Null)
        , from(PlayerType::Player0)
    {
    }

    MeldedBlock(int type, std::vector<int> tiles, int discarded_tile, int from)
        : type(type), tiles(tiles), discarded_tile(discarded_tile), from(from)
    {
    }

    std::string to_string() const;

    /*! 副露の種類 */
    int type;

    /*! 構成牌 */
    std::vector<int> tiles;

    /*! 鳴いた牌 */
    int discarded_tile;

    /*! 鳴かれたプレイヤー */
    int from;

    friend bool operator==(const MeldedBlock &a, const MeldedBlock &b);
};

inline bool operator==(const MeldedBlock &a, const MeldedBlock &b)
{
    return a.tiles.size() == b.tiles.size() &&
           std::equal(a.tiles.begin(), a.tiles.end(), b.tiles.begin()) && a.type == b.type &&
           a.discarded_tile == b.discarded_tile && a.from == b.from;
}

/**
 * @brief 文字列に変換する。
 *
 * @return std::string ブロックを表す文字列
 */
inline std::string MeldedBlock::to_string() const
{
    std::string s;

    s += "[";
    for (auto tile : tiles) {
        if (is_akahai(tile))
            s += "r5";
        else if (is_manzu(tile))
            s += std::to_string(tile + 1);
        else if (is_pinzu(tile))
            s += std::to_string(tile - 8);
        else if (is_sozu(tile))
            s += std::to_string(tile - 17);
        else
            s += Tile::Name.at(tile);
    }

    if (is_manzu(tiles[0]))
        s += "m";
    else if (is_pinzu(tiles[0]))
        s += "p";
    else if (is_sozu(tiles[0]))
        s += "s";
    s += fmt::format(", {}]", MeldType::Name.at(type));

    return s;
}

} // namespace mahjong

#endif /* MAHJONG_CPP_MELDEDBLOCK */

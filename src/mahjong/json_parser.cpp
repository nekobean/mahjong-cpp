#include "json_parser.hpp"

#include <sstream>

#include <rapidjson/istreamwrapper.h>
#include <rapidjson/ostreamwrapper.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

using namespace mahjong;

/**
 * @brief JSON ドキュメントを文字列にする。
 * 
 * @param[in] doc ドキュメント
 * @return std::string JSON ドキュメント
 */
std::string to_json_str(rapidjson::Document &doc)
{
    std::stringstream ss;
    rapidjson::OStreamWrapper osw(ss);
    rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
    doc.Accept(writer);

    return ss.str();
}

/**
 * @brief JSON データを解析する。
 *
 * @param[in] doc ドキュメント
 * @return リクエストデータ
 */
RequestData parse_request(const rapidjson::Value &doc)
{
    RequestData req;

    req.zikaze = doc["zikaze"].GetInt();
    req.bakaze = doc["bakaze"].GetInt();
    req.turn = doc["turn"].GetInt();
    req.syanten_type = doc["syanten_type"].GetInt();

    for (auto &x : doc["dora_tiles"].GetArray())
        req.dora_tiles.push_back(x.GetInt());

    req.flag = doc["flag"].GetInt();

    std::vector<int> hand_tiles;
    for (auto &x : doc["hand_tiles"].GetArray())
        hand_tiles.push_back(x.GetInt());

    std::vector<MeldedBlock> melds;
    for (auto &meld : doc["melded_blocks"].GetArray()) {
        int meld_type = meld["type"].GetInt();

        std::vector<int> tiles;
        for (auto &x : meld["tiles"].GetArray())
            tiles.push_back(x.GetInt());

        int discarded_tile = meld["discarded_tile"].GetInt();
        int from = meld["from"].GetInt();

        melds.emplace_back(meld_type, tiles, discarded_tile, from);
    }
    req.hand = Hand(hand_tiles, melds);

    if (doc.HasMember("ip"))
        req.ip = doc["ip"].GetString();

    return req;
}

/**
 * @brief JSON データを解析する。
 *
 * @param[in] doc ドキュメント
 * @return リクエストデータ
 */
DiscardResponseData parse_response(const rapidjson::Value &value)
{
    DiscardResponseData res;

    res.syanten = value["syanten"].GetInt();
    res.time_us = value["time"].GetInt();
    for (auto &x : value["candidates"].GetArray()) {
        int tile = x["tile"].GetInt();
        bool syanten_down = x["syanten_down"].GetBool();

        std::vector<std::tuple<int, int>> required_tiles;
        for (auto &y : x["required_tiles"].GetArray()) {
            int tile = y["tile"].GetInt();
            int count = y["count"].GetInt();
            required_tiles.emplace_back(tile, count);
        }

        std::vector<double> exp_values, win_probs, tenpai_probs;
        for (auto &y : x["tenpai_probs"].GetArray())
            tenpai_probs.push_back(y.GetDouble());
        for (auto &y : x["win_probs"].GetArray())
            win_probs.push_back(y.GetDouble());
        for (auto &y : x["exp_values"].GetArray())
            exp_values.push_back(y.GetDouble());

        res.candidates.emplace_back(tile, required_tiles, tenpai_probs, win_probs, exp_values,
                                    syanten_down);
    }

    return res;
}

rapidjson::Value dump_required_tiles(const std::vector<std::tuple<int, int>> &tiles,
                                     rapidjson::Document &doc)
{
    rapidjson::Value value(rapidjson::kArrayType);
    for (auto [tile, count] : tiles) {
        rapidjson::Value x(rapidjson::kObjectType);
        x.AddMember("tile", tile, doc.GetAllocator());
        x.AddMember("count", count, doc.GetAllocator());
        value.PushBack(x, doc.GetAllocator());
    }

    return value;
}

rapidjson::Value dump_candidate(const Candidate &candidate, rapidjson::Document &doc)
{
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("tile", candidate.tile, doc.GetAllocator());
    value.AddMember("syanten_down", candidate.syanten_down, doc.GetAllocator());
    value.AddMember("required_tiles", dump_required_tiles(candidate.required_tiles, doc),
                    doc.GetAllocator());

    if (!candidate.exp_values.empty()) {
        value.AddMember("exp_values", rapidjson::kArrayType, doc.GetAllocator());
        for (auto x : candidate.exp_values)
            value["exp_values"].PushBack(x, doc.GetAllocator());
    }

    if (!candidate.win_probs.empty()) {
        value.AddMember("win_probs", rapidjson::kArrayType, doc.GetAllocator());
        for (auto x : candidate.win_probs)
            value["win_probs"].PushBack(x, doc.GetAllocator());
    }

    if (!candidate.tenpai_probs.empty()) {
        value.AddMember("tenpai_probs", rapidjson::kArrayType, doc.GetAllocator());
        for (auto x : candidate.tenpai_probs)
            value["tenpai_probs"].PushBack(x, doc.GetAllocator());
    }

    return value;
}

DrawResponseData create_draw_response(const RequestData &req)
{
    ScoreCalculator score_calc;
    ExpectedValueCalculator exp_value_calc;

    // 13枚の場合は有効牌を求める。
    auto begin = std::chrono::steady_clock::now();

    // 向聴数を計算する。
    auto [syanten_type, syanten] = SyantenCalculator::calc(req.hand, req.syanten_type);

    // 各牌の残り枚数を数える。
    std::vector<int> counts = ExpectedValueCalculator::count_left_tiles(req.hand, req.dora_tiles);

    // 有効牌を求める。
    auto [total_count, required_tiles] =
        ExpectedValueCalculator::get_required_tiles(req.hand, req.syanten_type, counts);

    auto end = std::chrono::steady_clock::now();
    auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

    DrawResponseData res;
    res.syanten = syanten;
    res.time_us = elapsed_us;
    res.required_tiles = required_tiles;

    return res;
}

DiscardResponseData create_discard_response(const RequestData &req)
{
    ScoreCalculator score_calc;
    ExpectedValueCalculator exp_value_calc;

    auto begin = std::chrono::steady_clock::now();

    // 向聴数を計算する。
    auto [syanten_type, syanten] = SyantenCalculator::calc(req.hand, req.syanten_type);

    // 点数計算の設定
    score_calc.set_bakaze(req.bakaze);
    score_calc.set_zikaze(req.zikaze);
    score_calc.set_num_tumibo(0);
    score_calc.set_num_kyotakubo(0);
    score_calc.set_dora_tiles(req.dora_tiles);

    // 各打牌を分析する。
    auto [success, candidates] =
        exp_value_calc.calc(req.hand, score_calc, req.syanten_type, req.flag);

    auto end = std::chrono::steady_clock::now();
    auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

    DiscardResponseData res;
    res.syanten = syanten;
    res.time_us = elapsed_us;
    res.candidates = candidates;

    return res;
}

rapidjson::Value dump_draw_response(const DrawResponseData &res, rapidjson::Document &doc)
{
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("result_type", 0, doc.GetAllocator());
    value.AddMember("syanten", res.syanten, doc.GetAllocator());
    value.AddMember("time", res.time_us, doc.GetAllocator());
    value.AddMember("required_tiles", dump_required_tiles(res.required_tiles, doc),
                    doc.GetAllocator());

    return value;
}

rapidjson::Value dump_discard_response(const DiscardResponseData &res, rapidjson::Document &doc)
{
    rapidjson::Value value(rapidjson::kObjectType);
    value.AddMember("result_type", 1, doc.GetAllocator());
    value.AddMember("syanten", res.syanten, doc.GetAllocator());
    value.AddMember("time", res.time_us, doc.GetAllocator());
    value.AddMember("candidates", rapidjson::kArrayType, doc.GetAllocator());
    for (const auto &candidate : res.candidates)
        value["candidates"].PushBack(dump_candidate(candidate, doc), doc.GetAllocator());

    return value;
}

rapidjson::Value create_response(const RequestData &req, rapidjson::Document &doc)
{
    // 手牌の枚数を求める。
    int n_tiles = req.hand.num_tiles() + int(req.hand.melds.size()) * 3;

    if (n_tiles == 13) {
        DrawResponseData res = create_draw_response(req);
        return dump_draw_response(res, doc);
    }
    else if (n_tiles == 14) {
        DiscardResponseData res = create_discard_response(req);
        return dump_discard_response(res, doc);
    }
}

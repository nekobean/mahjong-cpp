#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

class SyantenTableGenerator
{
  public:
    std::vector<int> calc_pair_with_head(const std::vector<int> &key);
    std::vector<int> calc_pair(const std::vector<int> &key);
    std::vector<int> count(const std::vector<int> &key) const;

  private:
    void cut_mentu(std::vector<int> &key, int n_mentu = 0, int n_kouho = 0, int i = 0);
    void cut_kouho(std::vector<int> &key, int n_mentu = 0, int n_kouho = 0, int i = 0);
    void aggregate(int n_mentu, int n_kouho);

  private:
    int max_pair;
    int max_mentu;
    int max_kouho;
};

std::vector<int> SyantenTableGenerator::calc_pair_with_head(const std::vector<int> &_key)
{
    std::vector<int> key = _key;
    max_pair = max_mentu = max_kouho = 0;
    int head = 0;

    for (size_t i = 0; i < key.size(); ++i) {
        if (key[i] >= 2) {
            key[i] -= 2;
            cut_mentu(key);
            key[i] += 2;

            head = 1;
        }
    }

    return {head, max_mentu, max_kouho};
}

std::vector<int> SyantenTableGenerator::calc_pair(const std::vector<int> &_key)
{
    std::vector<int> key = _key;
    max_pair = max_mentu = max_kouho = 0;
    cut_mentu(key);

    return {max_mentu, max_kouho};
}

void SyantenTableGenerator::cut_mentu(std::vector<int> &key, int n_mentu, int n_kouho, int i)
{
    if (i == key.size()) {
        cut_kouho(key, n_mentu, n_kouho);
        return;
    }

    if (key[i] >= 3) {
        key[i] -= 3;
        cut_mentu(key, n_mentu + 1, n_kouho, i);
        key[i] += 3;
    }

    if (key.size() == 9 && i < key.size() - 2 && key[i] && key[i + 1] && key[i + 2]) {
        key[i] -= 1;
        key[i + 1] -= 1;
        key[i + 2] -= 1;
        cut_mentu(key, n_mentu + 1, n_kouho, i);
        key[i] += 1;
        key[i + 1] += 1;
        key[i + 2] += 1;
    }

    cut_mentu(key, n_mentu, n_kouho, i + 1);
}

void SyantenTableGenerator::cut_kouho(std::vector<int> &key, int n_mentu, int n_kouho, int i)
{
    if (i == key.size()) {
        aggregate(n_mentu, n_kouho);
        return;
    }

    if (key[i] >= 2) {
        key[i] -= 2;
        cut_kouho(key, n_mentu, n_kouho + 1, i);
        key[i] += 2;
    }

    if (key.size() == 9 && i < key.size() - 1 && key[i] && key[i + 1]) {
        key[i] -= 1;
        key[i + 1] -= 1;
        cut_kouho(key, n_mentu, n_kouho + 1, i);
        key[i] += 1;
        key[i + 1] += 1;
    }

    if (key.size() == 9 && i < key.size() - 2 && key[i] && key[i + 2]) {
        key[i] -= 1;
        key[i + 2] -= 1;
        cut_kouho(key, n_mentu, n_kouho + 1, i);
        key[i] += 1;
        key[i + 2] += 1;
    }

    cut_kouho(key, n_mentu, n_kouho, i + 1);
}

void SyantenTableGenerator::aggregate(int n_mentu, int n_kouho)
{
    int pair = n_mentu * 2 + n_kouho;

    if (pair > max_pair || (pair == max_pair && n_mentu > max_mentu)) {
        max_pair = pair;
        max_mentu = n_mentu;
        max_kouho = n_kouho;
    }
}

std::vector<int> SyantenTableGenerator::count(const std::vector<int> &key) const
{
    int n_ge1 = 0;
    int n_ge2 = 0;
    int n_ge3 = 0;
    int n_ge4 = 0;

    for (auto n : key) {
        n_ge1 += n >= 1;
        n_ge2 += n >= 2;
        n_ge3 += n >= 3;
        n_ge4 += n >= 4;
    }

    return {n_ge1, n_ge2, n_ge3, n_ge4};
}

class Product
{
  public:
    std::vector<std::vector<int>> generate(int n_keys)
    {
        keys_.clear();
        n_keys_ = n_keys;
        std::vector<int> key(n_keys);

        product(key);

        return keys_;
    }

  private:
    void product(std::vector<int> &key, int i = 0, int cnt = 0)
    {
        if (i == n_keys_) {
            if (cnt <= 14)
                keys_.push_back(key);
            return;
        }

        for (int n = 0; n < 5; ++n) {
            key[i] = n;
            product(key, i + 1, cnt + n);
        }
    }

  private:
    std::vector<std::vector<int>> keys_;
    int n_keys_;
};

int main()
{
    Product product;
    SyantenTableGenerator gen;

    {

        // 数牌のテーブルを作成する。
        std::vector<std::vector<int>> keys = product.generate(9);
        std::ofstream file(R"(F:\work\mahjong-cpp\data\config\syupai_table.txt)",
                           std::ios_base::binary | std::ios_base::out);
        for (const auto &key : keys) {
            auto v1 = gen.calc_pair(key);
            auto v2 = gen.calc_pair_with_head(key);
            auto v3 = gen.count(key);

            // ファイルに出力する。
            for (size_t i = 0; i < 9; ++i) {
                file << (i < key.size() ? key[i] : 0);
            }
            file << " ";

            for (auto x : v1)
                file << x;
            for (auto x : v2)
                file << x;
            for (auto x : v3)
                file << x;
            file << "\n";
        }
    }

    {
        // 字牌のテーブルを作成する。
        std::vector<std::vector<int>> keys = product.generate(7);
        std::ofstream file(R"(F:\work\mahjong-cpp\data\config\zihai_table.txt)",
                           std::ios_base::binary | std::ios_base::out);
        for (const auto &key : keys) {
            auto v1 = gen.calc_pair(key);
            auto v2 = gen.calc_pair_with_head(key);
            auto v3 = gen.count(key);

            // ファイルに出力する。
            for (size_t i = 0; i < 9; ++i) {
                file << (i < key.size() ? key[i] : 0);
            }
            file << " ";

            for (auto x : v1)
                file << x;
            for (auto x : v2)
                file << x;
            for (auto x : v3)
                file << x;
            file << "\n";
        }
    }
}

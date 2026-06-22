#include "table.hpp"

#include <boost/dll.hpp>

namespace mahjong
{

Table::Table()
{
    initialize();
}

/**
 * @brief Initialize the table.
 *
 * @return Returns true if initialization is successful, otherwise false.
 */
bool Table::initialize()
{
    boost::filesystem::path exe_path = boost::dll::program_location().parent_path();
#ifdef USE_NYANTEN_TABLE
    boost::filesystem::path suits_table_path = exe_path / "suits_table_nyanten.bin";
    boost::filesystem::path honors_table_path = exe_path / "honors_table_nyanten.bin";
#else
    boost::filesystem::path suits_table_path = exe_path / "suits_table.bin";
    boost::filesystem::path honors_table_path = exe_path / "honors_table.bin";
#endif

    return load_table(suits_table_path.string(), suits_table_) &&
           load_table(honors_table_path.string(), honors_table_);
}

std::array<Table::TableType, Table::SuitsTableSize> Table::suits_table_;
std::array<Table::TableType, Table::HonorsTableSize> Table::honors_table_;
std::array<Table::TableType, Table::SanmaManzuTableSize> Table::sanma_manzu_table_ = {{
    Table::TableType{{0,   3,   6, 15, 15, 2, 5, 15, 15, 15, 0, 257, 257, 0, 0,
                      257, 257, 0, 0,  0,  0, 0, 0,  0,  0,  0, 0,   0,   0, 0}},
    Table::TableType{{0,   2,   5, 15, 15, 1,   4, 15, 15, 15, 0, 256, 257, 0, 0,
                      256, 257, 0, 0,  0,  256, 0, 0,  0,  0,  0, 0,   0,   0, 0}},
    Table::TableType{{0, 1,   4, 15, 15, 0,   3, 15, 15, 15, 0, 256, 257, 0, 0,
                      0, 257, 0, 0,  0,  256, 0, 0,  0,  0,  0, 0,   0,   0, 0}},
    Table::TableType{{0, 0, 3, 15, 15, 0,   2, 15, 15, 15, 0,   0, 1, 0, 0,
                      0, 1, 0, 0,  0,  256, 0, 0,  0,  0,  256, 0, 0, 0, 0}},
    Table::TableType{{0, 0, 3, 15, 15, 0,   2,   15,  15, 15, 0,   0,   1, 0, 0,
                      0, 1, 0, 0,  0,  256, 256, 256, 0,  0,  256, 256, 0, 0, 0}},
    Table::TableType{{0, 2,   5, 15, 15, 1, 4, 15, 15, 15, 0, 1, 257, 0, 0,
                      1, 257, 0, 0,  0,  1, 0, 0,  0,  0,  0, 0, 0,   0, 0}},
    Table::TableType{{0,   2,   4, 15, 15, 1,   3,   15, 15, 15, 0,   257, 257, 0, 0,
                      257, 257, 0, 0,  0,  257, 257, 0,  0,  0,  257, 0,   0,   0, 0}},
    Table::TableType{{0, 1,   3, 15, 15, 0,   2, 15, 15, 15, 0, 256, 257, 0, 0,
                      0, 257, 0, 0,  0,  257, 1, 0,  0,  0,  1, 0,   0,   0, 0}},
    Table::TableType{{0, 0, 2, 15, 15, 0,   1, 15, 15, 15, 0,   0, 1, 0, 0,
                      0, 1, 0, 0,  0,  257, 1, 0,  0,  0,  257, 0, 0, 0, 0}},
    Table::TableType{{0, 0, 2, 15, 15, 0,   1,   15,  15, 15, 0,   0,   1, 0, 0,
                      0, 1, 0, 0,  0,  257, 257, 256, 0,  0,  257, 256, 0, 0, 0}},
    Table::TableType{{0, 1,   4, 15, 15, 0, 3, 15, 15, 15, 0, 1, 257, 0, 0,
                      0, 257, 0, 0,  0,  1, 0, 0,  0,  0,  0, 0, 0,   0, 0}},
    Table::TableType{{0, 1,   3, 15, 15, 0,   2,   15, 15, 15, 0,   1, 257, 0, 0,
                      0, 257, 0, 0,  0,  257, 256, 0,  0,  0,  256, 0, 0,   0, 0}},
    Table::TableType{{0, 1,   2, 15, 15, 0,   1,   15, 15, 15, 0,   257, 257, 0, 0,
                      0, 257, 0, 0,  0,  257, 257, 0,  0,  0,  257, 0,   0,   0, 0}},
    Table::TableType{{0, 0, 1, 15, 15, 0,   0, 15, 15, 15, 0,   0, 1, 0, 0,
                      0, 0, 0, 0,  0,  257, 1, 0,  0,  0,  257, 0, 0, 0, 0}},
    Table::TableType{{0, 0, 1, 15, 15, 0,   0,   15,  15, 15, 0,   0,   1, 0, 0,
                      0, 0, 0, 0,  0,  257, 257, 256, 0,  0,  257, 256, 0, 0, 0}},
    Table::TableType{{0, 0,   3, 15, 15, 0, 2, 15, 15, 15, 0, 0, 256, 0, 0,
                      0, 256, 0, 0,  0,  1, 0, 0,  0,  0,  1, 0, 0,   0, 0}},
    Table::TableType{{0, 0,   2, 15, 15, 0,   1,   15, 15, 15, 0,   0, 256, 0, 0,
                      0, 256, 0, 0,  0,  257, 256, 0,  0,  0,  257, 0, 0,   0, 0}},
    Table::TableType{{0, 0, 1, 15, 15, 0,   0,   15, 15, 15, 0,   0, 256, 0, 0,
                      0, 0, 0, 0,  0,  257, 256, 0,  0,  0,  257, 0, 0,   0, 0}},
    Table::TableType{{0, 0, 0, 15, 15, 0,   0,   15, 15, 15, 0,   0,   0, 0, 0,
                      0, 0, 0, 0,  0,  257, 257, 0,  0,  0,  257, 257, 0, 0, 0}},
    Table::TableType{{0, 0, 0, 15, 15, 0,   0,   15,  15, 15, 0,   0,   0, 0, 0,
                      0, 0, 0, 0,  0,  257, 257, 256, 0,  0,  257, 257, 0, 0, 0}},
    Table::TableType{{0, 0,   3, 15, 15, 0, 2, 15, 15, 15, 0, 0, 256, 0, 0,
                      0, 256, 0, 0,  0,  1, 1, 1,  0,  0,  1, 1, 0,   0, 0}},
    Table::TableType{{0, 0,   2, 15, 15, 0,   1,   15, 15, 15, 0,   0, 256, 0, 0,
                      0, 256, 0, 0,  0,  257, 257, 1,  0,  0,  257, 1, 0,   0, 0}},
    Table::TableType{{0, 0, 1, 15, 15, 0,   0,   15, 15, 15, 0,   0, 256, 0, 0,
                      0, 0, 0, 0,  0,  257, 257, 1,  0,  0,  257, 1, 0,   0, 0}},
    Table::TableType{{0, 0, 0, 15, 15, 0,   0,   15, 15, 15, 0,   0,   0, 0, 0,
                      0, 0, 0, 0,  0,  257, 257, 1,  0,  0,  257, 257, 0, 0, 0}},
    Table::TableType{{0, 0, 0, 15, 15, 0,   0,   15,  15, 15, 0,   0,   0, 0, 0,
                      0, 0, 0, 0,  0,  257, 257, 257, 0,  0,  257, 257, 0, 0, 0}},
}};

static Table inst;

} // namespace mahjong

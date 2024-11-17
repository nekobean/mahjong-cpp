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

static Table inst;

} // namespace mahjong

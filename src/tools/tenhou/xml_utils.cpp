#include "xml_utils.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

#include <zlib.h>

namespace mahjong::tools::tenhou::detail
{

namespace
{

const char *attr_value(const rapidxml::xml_node<> &node, const char *name)
{
    const auto *attr = node.first_attribute(name);
    return attr ? attr->value() : nullptr;
}

std::vector<int> split_ints(const std::string &value)
{
    std::vector<int> ret;
    size_t pos = 0;
    while (pos < value.size()) {
        const size_t next = value.find(',', pos);
        const auto token =
            value.substr(pos, next == std::string::npos ? next : next - pos);
        if (!token.empty()) {
            ret.push_back(std::stoi(token));
        }
        if (next == std::string::npos) {
            break;
        }
        pos = next + 1;
    }
    return ret;
}

std::vector<double> split_doubles(const std::string &value)
{
    std::vector<double> ret;
    size_t pos = 0;
    while (pos < value.size()) {
        const size_t next = value.find(',', pos);
        const auto token =
            value.substr(pos, next == std::string::npos ? next : next - pos);
        if (!token.empty()) {
            ret.push_back(std::stod(token));
        }
        if (next == std::string::npos) {
            break;
        }
        pos = next + 1;
    }
    return ret;
}

std::vector<std::string> split_strings(const std::string &value)
{
    if (value.empty()) {
        return {};
    }

    std::vector<std::string> ret;
    size_t pos = 0;
    while (pos < value.size()) {
        const size_t next = value.find(',', pos);
        ret.push_back(value.substr(pos, next == std::string::npos ? next : next - pos));
        if (next == std::string::npos) {
            break;
        }
        pos = next + 1;
    }
    return ret;
}

bool is_gzip(const std::vector<char> &buffer)
{
    return buffer.size() >= 2 && static_cast<unsigned char>(buffer[0]) == 0x1F &&
           static_cast<unsigned char>(buffer[1]) == 0x8B;
}

std::vector<char> inflate_gzip(const std::vector<char> &buffer)
{
    z_stream stream{};
    stream.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(buffer.data()));
    stream.avail_in = static_cast<uInt>(buffer.size());

    if (inflateInit2(&stream, MAX_WBITS + 16) != Z_OK) {
        throw std::runtime_error("Failed to initialize gzip decompressor.");
    }

    std::vector<char> ret;
    std::vector<char> chunk(64 * 1024);
    int status = Z_OK;
    while (status == Z_OK) {
        stream.next_out = reinterpret_cast<Bytef *>(chunk.data());
        stream.avail_out = static_cast<uInt>(chunk.size());

        status = inflate(&stream, Z_NO_FLUSH);
        const auto written = chunk.size() - stream.avail_out;
        ret.insert(ret.end(), chunk.begin(), chunk.begin() + written);
    }

    inflateEnd(&stream);
    if (status != Z_STREAM_END) {
        throw std::runtime_error("Failed to decompress gzip mjlog file.");
    }

    ret.push_back('\0');
    return ret;
}

} // namespace

std::vector<char> read_xml_file(const std::filesystem::path &path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open mjlog file: " + path.string());
    }

    file.seekg(0, std::ios::end);
    const auto size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(static_cast<size_t>(size));
    file.read(buffer.data(), size);
    if (is_gzip(buffer)) {
        return inflate_gzip(buffer);
    }

    buffer.push_back('\0');
    return buffer;
}

std::string attr_string(const rapidxml::xml_node<> &node, const char *name)
{
    const char *value = attr_value(node, name);
    return value ? value : "";
}

std::vector<std::string> attr_strings(const rapidxml::xml_node<> &node,
                                      const char *name)
{
    const char *value = attr_value(node, name);
    return value ? split_strings(value) : std::vector<std::string>{};
}

int attr_int(const rapidxml::xml_node<> &node, const char *name)
{
    const char *value = attr_value(node, name);
    if (!value) {
        throw std::runtime_error("Missing required XML attribute: " +
                                 std::string(name));
    }
    return std::stoi(value);
}

std::vector<int> attr_ints(const rapidxml::xml_node<> &node, const char *name)
{
    const char *value = attr_value(node, name);
    return value ? split_ints(value) : std::vector<int>{};
}

double attr_double(const rapidxml::xml_node<> &node, const char *name)
{
    const char *value = attr_value(node, name);
    if (!value) {
        throw std::runtime_error("Missing required XML attribute: " +
                                 std::string(name));
    }
    return std::stod(value);
}

std::vector<double> attr_doubles(const rapidxml::xml_node<> &node, const char *name)
{
    const char *value = attr_value(node, name);
    return value ? split_doubles(value) : std::vector<double>{};
}

std::vector<std::vector<int>> attr_hands(const rapidxml::xml_node<> &node)
{
    std::vector<std::vector<int>> ret;
    ret.reserve(4);
    for (int i = 0; i < 4; ++i) {
        ret.push_back(attr_ints(node, ("hai" + std::to_string(i)).c_str()));
    }
    return ret;
}

} // namespace mahjong::tools::tenhou::detail

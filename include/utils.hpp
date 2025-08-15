#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <stdexcept>
#include <regex>
#include <type_traits>
#include <cstring>
#include <iterator>
#include <memory>
#include <cstdint>
#include <chrono>
#include <ctime>
#include <random>
#include <functional>
#include <map>
#include <set>
#include <list>
#include <array>
#include <queue>
#include <stack>
#include <fstream>
#include <iostream>
#include <initializer_list>
#include <limits>
#include <numeric>
#include <cstdlib>
#include <array>
#include <memory>

namespace utils
{

    // ======================== 字符串处理 ========================
    namespace string
    {

        // 去除字符串首尾空白字符
        inline std::string trim(const std::string &str)
        {
            if (str.empty())
                return "";

            size_t start = 0;
            size_t end = str.size() - 1;

            while (start <= end && std::isspace(static_cast<unsigned char>(str[start])))
                ++start;
            while (end >= start && std::isspace(static_cast<unsigned char>(str[end])))
                --end;

            return (start > end) ? "" : str.substr(start, end - start + 1);
        }

        // 字符串分割
        inline std::vector<std::string> split(const std::string &str, char delimiter)
        {
            std::vector<std::string> tokens;
            std::string token;
            std::istringstream tokenStream(str);

            while (std::getline(tokenStream, token, delimiter))
            {
                if (!token.empty())
                {
                    tokens.push_back(token);
                }
            }
            return tokens;
        }

        // 字符串连接
        template <typename Iter>
        inline std::string join(Iter begin, Iter end, const std::string &delimiter = "")
        {
            std::ostringstream os;
            if (begin != end)
            {
                os << *begin++;
                while (begin != end)
                {
                    os << delimiter << *begin++;
                }
            }
            return os.str();
        }

        // 检查字符串是否以指定前缀开头
        inline bool startsWith(const std::string &str, const std::string &prefix)
        {
            if (prefix.size() > str.size())
                return false;
            return std::equal(prefix.begin(), prefix.end(), str.begin());
        }

        // 检查字符串是否以指定后缀结尾
        inline bool endsWith(const std::string &str, const std::string &suffix)
        {
            if (suffix.size() > str.size())
                return false;
            return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
        }

        // 字符串替换
        inline std::string replaceAll(std::string str,
                                      const std::string &from,
                                      const std::string &to)
        {
            if (from.empty())
                return str;

            size_t start_pos = 0;
            while ((start_pos = str.find(from, start_pos)) != std::string::npos)
            {
                str.replace(start_pos, from.length(), to);
                start_pos += to.length();
            }
            return str;
        }

        // 转换为小写
        inline std::string toLower(std::string str)
        {
            std::transform(str.begin(), str.end(), str.begin(),
                           [](unsigned char c)
                           { return std::tolower(c); });
            return str;
        }

        // 转换为大写
        inline std::string toUpper(std::string str)
        {
            std::transform(str.begin(), str.end(), str.begin(),
                           [](unsigned char c)
                           { return std::toupper(c); });
            return str;
        }

    } // namespace string
    // ======================== 基础类型转换 ========================
    namespace convert
    {

        // 字符串转数值类型
        template <typename T>
        inline T toNumber(const std::string &str, bool hex = false)
        {
            static_assert(std::is_arithmetic<T>::value, "T must be numeric type");

            std::stringstream ss;
            if (hex)
                ss << std::hex;
            ss << ::utils::string::trim(str);

            T result;
            if (!(ss >> result))
            {
                throw std::invalid_argument("Invalid numeric conversion: " + str);
            }
            return result;
        }

        // 通用类型转字符串
        template <typename T>
        inline std::string toString(T value)
        {
            std::stringstream ss;

            if constexpr (std::is_floating_point_v<T>)
            {
                // 浮点数：使用 fixed 格式 + 最大精度
                ss << std::fixed << std::setprecision(std::numeric_limits<T>::max_digits10) << value;
            }
            else
            {
                // 非浮点数：直接输出
                ss << value;
            }
            return ss.str();
        }

        // 浮点数格式化 (保留小数)
        template <typename T>
        inline std::string toFixed(T value, int precision = 2)
        {
            static_assert(std::is_floating_point<T>::value, "T must be floating point type");

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(precision) << value;
            return oss.str();
        }

    } // namespace convert

    // ======================== 正则表达式工具 ========================
    namespace regex
    {

        // 正则匹配
        inline bool match(const std::string &input, const std::string &pattern)
        {
            try
            {
                std::regex re(pattern);
                return std::regex_match(input, re);
            }
            catch (const std::regex_error &e)
            {
                throw std::runtime_error("Regex error: " + std::string(e.what()));
            }
        }

        // 正则搜索
        inline bool search(const std::string &input, const std::string &pattern)
        {
            try
            {
                std::regex re(pattern);
                return std::regex_search(input, re);
            }
            catch (const std::regex_error &e)
            {
                throw std::runtime_error("Regex error: " + std::string(e.what()));
            }
        }

        // 正则替换
        inline std::string replace(const std::string &input,
                                   const std::string &pattern,
                                   const std::string &replacement)
        {
            try
            {
                std::regex re(pattern);
                return std::regex_replace(input, re, replacement);
            }
            catch (const std::regex_error &e)
            {
                throw std::runtime_error("Regex error: " + std::string(e.what()));
            }
        }

        // 正则提取所有匹配项
        inline std::vector<std::string> extract(const std::string &input,
                                                const std::string &pattern)
        {
            try
            {
                std::regex re(pattern);
                auto begin = std::sregex_iterator(input.begin(), input.end(), re);
                auto end = std::sregex_iterator();

                std::vector<std::string> matches;
                for (std::sregex_iterator i = begin; i != end; ++i)
                {
                    std::smatch match = *i;
                    matches.push_back(match.str());
                }
                return matches;
            }
            catch (const std::regex_error &e)
            {
                throw std::runtime_error("Regex error: " + std::string(e.what()));
            }
        }

    } // namespace regex

    // ======================== HEX 转换工具 ========================
    namespace hex
    {

        // 单个字符转HEX
        inline std::string charToHex(unsigned char c, bool uppercase = true)
        {
            std::ostringstream oss;
            if (uppercase)
            {
                oss << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<unsigned>(c);
            }
            else
            {
                oss << std::nouppercase << std::hex << std::setw(2) << std::setfill('0')
                    << static_cast<unsigned>(c);
            }
            return oss.str();
        }

        // 字符串转HEX
        inline std::string strToHex(const std::string &input, bool uppercase = true,
                                    const std::string &delimiter = "")
        {
            std::ostringstream oss;
            for (size_t i = 0; i < input.size(); ++i)
            {
                oss << charToHex(static_cast<unsigned char>(input[i]), uppercase);
                if (i < input.size() - 1)
                    oss << delimiter;
            }
            return oss.str();
        }

        // 数字转HEX字符串
        template <typename T>
        inline std::string numToHex(T value, bool uppercase = true, bool prefix = true)
        {
            static_assert(std::is_integral<T>::value, "T must be integral type");

            std::ostringstream oss;
            if (prefix)
                oss << "0x";
            if (uppercase)
            {
                oss << std::uppercase << std::hex << value;
            }
            else
            {
                oss << std::nouppercase << std::hex << value;
            }
            return oss.str();
        }

        // 容器转HEX
        template <typename Container>
        inline std::string containerToHex(const Container &data, bool uppercase = true,
                                          const std::string &delimiter = " ")
        {
            std::ostringstream oss;
            bool first = true;
            for (const auto &item : data)
            {
                if (!first)
                    oss << delimiter;
                oss << charToHex(static_cast<unsigned char>(item), uppercase);
                first = false;
            }
            return oss.str();
        }

        // HEX字符串转字节数组
        inline std::vector<unsigned char> toBytes(const std::string &hex)
        {
            std::vector<unsigned char> bytes;
            if (hex.empty())
                return bytes;

            std::string hex_clean = ::utils::regex::replace(hex, "[^0-9A-Fa-f]", "");
            if (hex_clean.size() % 2 != 0)
            {
                throw std::invalid_argument("Invalid hex string length");
            }

            for (size_t i = 0; i < hex_clean.size(); i += 2)
            {
                std::string byteStr = hex_clean.substr(i, 2);
                unsigned char byte = static_cast<unsigned char>(
                    std::stoul(byteStr, nullptr, 16));
                bytes.push_back(byte);
            }
            return bytes;
        }

    } // namespace hex
    // ======================== 文件路径处理 ========================
    namespace path
    {

        // 获取文件名（含扩展名）
        inline std::string filename(const std::string &path)
        {
            size_t pos = path.find_last_of("/\\");
            return (pos == std::string::npos) ? path : path.substr(pos + 1);
        }

        // 获取文件扩展名
        inline std::string extension(const std::string &path)
        {
            std::string name = filename(path);
            size_t pos = name.find_last_of('.');
            return (pos == std::string::npos) ? "" : name.substr(pos + 1);
        }

        // 获取目录路径
        inline std::string directory(const std::string &path)
        {
            size_t pos = path.find_last_of("/\\");
            return (pos == std::string::npos) ? "" : path.substr(0, pos);
        }

        // 合并路径
        inline std::string combine(const std::string &path1, const std::string &path2)
        {
            if (path1.empty())
                return path2;
            if (path2.empty())
                return path1;

            char last = path1[path1.size() - 1];
            return (last == '/' || last == '\\') ? path1 + path2 : path1 + '/' + path2;
        }

    } // namespace path

    // ======================== 时间日期工具 ========================
    namespace time
    {

        // 获取当前时间戳（毫秒）
        inline uint64_t timestamp()
        {
            using namespace std::chrono;
            return duration_cast<milliseconds>(
                       system_clock::now().time_since_epoch())
                .count();
        }

        // 格式化时间
        inline std::string format(const std::string &fmt = "%Y-%m-%d %H:%M:%S")
        {
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::tm *now_tm = std::localtime(&now_c);

            char buf[100];
            std::strftime(buf, sizeof(buf), fmt.c_str(), now_tm);
            return std::string(buf);
        }

        // 时间戳转格式化时间
        inline std::string format(uint64_t timestamp, const std::string &fmt = "%Y-%m-%d %H:%M:%S")
        {
            std::time_t time = static_cast<time_t>(timestamp / 1000);
            std::tm *tm_ptr = std::localtime(&time);

            char buf[100];
            std::strftime(buf, sizeof(buf), fmt.c_str(), tm_ptr);
            return std::string(buf);
        }

    } // namespace time

    // ======================== 随机数生成 ========================
    namespace random
    {

        // 生成随机整数 [min, max]
        template <typename T>
        inline T integer(T min, T max)
        {
            static_assert(std::is_integral<T>::value, "T must be integral type");
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<T> dist(min, max);
            return dist(gen);
        }

        // 生成随机浮点数 [min, max]
        template <typename T>
        inline T real(T min, T max)
        {
            static_assert(std::is_floating_point<T>::value, "T must be floating point type");
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<T> dist(min, max);
            return dist(gen);
        }

        // 生成随机字符串
        inline std::string string(size_t length, const std::string &charset =
                                                     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789")
        {
            if (charset.empty())
                return "";

            std::string result;
            result.reserve(length);

            for (size_t i = 0; i < length; ++i)
            {
                result += charset[integer<size_t>(0, charset.size() - 1)];
            }
            return result;
        }

    } // namespace random

    // ======================== 文件操作工具 ========================
    namespace file
    {

        // 读取文件全部内容
        inline std::string readAll(const std::string &filename)
        {
            std::ifstream file(filename, std::ios::binary);
            if (!file)
            {
                throw std::runtime_error("Cannot open file: " + filename);
            }

            std::ostringstream ss;
            ss << file.rdbuf();
            return ss.str();
        }

        // 写入文件内容
        inline bool writeAll(const std::string &filename, const std::string &content)
        {
            std::ofstream file(filename, std::ios::binary);
            if (!file)
                return false;

            file << content;
            return !!file;
        }

        // 检查文件是否存在
        inline bool exists(const std::string &filename)
        {
            std::ifstream file(filename);
            return file.good();
        }

    } // namespace file

    // ======================== 容器操作工具 ========================
    namespace container
    {

        // 检查容器是否包含元素
        template <typename Container, typename T>
        inline bool contains(const Container &container, const T &value)
        {
            return std::find(std::begin(container), std::end(container), value) != std::end(container);
        }

        // 过滤容器元素
        template <typename Container, typename Predicate>
        inline Container filter(const Container &container, Predicate pred)
        {
            Container result;
            std::copy_if(std::begin(container), std::end(container),
                         std::back_inserter(result), pred);
            return result;
        }

        // 映射容器元素
        template <typename Container, typename Function>
        inline auto map(const Container &container, Function func)
            -> std::vector<decltype(func(*container.begin()))>
        {
            using ResultType = decltype(func(*container.begin()));
            std::vector<ResultType> result;
            std::transform(std::begin(container), std::end(container),
                           std::back_inserter(result), func);
            return result;
        }

        // 容器元素连接为字符串
        template <typename Container>
        inline std::string join(const Container &container, const std::string &delimiter = "")
        {
            return utils::string::join(std::begin(container), std::end(container), delimiter);
        }

    } // namespace container

    // ======================== 安全转换工具 ========================
    namespace safe
    {

        // 安全数值转换（避免溢出）
        template <typename To, typename From>
        inline To numeric_cast(From value)
        {
            if (value < static_cast<From>(std::numeric_limits<To>::min()))
            {
                throw std::overflow_error("Value underflow in numeric_cast");
            }
            if (value > static_cast<From>(std::numeric_limits<To>::max()))
            {
                throw std::overflow_error("Value overflow in numeric_cast");
            }
            return static_cast<To>(value);
        }

        // 安全指针转换
        template <typename To, typename From>
        inline std::shared_ptr<To> dynamic_pointer_cast(const std::shared_ptr<From> &from)
        {
            auto ptr = std::dynamic_pointer_cast<To>(from);
            if (!ptr)
                throw std::bad_cast();
            return ptr;
        }

    } // namespace safe

    // ======================== 系统工具 ========================
    namespace system
    {

        // 获取环境变量
        inline std::string env(const std::string &name, const std::string &defaultValue = "")
        {
            const char *value = std::getenv(name.c_str());
            return value ? std::string(value) : defaultValue;
        }

        // 执行系统命令并获取输出
        inline std::string execute(const std::string &cmd)
        {
            std::array<char, 128> buffer;
            std::string result;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

            if (!pipe)
            {
                throw std::runtime_error("popen() failed!");
            }

            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
            {
                result += buffer.data();
            }
            return result;
        }

    } // namespace system

    // ======================== 数学工具 ========================
    namespace math
    {

        // 线性插值
        template <typename T>
        inline T lerp(T a, T b, float t)
        {
            return a + t * (b - a);
        }

        // 约束值在范围内
        template <typename T>
        inline T clamp(T value, T min, T max)
        {
            return (value < min) ? min : (value > max) ? max
                                                       : value;
        }

        // 计算平均值
        template <typename Container>
        inline auto average(const Container &c) -> decltype(*c.begin() + *c.begin())
        {
            if (c.empty())
                return 0;
            auto sum = std::accumulate(c.begin(), c.end(), decltype (*c.begin())(0));
            return sum / static_cast<decltype(sum)>(c.size());
        }

        // 计算平方和
        template <typename Container>
        inline auto sumOfSquares(const Container &c) -> decltype(*c.begin() * *c.begin())
        {
            return std::accumulate(c.begin(), c.end(), decltype (*c.begin())(0),
                                   [](auto acc, auto val)
                                   { return acc + val * val; });
        }

    } // namespace math

    // ======================== 调试工具 ========================
    namespace debug
    {

        // 打印容器内容
        template <typename Container>
        inline void printContainer(const Container &c, const std::string &name = "",
                                   std::ostream &os = std::cout)
        {
            if (!name.empty())
                os << name << ": ";
            os << "[";
            for (auto it = c.begin(); it != c.end(); ++it)
            {
                os << *it;
                if (std::next(it) != c.end())
                    os << ", ";
            }
            os << "]" << std::endl;
        }

        // 获取变量类型名称
        template <typename T>
        inline std::string typeName()
        {
            return typeid(T).name();
        }

    } // namespace debug

    // ======================== 向后兼容别名 ========================

    // 为原始函数提供别名以保持向后兼容
    inline std::string trim(const std::string &str) { return string::trim(str); }
    template <typename T>
    inline T toNumber(const std::string &str, bool hex = false)
    {
        return convert::toNumber<T>(str, hex);
    }
    template <typename T>
    inline std::string toString(T value)
    {
        return convert::toString(value);
    }
    template <typename T>
    inline std::string toFixed(T value, int precision = 2)
    {
        return convert::toFixed(value, precision);
    }
    inline std::vector<std::string> split(const std::string &str, char delimiter)
    {
        return string::split(str, delimiter);
    }
    template <typename Iter>
    inline std::string join(Iter begin, Iter end, const std::string &delimiter = "")
    {
        return string::join(begin, end, delimiter);
    }
    inline bool startsWith(const std::string &str, const std::string &prefix)
    {
        return string::startsWith(str, prefix);
    }
    inline bool endsWith(const std::string &str, const std::string &suffix)
    {
        return string::endsWith(str, suffix);
    }
    inline std::string replaceAll(std::string str, const std::string &from, const std::string &to)
    {
        return string::replaceAll(str, from, to);
    }
    inline std::string toLower(std::string str) { return string::toLower(str); }
    inline std::string toUpper(std::string str) { return string::toUpper(str); }
    inline bool regexMatch(const std::string &input, const std::string &pattern)
    {
        return regex::match(input, pattern);
    }
    inline bool regexSearch(const std::string &input, const std::string &pattern)
    {
        return regex::search(input, pattern);
    }
    inline std::string regexReplace(const std::string &input, const std::string &pattern,
                                    const std::string &replacement)
    {
        return regex::replace(input, pattern, replacement);
    }
    inline std::vector<std::string> regexExtract(const std::string &input,
                                                 const std::string &pattern)
    {
        return regex::extract(input, pattern);
    }

} // namespace utils

#endif // UTILS_H
// color_utils.h
#pragma once
#include <string>

namespace ansi
{
    constexpr const char *reset = "\033[0m";
    constexpr const char *bold = "\033[1m";
    constexpr const char *dim = "\033[2m";
    constexpr const char *underline = "\033[4m";
    constexpr const char *blink = "\033[5m";
    constexpr const char *reverse = "\033[7m";
    constexpr const char *hidden = "\033[8m";

    constexpr const char *black = "\033[30m";
    constexpr const char *red = "\033[31m";
    constexpr const char *green = "\033[32m";
    constexpr const char *yellow = "\033[33m";
    constexpr const char *blue = "\033[34m";
    constexpr const char *magenta = "\033[35m";
    constexpr const char *cyan = "\033[36m";
    constexpr const char *white = "\033[37m";

    constexpr const char *bg_black = "\033[40m";
    constexpr const char *bg_red = "\033[41m";
    constexpr const char *bg_green = "\033[42m";
    constexpr const char *bg_yellow = "\033[43m";
    constexpr const char *bg_blue = "\033[44m";
    constexpr const char *bg_magenta = "\033[45m";
    constexpr const char *bg_cyan = "\033[46m";
    constexpr const char *bg_white = "\033[47m";

    inline std::string remove_ansi_escape(const std::string &input)
    {
        std::string result;
        result.reserve(input.size());

        size_t pos = 0;
        while (pos < input.size())
        {
            if (input[pos] == '\033' && pos + 1 < input.size() && input[pos + 1] == '[')
            {
                // 找到 ANSI 序列开始
                size_t end = pos + 2;
                while (end < input.size())
                {
                    // 序列以字母结束
                    if (input[end] >= '@' && input[end] <= '~')
                    {
                        pos = end + 1; // 跳过整个序列
                        break;
                    }
                    end++;
                }
                if (end >= input.size())
                    break;
            }
            else
            {
                result.push_back(input[pos++]);
            }
        }

        return result;
    }
}

// 自定义文本着色函数
inline std::string colorize_text(const std::string &text,
                                 const std::string &fg_color = "",
                                 const std::string &bg_color = "")
{
    if (fg_color.empty() && bg_color.empty())
    {
        return text;
    }
    return fg_color + bg_color + text + ansi::reset;
}

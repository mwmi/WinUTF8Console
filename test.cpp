#include "utf8stream.h"

int main() {
    ucout << sizeof(char32_t) / sizeof(char) << uendl;
    ucout << "===测试不同类型的字符串输出显示===" << uendl << uendl;

    auto s = "😁😀😂 123 一二三 abc";
    auto ws = L"😁😀😂 123 一二三 abc";
    auto u32s = U"😁😀😂 123 一二三 abc";
    ucout << "string:   " << s << uendl;
    ucout << "wstring:  " << ws << uendl;
    ucout << "u32string:" << u32s << uendl;

    ucout << uendl << "===测试不同类型的字符串输入输出===" << uendl;

    ucout << uendl << "请输入3个字符串以空格隔开: " << uendl;
    std::string a, b, c;
    ucin >> a >> b >> c;
    ucout << uendl << "输出内容: “" << a << "”  “" << b << "”  “" << c << "”" << uendl;

    ucout << uendl << "请输入一行字符串: " << uendl;
    const auto line = ucin.readLine<std::wstring>();
    ucout << uendl << "输出内容: “" << line << "”" << uendl;

    ucout << uendl << "请输入多行字符串: " << uendl;
    const auto lines = ucin.readLines<std::u32string>(true);
    ucout << "输出内容: " << uendl;
    ucout << "=========================" << uendl;
    ucout << lines << uendl;
    ucout << "=========================" << uendl;
    return 0;
}
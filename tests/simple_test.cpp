/**
 * @file simple_test.cpp
 * @brief 简单的UTF8控制台功能测试
 */

#include "../utf8stream.h"
#include <string>

int main() {
    // 测试基本输出
    ucout << "=== 简单测试 ===" << uendl;
    
    // 测试不同字符串类型输出
    std::string str = "Hello 世界";
    std::wstring wstr = L"Hello 世界 😁";
    std::u32string u32str = U"Hello 世界 😀";
    
    ucout << "std::string: " << str << uendl;
    ucout << "std::wstring: " << wstr << uendl;
    ucout << "std::u32string: " << u32str << uendl;
    
    ucout << "简单测试完成" << uendl;
    
    return 0;
}
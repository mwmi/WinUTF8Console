/**
 * @file utf8_console_tests.cpp
 * @brief UTF8控制台输入输出功能测试文件
 */

#include "../utf8stream.h"
#include <string>
#include <vector>
#include <cstdio>

// 测试字符串转换功能
void test_string_conversions() {
    ucout << "测试字符串转换功能..." << uendl;
    
    // 测试宽字符串到UTF-8的转换
    std::wstring wstr = L"Hello 世界";
    std::string utf8_str = wstring_to_utf8(wstr);
    std::wstring converted_back = utf8_to_wstring(utf8_str);
    
    // 测试UTF-32到UTF-8的转换
    std::u32string u32str = U"Hello 世界";
    std::string utf8_from_u32 = u32string_to_utf8(u32str);
    std::u32string converted_u32_back = utf8_to_u32string(utf8_from_u32);
    
    // 测试UTF-32到宽字符串的转换
    std::wstring wstr_from_u32 = u32string_to_wstring(u32str);
    
    ucout << "字符串转换功能测试完成" << uendl;
}

// 测试输出功能
void test_output_functions() {
    ucout << "测试输出功能..." << uendl;
    
    // 测试不同字符串类型的输出
    std::string str = "Hello 世界";
    std::wstring wstr = L"Hello 世界";
    std::u32string u32str = U"Hello 世界";
    
    // 这些调用应该能正常编译和执行
    ucout << "std::string: " << str << uendl;
    ucout << "std::wstring: " << wstr << uendl;
    ucout << "std::u32string: " << u32str << uendl;
    
    ucout << "输出功能测试通过" << uendl;
}

// 测试输入功能
void test_input_functions() {
    ucout << "测试输入功能..." << uendl;
    
    // 使用freopen重定向stdin到test_input.txt文件
    FILE* original_stdin = stdin;
    FILE* redirected_input = nullptr;
    
    // 尝试在当前目录打开测试输入文件
    redirected_input = freopen("test_input.txt", "r", stdin);
    
    if (!redirected_input) {
        // 如果在当前目录找不到，尝试在上一级目录查找
        redirected_input = freopen("../tests/test_input.txt", "r", stdin);
        if (!redirected_input) {
            // 如果在当前目录找不到，尝试在上上级目录查找
            redirected_input = freopen("../../tests/test_input.txt", "r", stdin);
        }
    }
    
    if (redirected_input) {
        // 测试读取不同类型的字符串
        UTF8ConsoleInput input;
        
        // 这些调用应该能正常编译
        std::string str = input.readWord<std::string>();
        std::wstring wstr = input.readWord<std::wstring>();
        std::u32string u32str = input.readWord<std::u32string>();
        
        ucout << "输入功能测试通过" << uendl;
        
        // 恢复原始的stdin
        fclose(redirected_input);
    } else {
        ucout << "无法打开测试输入文件 test_input.txt" << uendl;
    }
}

// 测试向量输出功能
void test_vector_output() {
    ucout << "测试向量输出功能..." << uendl;
    
    std::vector<std::string> lines = {
        "第一行",
        "第二行",
        "第三行"
    };
    
    ucout << "输出字符串向量:" << uendl;
    ucout << lines << uendl;
    
    ucout << "向量输出功能测试通过" << uendl;
}

int main() {
    ucout << "开始UTF8控制台功能测试..." << uendl;
    
    try {
        test_string_conversions();
        test_output_functions();
        test_input_functions();
        test_vector_output();
        
        ucout << "所有测试完成!" << uendl;
        return 0;
    } catch (const std::exception& e) {
        ucout << "测试失败: " << e.what() << uendl;
        return 1;
    }
}
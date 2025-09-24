/**
 * @file input_test.cpp
 * @brief UTF8控制台输入功能测试
 */

#include "../utf8stream.h"
#include <string>
#include <cstdio>

int main() {
    ucout << "=== 输入功能测试开始 ===" << uendl;

    // 使用freopen重定向stdin到test_input.txt文件
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
        try {
            // 测试UTF8ConsoleInput对象的创建
            UTF8ConsoleInput input;

            // 测试读取单词功能
            ucout << "测试读取单词功能..." << uendl;
            std::string word1, word2, word3;
            input >> word1 >> word2 >> word3;
            ucout << "读取到的单词: " << word1 << ", " << word2 << ", " << word3 << uendl;

            // 测试读取行功能
            ucout << "测试读取行功能..." << uendl;
            std::string line = input.readLine<std::string>();
            ucout << "读取到的行: " << line << uendl;

            // 测试读取多行功能
            ucout << "测试读取多行功能..." << uendl;
            auto lines = input.readLines<std::string>(false);
            ucout << "读取到的多行:" << uendl;
            for (const auto& l : lines) {
                ucout << "  " << l << uendl;
            }

            ucout << "=== 输入功能测试完成 ===" << uendl;
        } catch (const std::exception& e) {
            ucout << "测试过程中发生异常: " << e.what() << uendl;
        }

        // 恢复原始的stdin
        fclose(redirected_input);
    } else {
        ucout << "无法打开测试输入文件 test_input.txt" << uendl;
        ucout << "请确保文件存在于当前目录或 ../tests/ 或 ../../tests/ 目录中" << uendl;
        return 1;
    }

    return 0;
}
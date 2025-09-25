#pragma once
#if defined(_MSVC_LANG)
#if _MSVC_LANG < 201703L
#error "This header requires C++17 or higher"
#endif
#elif defined(__cplusplus) && __cplusplus < 201703L
#error "This header requires C++17 or higher"
#endif

#include <Windows.h>
#include <string>
#include <vector>


/**
 * @brief 自定义异常类，继承自std::exception
 *
 * 该类用于封装自定义的异常信息，提供异常消息的存储和获取功能
 */
class UException final : public std::exception {
    std::string msg; // 存储异常消息字符串
public:
    /**
     * @brief 构造函数，使用C风格字符串初始化异常消息
     *
     * @param message 异常消息字符串指针
     */
    explicit UException(const char* message) : msg(message) {
    }

    /**
     * @brief 获取异常消息字符串
     *
     * @return const char* 返回异常消息的C风格字符串表示
     */
    [[nodiscard]] const char* what() const noexcept override {
        return msg.c_str();
    }
};

/**
 * @brief 将宽字符串(std::wstring)转换为UTF-8编码的字符串(std::string)
 * @param wideStr 输入的宽字符串引用
 * @return std::string 转换后的UTF-8编码字符串，转换失败时返回空字符串
 */
inline std::string wstring_to_string(const std::wstring& wideStr) {
    // 检查输入字符串是否为空
    if (wideStr.empty()) return {};

    // 第一次调用WideCharToMultiByte获取转换所需的缓冲区大小
    const int size = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return {};

    // 创建适当大小的字符串容器，减1是因为WideCharToMultiByte返回的大小包含终止符
    std::string str(size - 1, 0);

    // 第二次调用WideCharToMultiByte执行实际的字符串转换
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, str.data(), size, nullptr, nullptr);
    // str.pop_back(); // 移除末尾的null终止符（如果需要保留null，则省略此行）
    return str;
}

/**
 * @brief 将UTF-8编码的字符串转换为宽字符串(std::wstring)
 * @param utf8_str 输入的UTF-8编码字符串
 * @return 转换后的宽字符串，如果转换失败或输入为空则返回空字符串
 */
inline std::wstring string_to_wstring(const std::string& utf8_str) {
    // 检查输入字符串是否为空
    if (utf8_str.empty()) return {};

    // 第1次调用MultiByteToWideChar获取转换所需的缓冲区大小
    const int size = MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, nullptr, 0);
    if (size <= 0) return {};

    // 创建适当大小的字符串容器，减1是因为MultiByteToWideChar返回的大小包含终止符
    std::wstring wideStr(size - 1, 0);

    // 第二次调用MultiByteToWideChar执行实际的字符串转换
    MultiByteToWideChar(CP_UTF8, 0, utf8_str.c_str(), -1, wideStr.data(), size);

    return wideStr;
}

/**
 * @brief 将 UTF-32 字符串转换为宽字符串(std::wstring)
 * @param u32str 输入的 UTF-32 字符串
 * @return 转换后的宽字符串，如果输入为空则返回空字符串
 */
inline std::wstring u32string_to_wstring(const std::u32string& u32str) {
    if (u32str.empty()) return {};

    std::wstring wideStr;
    // 预分配空间以优化性能
    wideStr.reserve(u32str.length() * 2);

    for (char32_t ch : u32str) {
        if (ch <= 0xFFFF) {
            // BMP 字符直接转换
            // 检查是否是代理对范围的非法字符
            if (ch >= 0xD800 && ch <= 0xDFFF) {
                // 这是非法的 UTF-32 字符，因为它映射到 UTF-16 的代理对范围
                // 可以选择跳过或者用替换字符代替
                wideStr.push_back(0xFFFD); // 使用替换字符
            } else {
                wideStr.push_back(static_cast<wchar_t>(ch));
            }
        } else {
            // 处理超出 BMP 的字符，生成高代理和低代理对
            // 检查是否超出 Unicode 最大码点
            if (ch > 0x10FFFF) {
                // 超出 Unicode 范围，使用替换字符
                wideStr.push_back(0xFFFD);
            } else {
                ch -= 0x10000;
                wideStr.push_back(static_cast<wchar_t>(0xD800 + (ch >> 10))); // 高代理
                wideStr.push_back(static_cast<wchar_t>(0xDC00 + (ch & 0x3FF))); // 低代理
            }
        }
    }
    wideStr.shrink_to_fit();
    return wideStr;
}


/**
 * @brief 将 UTF-32 字符串转换为 UTF-8 字符串。
 * @param u32str 输入的 UTF-32 字符串
 * @return 转换后的 UTF-8 字符串；如果输入为空或转换失败，则返回空字符串
 */
inline std::string u32string_to_string(const std::u32string& u32str) {
    if (u32str.empty()) return {};

    std::wstring wideStr = u32string_to_wstring(u32str);

    if (wideStr.empty()) return {};

    // 第一次调用WideCharToMultiByte获取转换所需的缓冲区大小
    const int size = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, nullptr, 0, nullptr, nullptr);

    if (size <= 0) return {};

    // 创建适当大小的字符串容器，减1是因为WideCharToMultiByte返回的大小包含终止符
    std::string str(size - 1, 0);

    // 第二次调用WideCharToMultiByte执行实际的字符串转换
    WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), -1, str.data(), size, nullptr, nullptr);

    return str;
}

/**
 * @brief 将 UTF-8 字符串转换为 UTF-32 字符串。
 * @param utf8_str 输入的 UTF-8 字符串
 * @return 转换后的 UTF-32 字符串
 * @throws UException 如果输入字符串不是合法的 UTF-8 编码
 */
inline std::u32string string_to_u32string(const std::string& utf8_str) {
    std::u32string u32str;
    u32str.reserve(utf8_str.size()); // 预分配空间以优化性能
    for (size_t i = 0; i < utf8_str.size();) {
        if (const unsigned char ch = utf8_str[i]; ch <= 0x7F) {
            // 单字节 ASCII 字符
            u32str += ch;
            i++;
        } else if (ch > 0xBF && ch <= 0xDF) {
            // 双字节 UTF-8 序列
            if (i + 1 >= utf8_str.size()) {
                throw UException("Invalid UTF-8 string");
            }
            const unsigned char ch2 = utf8_str[i + 1];
            if (ch2 < 0x80 || ch2 > 0xBF) {
                throw UException("Invalid UTF-8 string");
            }
            const char32_t codepoint = (ch & 0x1F) << 6 | ch2 & 0x3F;
            u32str += codepoint;
            i += 2;
        } else if (ch <= 0xEF) {
            // 三字节 UTF-8 序列
            if (i + 2 >= utf8_str.size()) {
                throw UException("Invalid UTF-8 string: not enough bytes");
            }
            const unsigned char ch2 = utf8_str[i + 1];
            const unsigned char ch3 = utf8_str[i + 2];
            if (ch2 < 0x80 || ch2 > 0xBF || ch3 < 0x80 || ch3 > 0xBF) {
                throw UException("Invalid UTF-8 string: invalid byte sequence");
            }
            const char32_t codepoint = (ch & 0x0F) << 12 | (ch2 & 0x3F) << 6 | ch3 & 0x3F;
            // 检查是否是过长编码或代理区字符
            if (codepoint < 0x800 || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
                throw UException("Invalid UTF-8 string: invalid codepoint");
            }
            u32str += codepoint;
            i += 3;
        } else if (ch <= 0xF7) {
            // 四字节 UTF-8 序列
            if (i + 3 >= utf8_str.size()) {
                throw UException("Invalid UTF-8 string: not enough bytes");
            }
            const unsigned char ch2 = utf8_str[i + 1];
            const unsigned char ch3 = utf8_str[i + 2];
            const unsigned char ch4 = utf8_str[i + 3];
            if (ch2 < 0x80 || ch2 > 0xBF || ch3 < 0x80 || ch3 > 0xBF || ch4 < 0x80 || ch4 > 0xBF) {
                throw UException("Invalid UTF-8 string: invalid byte sequence");
            }
            const char32_t codepoint = (ch & 0x07) << 18 | (ch2 & 0x3F) << 12 | (ch3 & 0x3F) << 6 | ch4 & 0x3F;
            // 检查是否是过长编码或超出 Unicode 最大码点
            if (codepoint < 0x10000 || codepoint > 0x10FFFF) {
                throw UException("Invalid UTF-8 string: invalid codepoint");
            }
            u32str += codepoint;
            i += 4;
        } else {
            // 超出定义范围的起始字节
            throw UException("Invalid UTF-8 string");
        }
    }
    u32str.shrink_to_fit();
    return u32str;
}

/**
 * 将std::wstring转换为std::u32string
 *
 * @param wstr 输入的宽字符串，假设为UTF-16编码
 * @return 转换后的UTF-32字符串
 * @throws UException 当UTF-16字符串格式无效时抛出异常
 */
inline std::u32string wstring_to_u32string(const std::wstring& wstr) {
    std::u32string u32str;
    u32str.reserve(wstr.size()); // 预分配空间以优化性能

    for (size_t i = 0; i < wstr.size(); i++) {
        const wchar_t ch = wstr[i];

        // 检查是否是基本多文种平面（BMP）字符
        if (ch < 0xD800 || ch > 0xDFFF) {
            // 单个字符（BMP）
            u32str += static_cast<char32_t>(ch);
        } else if (ch <= 0xDBFF) {
            // 高代理项（high surrogate）
            if (i + 1 >= wstr.size()) {
                throw UException("Invalid UTF-16 string: truncated surrogate pair");
            }

            const wchar_t low_surrogate = wstr[i + 1];
            if (low_surrogate < 0xDC00 || low_surrogate > 0xDFFF) {
                throw UException("Invalid UTF-16 string: missing low surrogate");
            }

            // 计算UTF-32码点
            char32_t code_point = 0x10000;
            code_point += (static_cast<char32_t>(ch) - 0xD800) << 10;
            code_point += static_cast<char32_t>(low_surrogate) - 0xDC00;

            u32str += code_point;
            i++; // 跳过低代理项
        } else {
            // 孤立的低代理项（无效）
            throw UException("Invalid UTF-16 string: lone low surrogate");
        }
    }

    u32str.shrink_to_fit();
    return u32str;
}

/// @brief 输出函数
/// @tparam Args 参数类型
/// @param format 格式字符串
/// @param args 参数，注意args字符串类型只支持`char`, 不支持 `wchar_t`
/// @return 成功输出`UTF-8`编码字符占用的字节数，失败时返回负值
template<typename... Args>
int print(const std::string& format, Args... args) {
    return printf_s(format.c_str(), args...);
}

/// @brief 输出函数
/// @tparam Args 参数类型
/// @param format 格式字符串
/// @param args 参数，注意args字符串类型只支持`char`, 不支持 `wchar_t`
/// @return 成功输出`UTF-8`编码字符占用的字节数，失败时返回负值
template<typename... Args>
int print(const std::wstring& format, Args... args) {
    return print(wstring_to_string(format).c_str(), args...);
}

/// @brief 输出函数
/// @tparam Args 参数类型
/// @param format 格式字符串
/// @param args 参数，注意args字符串类型只支持`char`, 不支持 `char32_t`
/// @return 成功输出`UTF-8`编码字符占用的字节数，失败时返回负值
template<typename... Args>
int print(const std::u32string& format, Args... args) {
    return print(u32string_to_string(format).c_str(), args...);
}

/**
 * @brief 打印格式化字符串并在末尾添加换行符（宽字符串版本）
 * @tparam Args 可变参数模板参数包，表示格式化字符串中占位符对应的参数类型
 * @param format 宽字符串格式，包含可选的占位符
 * @param args 可变参数列表，用于替换格式字符串中的占位符
 * @return int 成功输出`UTF-8`编码字符占用的字节数，失败时返回负值
 */
template<typename... Args>
int println(const std::wstring& format, Args... args) {
    return print(format + L'\n', args...);
}

/**
 * @brief 打印格式化字符串并在末尾添加换行符（窄字符串版本）
 * @tparam Args 可变参数模板参数包，表示格式化字符串中占位符对应的参数类型
 * @param format 窄字符串格式，包含可选的占位符
 * @param args 可变参数列表，用于替换格式字符串中的占位符
 * @return int 成功输出`UTF-8`编码字符占用的字节数，失败时返回负值
 */
template<typename... Args>
int println(const std::string& format, Args... args) {
    return print(format + '\n', args...);
}

/**
 * @brief 打印格式化字符串并在末尾添加换行符（窄字符串版本）
 * @tparam Args 可变参数模板参数包，表示格式化字符串中占位符对应的参数类型
 * @param format 窄字符串格式，包含可选的占位符
 * @param args 可变参数列表，用于替换格式字符串中的占位符
 * @return int 成功输出`UTF-8`编码字符占用的字节数，失败时返回负值
 */
template<typename... Args>
int println(const std::u32string& format, Args... args) {
    return print(format + U'\n', args...);
}

/**
 * @brief UTF8ConsoleInput 类用于从标准输入读取 UTF-8 编码的文本数据。
 *
 * 支持从控制台或管道读取输入，并能正确处理 UTF-8 编码的多字节字符。
 * 提供了读取字符、单词、行以及多行的功能，并支持将宽字符转换为 UTF-8 字符串。
 */
class UTF8ConsoleInput {
    static constexpr int chunkSize = 1024; ///< 每次读取的缓冲区大小
    int pos = 0;
    std::string buffer; ///< 存储读取到的字符数据

    /**
     * @brief 判断字符是否为空白字符
     * @param ch 要判断的字符
     * @return true 如果是空白字符
     * @return false 如果不是空白字符
     */
    static bool is_whitespace(const int ch) {
        return ch == ' ' || ch == '\t' || ch == '\n' ||
            ch == '\r' || ch == '\f' || ch == '\v';
    }

    /**
     * @brief 填充内部缓冲区
     *
     * 根据输入来源（控制台或管道）选择不同的读取方式，
     * 并将读取到的数据追加到 buffer 中。每次最多读取 chunkSize 个字节，
     * 或者直到遇到换行符为止，以先达到者为准。如果读取失败或已到达输入末尾则返回 false。
     *
     * @return true 成功填充缓冲区
     * @return false 读取失败或已到达输入末尾
     */
    bool fillBuffer() {
        std::string chunk;
        char ch;
        for (int i = 0; i < chunkSize; i++) {
            if (fread(&ch, sizeof(char), 1, stdin) == 0) break;
            if (ch == EOF) return false;
            chunk.push_back(ch);
            if (ch == '\n') break;
        }
        if (chunk.empty()) return false;
        buffer.append(chunk);
        return true;
    }

public:
    /**
     * @brief 默认构造函数
     */
    UTF8ConsoleInput() = default;

    /**
     * @brief 析构函数，清理资源
     */
    ~UTF8ConsoleInput() { clear(); }

    /**
     * @brief 清空缓冲区内容
     *
     * 将 pos 置零并清空 buffer，使下一次读取重新开始
     */
    void clear() {
        pos = 0;
        buffer.clear();
    }

    /**
     * @brief 获取下一个字符
     *
     * 如果当前位置超出缓冲区范围，则尝试填充缓冲区；
     * 若仍无法获取字符则返回 EOF，否则返回当前字符并移动 pos 指针
     *
     * @return int 下一个字符，如果已到达输入末尾则返回 EOF
     */
    int get() {
        if (pos >= buffer.size()) {
            if (!fillBuffer() || pos >= buffer.size()) return EOF;
        }
        return buffer[pos++];
    }

    template<typename T>
    T readWord();

    template<typename T>
    T readLine();

    template<typename T>
    std::vector<T> readLines(bool empty_break = false, int break_word = EOF);

    /**
     * @brief 重载 >> 运算符，用于读取指定类型的值
     *
     * 支持读取基本数值类型和字符串类型，根据模板参数 T 自动调用相应的解析方法；
     * 不支持的类型会抛出异常
     *
     * @tparam T 要读取的类型（支持std::string, std::wstring, std::u32string, char, wchar_t, char32_t, int, long, float, double, long long, unsigned long long）
     * @param value 用于存储读取结果的变量
     * @return UTF8ConsoleInput& 返回自身引用，支持链式调用
     * @throws UException 如果类型不支持解析
     */
    template<typename T>
    UTF8ConsoleInput& operator>>(T& value);
};

/**
 * @brief 从控制台输入中读取一个由空白字符分隔的单词（UTF-8编码），并以 std::string 形式返回。
 *
 * 该函数会跳过前导空白字符，读取连续的非空白字符组成一个单词，
 * 然后继续跳过后续的空白字符，为下一次读取做准备。
 *
 * @return std::string 读取到的单词字符串
 */
template<>
inline std::string UTF8ConsoleInput::readWord<std::string>() {
    std::string result;
    int ch;
    result.reserve(chunkSize);
    // 跳过前导空白字符
    while ((ch = get()) != EOF && is_whitespace(ch)) {
    }

    if (ch != EOF) pos--;

    while ((ch = get()) != EOF && !is_whitespace(ch)) {
        result.push_back(static_cast<char>(ch));
    }

    // 跳过单词之间的空白字符
    while (pos < buffer.size() && (ch = get()) != EOF && is_whitespace(ch)) {
    }
    if (!is_whitespace(ch)) pos--;

    return result;
}

/**
 * @brief 从控制台输入中读取一个由空白字符分隔的单词，并以 std::wstring 形式返回。
 *
 * 该函数通过调用 readWord<std::string>() 获取 UTF-8 字符串，
 * 然后将其转换为宽字符串（std::wstring）返回。
 *
 * @return std::wstring 读取到的宽字符串形式的单词
 */
template<>
inline std::wstring UTF8ConsoleInput::readWord<std::wstring>() {
    return string_to_wstring(readWord<std::string>());
}

/**
 * @brief 从控制台输入中读取一个由空白字符分隔的单词，并以 std::u32string 形式返回。
 *
 * 该函数通过调用 readWord<std::string>() 获取 UTF-8 字符串，
 * 然后将其转换为 UTF-32 字符串（std::u32string）返回。
 *
 * @return std::u32string 读取到的 UTF-32 字符串形式的单词
 */
template<>
inline std::u32string UTF8ConsoleInput::readWord<std::u32string>() {
    return string_to_u32string(readWord<std::string>());
}

/**
 * @brief 从控制台输入中读取一行文本（直到遇到换行符 \n），并以 std::string 形式返回。
 *
 * 该函数会忽略回车符 \r，只保留实际内容字符和换行符 \n。
 *
 * @return std::string 读取到的一行文本
 */
template<>
inline std::string UTF8ConsoleInput::readLine<std::string>() {
    std::string result;
    int ch;
    result.reserve(chunkSize);
    while ((ch = get()) != EOF && ch != '\n') {
        if (ch != '\r') {
            result += static_cast<char>(ch);
        }
    }
    return result;
}

/**
 * @brief 从控制台输入中读取一行文本，并以 std::wstring 形式返回。
 *
 * 该函数通过调用 readLine<std::string>() 获取 UTF-8 字符串，
 * 然后将其转换为宽字符串（std::wstring）返回。
 *
 * @return std::wstring 读取到的宽字符串形式的一行文本
 */
template<>
inline std::wstring UTF8ConsoleInput::readLine<std::wstring>() {
    return string_to_wstring(readLine<std::string>());
}

/**
 * @brief 从控制台输入中读取一行文本，并以 std::u32string 形式返回。
 *
 * 该函数通过调用 readLine<std::string>() 获取 UTF-8 字符串，
 * 然后将其转换为 UTF-32 字符串（std::u32string）返回。
 *
 * @return std::u32string 读取到的 UTF-32 字符串形式的一行文本
 */
template<>
inline std::u32string UTF8ConsoleInput::readLine<std::u32string>() {
    return string_to_u32string(readLine<std::string>());
}

/**
 * @brief 从控制台输入中读取多行文本，直到遇到指定的终止字符或满足空行终止条件。
 *
 * @param empty_break 如果为 true，则在读取到一个空行时停止读取
 * @param break_word 指定的终止字符，当读取到该字符时停止读取（默认为 EOF）
 * @return std::vector<std::string> 读取到的所有行组成的字符串向量
 */
template<>
inline std::vector<std::string> UTF8ConsoleInput::readLines<std::string>(const bool empty_break, const int break_word) {
    std::vector<std::string> result;
    std::string line;
    while (true) {
        const int ch = get();
        if (ch == '\r') continue;
        if (ch == '\n' || ch == break_word) {
            if (empty_break && line.empty()) break;
            result.push_back(std::move(line));
            line.clear();
            if (ch == break_word) break;
        } else {
            line.push_back(static_cast<char>(ch));
        }
    }
    return result;
}

/**
 * @brief 从控制台输入中读取多行文本，并以 std::wstring 向量形式返回。
 *
 * 该函数通过调用 readLines<std::string>() 获取 UTF-8 字符串向量，
 * 然后将每一行转换为宽字符串（std::wstring）。
 *
 * @param empty_break 如果为 true，则在读取到一个空行时停止读取
 * @param break_word 指定的终止字符，当读取到该字符时停止读取（默认为 EOF）
 * @return std::vector<std::wstring> 读取到的所有行组成的宽字符串向量
 */
template<>
inline std::vector<std::wstring> UTF8ConsoleInput::readLines<
    std::wstring>(const bool empty_break, const int break_word) {
    std::vector<std::wstring> result;
    for (const auto& line : readLines<std::string>(empty_break, break_word)) {
        result.push_back(std::move(string_to_wstring(line)));
    }
    return result;
}

/**
 * @brief 从控制台输入中读取多行文本，并以 std::u32string 向量形式返回。
 *
 * 该函数通过调用 readLines<std::string>() 获取 UTF-8 字符串向量，
 * 然后将每一行转换为 UTF-32 字符串（std::u32string）。
 *
 * @param empty_break 如果为 true，则在读取到一个空行时停止读取
 * @param break_word 指定的终止字符，当读取到该字符时停止读取（默认为 EOF）
 * @return std::vector<std::u32string> 读取到的所有行组成的 UTF-32 字符串向量
 */
template<>
inline std::vector<std::u32string> UTF8ConsoleInput::readLines<std::u32string>(
    const bool empty_break, const int break_word) {
    std::vector<std::u32string> result;
    for (const auto& line : readLines<std::string>(empty_break, break_word)) {
        result.push_back(std::move(string_to_u32string(line)));
    }
    return result;
}


template<typename T>
UTF8ConsoleInput& UTF8ConsoleInput::operator>>(T& value) {
    auto token = readWord<std::string>();
    if (token.empty()) return *this;
    // 处理各种类型
    try {
        if constexpr (std::is_same_v<T, std::string>) {
            value = token;
        } else if constexpr (std::is_same_v<T, std::wstring>) {
            value = string_to_wstring(token);
        } else if constexpr (std::is_same_v<T, std::u32string>) {
            value = string_to_u32string(token);
        } else if constexpr (std::is_same_v<T, char>) {
            value = token[0];
        } else if constexpr (std::is_same_v<T, wchar_t>) {
            value = string_to_wstring(token).c_str()[0];
        } else if constexpr (std::is_same_v<T, char32_t>) {
            value = string_to_u32string(token).c_str()[0];
        } else if constexpr (std::is_same_v<T, int>) {
            value = std::stoi(token);
        } else if constexpr (std::is_same_v<T, long>) {
            value = std::stol(token);
        } else if constexpr (std::is_same_v<T, long long>) {
            value = std::stoll(token);
        } else if constexpr (std::is_same_v<T, unsigned long long>) {
            value = std::stoull(token);
        } else if constexpr (std::is_same_v<T, float>) {
            value = std::stof(token);
        } else if constexpr (std::is_same_v<T, double>) {
            value = std::stod(token);
        } else {
            // 对于不支持的类型，可以考虑抛出异常或使用其他方式处理
            throw UException("Unsupported type for parsing");
        }
    } catch (const std::exception& e) {
        const std::string error_msg = "Parse error at token '" + token + "': " + e.what();
        fputs(error_msg.c_str(), stderr);
    }
    return *this;
}

/**
 * @brief 全局 UTF-8 控制台输入流对象
 *
 * 这是一个内联声明的全局 UTF-8 控制台输入流对象，用于处理 UTF-8 编码的控制台输入。
 * 该对象提供了对控制台输入的封装，支持Unicode字符的正确读取。
 */
inline UTF8ConsoleInput ucin;

/**
 * @brief UTF-8 控制台输出类，用于支持 UTF-8 和宽字符的跨平台控制台输出。
 *
 * 该类封装了对字符串、宽字符串、数字、布尔值等类型的输出操作，
 * 并支持流式操作符（<<）以及自动刷新控制。
 */
class UTF8ConsoleOutput {
    bool should_flush = false; ///< 控制是否在每次输出后自动刷新 stdout 缓冲区

    /**
     * @brief 输出 UTF-8 字符串到控制台
     * @param str 要输出的 UTF-8 字符串
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& _puts(const char* str) {
        fwrite(str, sizeof(char), strlen(str), stdout);
        if (should_flush) fflush(stdout);
        return *this;
    }

public:
    /**
     * @brief 设置是否在每次输出后自动刷新缓冲区，通常在某些调试输出场景下使用
     * @param auto_flush 是否启用自动刷新
     */
    void setAutoFlush(const bool auto_flush) {
        should_flush = auto_flush;
    }

    /**
     * @brief 流式输出操作符 - 字符串
     * @param str 要输出的字符串
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const std::string& str) {
        return _puts(str.c_str());
    }

    /**
     * @brief 刷新 stdout 缓冲区
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& flush() {
        fflush(stdout);
        return *this;
    }

    /**
     * @brief 流式输出操作符 - C 风格字符串
     * @param str 要输出的 C 风格字符串指针
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const char* str) {
        return _puts(str);
    }

    /**
     * @brief 流式输出操作符 - 宽字符串
     * @param wideStr 要输出的宽字符串
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const std::wstring& wideStr) {
        return _puts(wstring_to_string(wideStr).c_str());
    }

    /**
     * @brief 重载输出流操作符，用于输出UTF-32字符串到控制台
     * @param u32str 指向UTF-32编码字符串的指针
     * @return UTF8ConsoleOutput对象的引用，支持链式调用
     *
     * 该函数将UTF-32编码的字符串转换为UTF-8编码，然后输出到控制台
     */
    UTF8ConsoleOutput& operator<<(const char32_t* u32str) {
        return _puts(u32string_to_string(u32str).c_str());
    }

    /**
     * @brief 重载输出流操作符，用于输出std::u32string对象到控制台
     * @param u32str UTF-32编码的std::u32string字符串对象
     * @return UTF8ConsoleOutput对象的引用，支持链式调用
     *
     * 该函数将UTF-32编码的std::u32string对象转换为UTF-8编码，然后输出到控制台
     */
    UTF8ConsoleOutput& operator<<(const std::u32string& u32str) {
        return _puts(u32string_to_string(u32str).c_str());
    }

    /**
     * @brief 流式输出操作符 - C 风格宽字符串
     * @param wideStr 要输出的 C 风格宽字符串指针
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const wchar_t* wideStr) {
        if (wideStr) {
            return _puts(wstring_to_string(wideStr).c_str());
        }
        return *this;
    }

    /**
     * @brief 流式输出操作符 - 字符
     * @param ch 要输出的字符
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const char ch) {
        return _puts(std::string(1, ch).c_str());
    }

    /**
     * @brief 流式输出操作符 - 宽字符 (不支持UTF-8字符)
     * @param ch 要输出的宽字符
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const wchar_t ch) {
        return _puts(wstring_to_string(std::wstring(1, ch)).c_str());
    }

    /**
     * @brief 重载输出流操作符，用于向UTF-8控制台输出单个Unicode字符
     * @param ch 要输出的UTF-32字符
     * @return 返回当前UTF8ConsoleOutput对象的引用，支持链式操作
     */
    UTF8ConsoleOutput& operator<<(const char32_t ch) {
        return _puts(u32string_to_string(std::u32string(1, ch)).c_str());
    }

    /**
     * @brief 重载输出流操作符，将算术类型的值转换为字符串并输出到UTF8控制台
     * @tparam T 输入值的类型，必须是算术类型
     * @param value 要输出的算术值
     * @return 返回当前UTF8ConsoleOutput对象的引用，支持链式调用
     */
    template<typename T>
    UTF8ConsoleOutput& operator<<(T value) {
        static_assert(std::is_arithmetic_v<T>, "Only arithmetic types are supported");
        return _puts(std::to_string(value).c_str());
    }

    /**
     * @brief 流式输出操作符 - 布尔值
     * @param value 要输出的布尔值
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const bool value) {
        return _puts(value ? "true" : "false");
    }

    /**
     * @brief 流式输出操作符 - 指针地址
     * @param ptr 要输出的指针地址
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const void* ptr) {
        print("%p", ptr);
        return *this;
    }

    /**
     * @brief 重载流插入运算符，用于支持函数指针操作
     * @param pfun 指向UTF8ConsoleOutput引用参数的函数指针
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(UTF8ConsoleOutput& (*pfun)(UTF8ConsoleOutput&)) {
        return pfun(*this);
    }

    /**
     * @brief 重载输出流操作符，将字符串向量输出到UTF8控制台
     * @tparam T 字符串类型，必须是std::string、std::wstring或std::u32string之一
     * @param lines 包含字符串的向量容器
     * @return 返回当前对象的引用，支持链式调用
     */
    template<typename T>
    UTF8ConsoleOutput& operator<<(const std::vector<T>& lines) {
        // 判断T类型不支持的报警告
        static_assert(
            std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring> || std::is_same_v<T, std::u32string>,
            "Unsupported type!");
        // 第一行不输出换行符
        bool first = true;
        // 遍历并向量中的每个字符串输出到控制台
        for (const auto& line : lines) {
            first ? first = false : fputc('\n', stdout);
            *this << line;
        }
        return *this;
    }
};


/**
 * @brief 全局 UTF-8 控制台输出流对象
 *
 * 这是一个内联声明的全局 UTF-8 控制台输出流对象，用于处理 UTF-8 编码的控制台输出。
 * 该对象提供了对控制台输出的封装，支持Unicode字符的正确显示，功能与std::cout相当。
 */
inline UTF8ConsoleOutput ucout;
/**
 * @brief 输出换行符并刷新标准输出缓冲区
 *
 * 该函数用于向UTF8ConsoleOutput对象输出一个换行符，并立即刷新标准输出缓冲区，
 * 确保输出内容立即显示在控制台上。
 *
 * @param out UTF8ConsoleOutput对象的引用，用于执行写入操作
 * @return UTF8ConsoleOutput对象的引用，支持链式调用
 */
inline UTF8ConsoleOutput& uendl(UTF8ConsoleOutput& out) {
    fputc('\n', stdout);
    fflush(stdout);
    return out;
}

/**
 * @brief UTF8控制台编码管理类
 *
 * 该类用于在Windows控制台环境中自动设置和恢复UTF-8编码格式，
 * 确保控制台能够正确显示和处理UTF-8编码的文本内容。
 */
inline class UTF8Console {
    UINT old_cp = 0; /**< 保存原始控制台输入编码 */
    UINT old_output_cp = 0; /**< 保存原始控制台输出编码 */

public:
    /**
     * @brief 构造函数，自动设置控制台UTF-8编码
     *
     * 保存当前控制台的输入和输出编码格式，然后将它们设置为UTF-8编码。
     * 这样可以确保控制台能够正确处理UTF-8格式的中文和其他多字节字符。
     */
    UTF8Console() {
        // 保存控制台输出编码
        old_cp = GetConsoleCP();
        old_output_cp = GetConsoleOutputCP();
        // 设置控制台中文输出
        if (old_cp != CP_UTF8) SetConsoleCP(CP_UTF8);
        if (old_output_cp != CP_UTF8) SetConsoleOutputCP(CP_UTF8);
    }

    /**
     * @brief 析构函数，自动恢复控制台原始编码
     *
     * 在对象销毁时，将控制台的输入和输出编码恢复到构造函数调用前的状态，
     * 确保不会影响其他代码对控制台编码的使用。
     */
    ~UTF8Console() {
        if (old_cp && old_cp != CP_UTF8) SetConsoleCP(old_cp);
        if (old_output_cp && old_output_cp != CP_UTF8) SetConsoleOutputCP(old_output_cp);
    }
} utf8_console_instance;

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

class FastString {
    static constexpr size_t default_capacity = 256; ///< 默认初始容量大小
    char* data_ = nullptr; ///< 字符串数据指针
    size_t size_ = 0; ///< 当前字符串长度（不含结尾'\0'）
    size_t capacity_ = 0; ///< 当前分配的内存容量

    FastString& grow_(const size_t min_capacity) {
        size_t new_capacity = capacity_ + (capacity_ >> 1); // 1.5倍增长
        if (new_capacity < min_capacity) new_capacity = min_capacity;
        if (new_capacity < default_capacity) new_capacity = default_capacity;

        // 使用realloc风格的优化：尝试原地扩展
        if (data_) {
            if (const auto new_data = static_cast<char*>(realloc(data_, new_capacity))) {
                data_ = new_data;
                capacity_ = new_capacity;
                return *this;
            }
        }

        // realloc失败，使用传统分配
        const auto new_data = static_cast<char*>(malloc(new_capacity));
        if (!new_data) throw UException("Failed to allocate memory");
        if (data_) {
            if (size_ > 0) memcpy(new_data, data_, size_);
            free(data_);
        }
        data_ = new_data;
        capacity_ = new_capacity;
        return *this;
    }

public:
    FastString() = default;

    explicit FastString(const char* str) {
        const size_t len = strlen(str);
        if (len == 0) return;
        grow_(len + 1);
        // ReSharper disable once CppDFAConstantConditions
        if (data_) {
            memcpy(data_, str, len);
            data_[size_] = '\0';
            size_ = len;
        }
    }

    explicit FastString(const std::string& str) : FastString(str.c_str()) {
    }

    FastString(const FastString& other) {
        append(other);
    }

    FastString(FastString&& other) noexcept : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    explicit FastString(const int value) {
        append(value);
    }

    FastString& operator=(const FastString& other) {
        if (this != &other) {
            // 防止自我赋值
            clear().append(other);
        }
        return *this;
    }

    FastString& operator=(const char* str) {
        clear().append(str);
        return *this;
    }

    FastString& operator+(const FastString& other) {
        return append(other);
    }

    ~FastString() { release(); }

    explicit operator std::string() const {
        return to_string();
    }

    [[nodiscard]] size_t size() const { return size_; }

    [[nodiscard]] const char* c_str() const { return data_ && size_ > 0 ? data_ : ""; }

    [[nodiscard]] char* data() const { return data_; }

    [[nodiscard]] bool empty() const { return size_ == 0; }

    FastString& clear() {
        size_ = 0;
        return *this;
    }

    FastString& release() {
        if (data_) free(data_);
        data_ = nullptr;
        size_ = 0;
        capacity_ = 0;
        return *this;
    }

    FastString& reserve(const size_t new_capacity) {
        if (new_capacity <= capacity_) return *this;
        return grow_(new_capacity);
    }

    FastString& push_back(const char c) {
        if (c == '\0') return *this;
        if (size_ >= capacity_) grow_(size_ + 1);
        if (data_) {
            data_[size_++] = c;
            data_[size_] = '\0';
        }
        return *this;
    }

    FastString& append(const int value) noexcept {
        char buffer[16]{};
        char* end = buffer + 16;
        char* ptr = end;

        // 手动整数转字符串，避免函数调用开销
        const bool negative = value < 0;
        unsigned int abs_value = negative ? -value : value;

        do {
            *--ptr = static_cast<char>('0' + abs_value % 10);
            abs_value /= 10;
        } while (abs_value > 0);

        if (negative) *--ptr = '-';

        const size_t len = end - ptr;
        return append(ptr, len);
    }

    FastString& append(const char* str) {
        const size_t len = strlen(str); // 第一次遍历字符串
        if (len == 0) return *this;
        if (size_ + len >= capacity_) grow_(size_ + len + 1);
        if (data_) {
            memcpy(data_ + size_, str, len); // 第二次遍历（拷贝）
            size_ += len;
            data_[size_] = '\0';
        }
        return *this;
    }

    FastString& append(const char* str, const size_t len) noexcept {
        if (!str || len <= 0) return *this;
        if (size_ + len >= capacity_) grow_(size_ + len + 1);
        if (data_) {
            memcpy(data_ + size_, str, len);
            size_ += len;
            data_[size_] = '\0';
        }
        return *this;
    }

    FastString& append(const FastString& other) {
        return append(other.c_str(), other.size_);
    }

    FastString& append(const std::string& str) {
        return append(str.c_str());
    }

    [[nodiscard]] std::string to_string() const {
        return {data_, size_};
    }

    FastString& operator+=(const char c) {
        return push_back(c);
    }

    FastString& operator+=(const char* str) {
        return append(str);
    }

    FastString& operator+=(const FastString& other) {
        return append(other);
    }

    char operator[](const size_t index) const {
        if (index >= size_) {
            throw UException("FastString index out of range");
        }
        return data_ ? data_[index] : '\0';
    }
};

class UConverter {
    // UTF-8解码
    // ReSharper disable once CppDFAUnreachableFunctionCall
    static UINT32 decode_(const char*& src, const char* end) {
        if (src >= end) return 0xFFFD;

        const auto first_byte = static_cast<unsigned char>(*src++);

        // 单字节字符 (0xxx xxxx)
        if (first_byte < 0x80) {
            return first_byte;
        }

        // 两字节字符 (110x xxxx)
        if ((first_byte & 0xE0) == 0xC0) {
            if (src >= end) return 0xFFFD; // 替换字符
            const auto second_byte = static_cast<unsigned char>(*src++);
            if ((second_byte & 0xC0) != 0x80) return 0xFFFD;
            return (first_byte & 0x1F) << 6 | second_byte & 0x3F;
        }

        // 三字节字符 (1110 xxxx)
        if ((first_byte & 0xF0) == 0xE0) {
            if (src + 1 >= end) return 0xFFFD;
            const auto second_byte = static_cast<unsigned char>(*src++);
            const auto third_byte = static_cast<unsigned char>(*src++);
            if ((second_byte & 0xC0) != 0x80 || (third_byte & 0xC0) != 0x80) return 0xFFFD;
            return (first_byte & 0x0F) << 12 | (second_byte & 0x3F) << 6 | third_byte & 0x3F;
        }

        // 四字节字符 (1111 0xxx)
        if ((first_byte & 0xF8) == 0xF0) {
            if (src + 2 >= end) return 0xFFFD;
            const auto second_byte = static_cast<unsigned char>(*src++);
            const auto third_byte = static_cast<unsigned char>(*src++);
            const auto fourth_byte = static_cast<unsigned char>(*src++);
            if ((second_byte & 0xC0) != 0x80 || (third_byte & 0xC0) != 0x80 ||
                (fourth_byte & 0xC0) != 0x80)
                return 0xFFFD;
            return (first_byte & 0x07) << 18 | (second_byte & 0x3F) << 12 |
                (third_byte & 0x3F) << 6 | fourth_byte & 0x3F;
        }

        return 0xFFFD; // 无效的UTF-8序列
    }

    // UTF-8编码
    static size_t encode_(UINT32 code_point, char* dest) {
        if (code_point <= 0x7F) {
            // 单字节: 0xxxxxxx
            dest[0] = static_cast<char>(code_point);
            return 1;
        }
        if (code_point <= 0x7FF) {
            // 两字节: 110xxxxx 10xxxxxx
            dest[0] = static_cast<char>(0xC0 | code_point >> 6);
            dest[1] = static_cast<char>(0x80 | code_point & 0x3F);
            return 2;
        }
        if (code_point <= 0xFFFF) {
            // 三字节: 1110xxxx 10xxxxxx 10xxxxxx
            if (code_point >= 0xD800 && code_point <= 0xDFFF) code_point = 0xFFFD;
            dest[0] = static_cast<char>(0xE0 | code_point >> 12);
            dest[1] = static_cast<char>(0x80 | code_point >> 6 & 0x3F);
            dest[2] = static_cast<char>(0x80 | code_point & 0x3F);
            return 3;
        }
        if (code_point <= 0x10FFFF) {
            // 四字节: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
            dest[0] = static_cast<char>(0xF0 | code_point >> 18);
            dest[1] = static_cast<char>(0x80 | code_point >> 12 & 0x3F);
            dest[2] = static_cast<char>(0x80 | code_point >> 6 & 0x3F);
            dest[3] = static_cast<char>(0x80 | code_point & 0x3F);
            return 4;
        }
        // 无效码点，使用替换字符
        return encode_(0xFFFD, dest);
    }

    static void convert_(const std::string& str, std::wstring& result) {
        if (str.empty()) return;
        const size_t len = str.size();
        result.resize(len);

        const char* src = str.data();
        const char* end = src + len;
        wchar_t* dest = result.data();
        size_t dest_index = 0;

        while (src < end) {
            if (static_cast<unsigned char>(*src) < 0x80) {
                // ASCII字符：快速路径
                dest[dest_index++] = static_cast<wchar_t>(*src++);
                continue;
            }

            if (UINT32 code_point = decode_(src, end); code_point <= 0xFFFF) {
                dest[dest_index++] = static_cast<wchar_t>(code_point);
            } else {
                code_point -= 0x10000;
                dest[dest_index++] = static_cast<wchar_t>(0xD800 + (code_point >> 10));
                dest[dest_index++] = static_cast<wchar_t>(0xDC00 + (code_point & 0x3FF));
            }
        }

        result.resize(dest_index);
    }

    static void convert_(const std::string& str, std::u32string& result) {
        if (str.empty()) return;

        const size_t len = str.size();
        const char* src = str.data();
        const char* end = src + len;

        result.resize(len);

        char32_t* dest = result.data();
        size_t dest_index = 0;

        // 单次遍历转换
        while (src < end) {
            // ASCII快速路径检查
            if (static_cast<UINT32>(static_cast<unsigned char>(*src)) < 0x80) {
                dest[dest_index++] = static_cast<char32_t>(*src++);
                continue;
            }

            // 多字节UTF-8字符
            const UINT32 code_point = decode_(src, end);
            dest[dest_index++] = static_cast<char32_t>(code_point);
        }

        // 调整到实际大小
        result.resize(dest_index);
    }

    static void convert_(const std::wstring& wstr, std::u32string& result) {
        if (wstr.empty()) return;

        const size_t len = wstr.size();
        const wchar_t* src = wstr.data();

        // 预分配空间（wstring长度 >= u32string长度）
        result.resize(len);
        char32_t* dest = result.data();
        size_t dest_len = 0;

        // wchar_t是16位（UTF-16），需要处理代理对
        for (size_t i = 0; i < len; ++i) {
            // 检查是否是高位代理（High Surrogate）
            if (const auto code_unit = static_cast<UINT32>(src[i]); code_unit >= 0xD800 && code_unit <= 0xDBFF) {
                // 有下一个字符且是低位代理（Low Surrogate）
                if (i + 1 < len) {
                    if (const auto next_unit = static_cast<UINT32>(src[i + 1]);
                        next_unit >= 0xDC00 && next_unit <= 0xDFFF) {
                        // 组合代理对
                        const UINT32 code_point = 0x10000 +
                            ((code_unit - 0xD800) << 10) +
                            (next_unit - 0xDC00);
                        dest[dest_len++] = static_cast<char32_t>(code_point);
                        i++; // 跳过已处理的低位代理
                        continue;
                    }
                }
                // 无效的高位代理（缺少配对的低位代理）
                dest[dest_len++] = 0xFFFD;
            }
            // 检查是否是低位代理（不应该单独出现）
            else if (code_unit >= 0xDC00 && code_unit <= 0xDFFF) {
                dest[dest_len++] = 0xFFFD;
            }
            // 基本多文种平面（BMP）字符
            else {
                dest[dest_len++] = static_cast<char32_t>(code_unit);
            }
        }
        result.resize(dest_len);
    }

    // ReSharper disable once CppDFAUnreachableFunctionCall
    static UINT32 get_code_point_(const wchar_t*& src, const wchar_t* end) {
        if (src >= end) return 0xFFFD;

        // UTF-16系统：处理代理对
        const auto first_unit = static_cast<UINT32>(*src++);

        // BMP字符快速路径
        if (first_unit < 0xD800 || first_unit > 0xDFFF) return first_unit;

        // 高位代理
        if (first_unit <= 0xDBFF && src < end) {
            if (const auto second_unit = static_cast<UINT32>(*src); second_unit >= 0xDC00 && second_unit <= 0xDFFF) {
                src++; // 消耗低位代理
                return 0x10000 + ((first_unit - 0xD800) << 10) + (second_unit - 0xDC00);
            }
        }

        // 孤立的低位代理
        return 0xFFFD;
    }

    static void convert_(const std::wstring& wstr, std::string& result) {
        if (wstr.empty()) return;

        const size_t len = wstr.size();

        const wchar_t* src = wstr.data();
        const wchar_t* end = src + len;

        // 预分配最大可能空间
        result.resize(len * 3);

        char* dest = result.data();
        size_t dest_len = 0;

        while (src < end) {
            const UINT32 code_point = get_code_point_(src, end);
            dest_len += encode_(code_point, dest + dest_len);
        }

        result.resize(dest_len);
    }

    static void convert_(const std::u32string& u32str, std::string& result) {
        if (u32str.empty()) return;

        const size_t len = u32str.size();

        const char32_t* src = u32str.data();
        const char32_t* end = src + len;

        // 预分配最大可能空间
        result.resize(len * 4);

        char* dest = result.data();
        size_t dest_len = 0;

        // 单次遍历编码
        while (src < end) {
            const auto code_point = static_cast<UINT32>(*src++);
            dest_len += encode_(code_point, dest + dest_len);
        }

        result.resize(dest_len);
    }

    static void convert_(const std::u32string& u32str, std::wstring& result) {
        if (u32str.empty()) return;
        result.reserve(u32str.size() * sizeof(char32_t) / sizeof(wchar_t));
        for (char32_t ch : u32str) {
            if (ch <= 0xFFFF) {
                // BMP 字符直接转换
                // 检查是否是代理对范围的非法字符
                if (ch >= 0xD800 && ch <= 0xDFFF) {
                    // 这是非法的 UTF-32 字符，因为它映射到 UTF-16 的代理对范围
                    // 可以选择跳过或者用替换字符代替
                    result.push_back(0xFFFD); // 使用替换字符
                } else {
                    result.push_back(static_cast<wchar_t>(ch));
                }
            } else {
                // 处理超出 BMP 的字符，生成高代理和低代理对
                // 检查是否超出 Unicode 最大码点
                if (ch > 0x10FFFF) {
                    // 超出 Unicode 范围，使用替换字符
                    result.push_back(0xFFFD);
                } else {
                    ch -= 0x10000;
                    result.push_back(static_cast<wchar_t>(0xD800 + (ch >> 10))); // 高代理
                    result.push_back(static_cast<wchar_t>(0xDC00 + (ch & 0x3FF))); // 低代理
                }
            }
        }
    }

public:
    template<typename To, typename From>
    static To convert(const From& str) {
        if constexpr (std::is_same_v<To, From>) return str;
        To result{};
        convert_(str, result);
        return result;
    }
};

/**
 * @brief UTF8ConsoleInput 类用于从标准输入读取 UTF-8 编码的文本数据。
 *
 * 支持从控制台或管道读取输入，并能正确处理 UTF-8 编码的多字节字符。
 * 提供了读取字符、单词、行以及多行的功能，并支持将宽字符转换为 UTF-8 字符串。
 */
class UTF8ConsoleInput {
    static constexpr int chunkSize = 1024; ///< 每次读取的缓冲区大小
    int pos = 0;
    FastString buffer; ///< 存储读取到的字符数据

    /**
     * @brief 判断字符是否为空白字符
     * @param ch 要判断的字符
     * @return true 如果是空白字符
     * @return false 如果不是空白字符
     */
    static bool is_whitespace_(const char ch) {
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
    bool fill_buffer_() {
        FastString chunk;
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
     * @brief 清空存储内容
     *
     * 将 pos 置零并清空 buffer，使下一次读取重新开始
     */
    UTF8ConsoleInput& clear() {
        pos = 0;
        buffer.release();
        return *this;
    }

    /**
     * @brief 清空输入缓冲区内容
     */
    UTF8ConsoleInput& flush() {
        fflush(stdin);
        return *this;
    }

    /**
     * @brief 获取下一个字符
     *
     * 如果当前位置超出缓冲区范围，则尝试填充缓冲区；
     * 若仍无法获取字符则返回 EOF，否则返回当前字符并移动 pos 指针
     *
     * @return char 下一个字符，如果已到达输入末尾则返回 EOF
     */
    char get() {
        if (pos >= buffer.size() && !fill_buffer_() || pos >= buffer.size()) return EOF;
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
 * 该函数默认会移除最后一个单词输入后的换行符，与 std::cin 实现略有差异
 *
 * @return std::string 读取到的单词字符串
 */
template<>
inline std::string UTF8ConsoleInput::readWord<std::string>() {
    FastString result;
    char ch;
    // 跳过前导空白字符
    while ((ch = get()) != EOF && is_whitespace_(ch)) {
    }

    if (ch != EOF) pos--;

    while ((ch = get()) != EOF && !is_whitespace_(ch)) {
        result += ch;
    }

    // 回退最后一个空白字符(忽略换行符，如果需和 std::cin 保持一致，请取消ch != '\n' 条件判断)
    if (ch != EOF && ch != '\n') pos--;

    return result.to_string();
}

/**
 * @brief 从控制台输入中读取一个由空白字符分隔的单词，并以 std::wstring 形式返回。
 *
 * 该函数通过调用 readWord<std::string>() 获取 UTF-8 字符串，
 * 然后将其转换为宽字符串（std::wstring）返回。
 *
 * @return std::wstring 读取到的宽字符串形式的单词
 */
template<typename T>
T UTF8ConsoleInput::readWord() {
    return UConverter::convert<T>(readWord<std::string>());
}

/**
 * @brief 从控制台读取一行 UTF-8 编码的文本，返回 std::string 类型。
 *
 * 该函数是 readLine 模板函数的特化版本，专门用于读取字符串。
 * 它会逐字符读取输入，直到遇到换行符（\n）或文件结束符（EOF）为止。
 * 回车符（\r）会被忽略。
 *
 * @return 返回读取到的一行文本，不包含换行符。
 */
template<>
inline std::string UTF8ConsoleInput::readLine<std::string>() {
    FastString result;
    char ch;
    while ((ch = get()) != EOF && ch != '\n') {
        if (ch != '\r') {
            result += ch;
        }
    }
    return result.to_string();
}

/**
 * @brief 从控制台读取一行文本，并将其转换为目标类型 T。
 *
 * 该函数是 readLine 的通用模板版本，它首先调用 readLine<std::string>() 获取字符串，
 * 然后通过 UConverter 工具类将字符串转换为目标类型 T。
 *
 * @tparam T 目标类型，必须支持从字符串转换。
 * @return 返回转换后的目标类型值。
 */
template<typename T>
T UTF8ConsoleInput::readLine() {
    return UConverter::convert<T>(readLine<std::string>());
}

/**
 * @brief 从控制台读取多行 UTF-8 编码的文本，返回 std::string 类型的字符串向量。
 *
 * 该函数是 readLines 模板函数的特化版本，专门用于读取多个字符串行。
 * 支持通过参数控制是否在遇到空行时停止读取，以及是否在遇到特定字符时停止读取。
 *
 * @param empty_break 如果为 true，则在读取到空行时停止读取。
 * @param break_word 指定一个字符，当读取到该字符时停止读取。
 * @return 返回读取到的所有行组成的字符串向量。
 */
template<>
inline std::vector<std::string> UTF8ConsoleInput::readLines<std::string>(const bool empty_break, const int break_word) {
    std::vector<std::string> result;
    result.reserve(20);
    FastString line;
    while (true) {
        const char ch = get();
        if (ch == '\r') continue;
        if (ch == '\n' || ch == break_word) {
            if (empty_break && line.empty()) break;
            result.push_back(line.to_string());
            line.clear();
            if (ch == break_word) break;
        } else {
            line.push_back(ch);
        }
    }
    return result;
}

/**
 * @brief 从控制台读取多行文本，并将其转换为目标类型 T 的向量。
 *
 * 该函数是 readLines 的通用模板版本，它首先调用 readLines<std::string>() 获取字符串向量，
 * 然后通过 UConverter 工具类将每个字符串转换为目标类型 T，并组成向量返回。
 *
 * @tparam T 目标类型，必须支持从字符串转换。
 * @param empty_break 如果为 true，则在读取到空行时停止读取。
 * @param break_word 指定一个字符，当读取到该字符时停止读取。
 * @return 返回转换后的目标类型向量。
 */
template<typename T>
std::vector<T> UTF8ConsoleInput::readLines(const bool empty_break, const int break_word) {
    const std::vector<std::string> lines = readLines<std::string>(empty_break, break_word);
    if (lines.empty()) return {};
    std::vector<T> result;
    result.reserve(lines.size());
    for (const auto& line : lines) {
        result.push_back(UConverter::convert<T>(line));
    }
    return result;
}

/**
 * @brief 重载 >> 操作符，用于从控制台读取一个单词并解析为目标类型 T。
 *
 * 该函数首先调用 readWord<std::string>() 获取一个单词字符串，
 * 然后根据目标类型 T 的不同，使用相应的转换方法将字符串转换为目标类型。
 * 支持的类型包括基本数据类型（如 int、float、double）和字符串类型。
 * 如果转换失败，会输出错误信息到标准错误流。
 *
 * @tparam T 目标类型，必须是支持的类型之一。
 * @param value 用于存储解析结果的变量引用。
 * @return 返回当前 UTF8ConsoleInput 对象的引用，支持链式调用。
 */
template<typename T>
UTF8ConsoleInput& UTF8ConsoleInput::operator>>(T& value) {
    auto token = readWord<std::string>();
    if (token.empty()) return *this;
    // 处理各种类型
    try {
        if constexpr (std::is_same_v<T, std::string>) {
            value = token;
        } else if constexpr (std::is_same_v<T, std::wstring> || std::is_same_v<T, std::u32string>) {
            value = UConverter::convert<T>(token);
        } else if constexpr (std::is_same_v<T, char>) {
            value = token[0];
        } else if constexpr (std::is_same_v<T, wchar_t> || std::is_same_v<T, char32_t>) {
            value = UConverter::convert<T>(token).c_str()[0];
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
    FILE* stream;

    UTF8ConsoleOutput& writes_(const char* str, const size_t len) {
        fwrite(str, sizeof(char), len, stream);
        return *this;
    }

    /**
     * @brief 输出 UTF-8 字符串到控制台
     * @param str 要输出的 UTF-8 字符串
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& writes_(const char* str) {
        return writes_(str, strlen(str));
    }

    UTF8ConsoleOutput& writes_(const std::string& str) {
        return writes_(str.c_str(), str.size());
    }

public:
    UTF8ConsoleOutput() : stream(stdout) {
    }

    explicit UTF8ConsoleOutput(FILE* stream) : stream(stream) {
    }

    /**
     * @brief 获取输出流指针
     * @return FILE* 返回指向输出流指针
     * @note 该函数使用[[nodiscard]]属性标记，表示返回值不应被忽略
     */
    [[nodiscard]] FILE* getStream() const { return stream; }

    /**
     * @brief 刷新 stdout 缓冲区
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& flush() {
        fflush(stream);
        return *this;
    }

    /**
     * @brief 设置是否在每次输出后自动刷新缓冲区，通常在某些调试输出场景下使用
     * @param auto_flush 是否启用自动刷新
     */
    UTF8ConsoleOutput& setAutoFlush(const bool auto_flush) {
        should_flush = auto_flush;
        return *this;
    }

    /**
     * @brief 流式输出操作符 - C 风格字符串
     * @param str 要输出的 C 风格字符串指针
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const char* str) {
        return writes_(str);
    }

    /**
     * @brief 流式输出操作符 - 字符串
     * @param str 要输出的字符串
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const std::string& str) {
        return writes_(str);
    }

    /**
     * @brief 流式输出操作符 - C 风格宽字符串
     * @param wideStr 要输出的 C 风格宽字符串指针
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const wchar_t* wideStr) {
        if (wideStr) writes_(UConverter::convert<std::string>(wideStr));
        return *this;
    }

    /**
     * @brief 流式输出操作符 - 宽字符串
     * @param wideStr 要输出的宽字符串
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const std::wstring& wideStr) {
        return writes_(UConverter::convert<std::string>(wideStr));
    }

    /**
     * @brief 重载输出流操作符，用于输出UTF-32字符串到控制台
     * @param u32str 指向UTF-32编码字符串的指针
     * @return UTF8ConsoleOutput对象的引用，支持链式调用
     *
     * 该函数将UTF-32编码的字符串转换为UTF-8编码，然后输出到控制台
     */
    UTF8ConsoleOutput& operator<<(const char32_t* u32str) {
        if (u32str) writes_(UConverter::convert<std::string>(u32str));
        return *this;
    }

    /**
     * @brief 重载输出流操作符，用于输出std::u32string对象到控制台
     * @param u32str UTF-32编码的std::u32string字符串对象
     * @return UTF8ConsoleOutput对象的引用，支持链式调用
     *
     * 该函数将UTF-32编码的std::u32string对象转换为UTF-8编码，然后输出到控制台
     */
    UTF8ConsoleOutput& operator<<(const std::u32string& u32str) {
        return writes_(UConverter::convert<std::string>(u32str));
    }

    /**
     * @brief 流式输出操作符 - 字符
     * @param ch 要输出的字符
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const char ch) {
        return writes_(&ch, 1);
    }

    /**
     * @brief 流式输出操作符 - 宽字符 (不支持UTF-8字符)
     * @param ch 要输出的宽字符
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const wchar_t ch) {
        return writes_(UConverter::convert<std::string>(std::wstring(1, ch)));
    }

    /**
     * @brief 重载输出流操作符，用于向UTF-8控制台输出单个Unicode字符
     * @param ch 要输出的UTF-32字符
     * @return 返回当前UTF8ConsoleOutput对象的引用，支持链式操作
     */
    UTF8ConsoleOutput& operator<<(const char32_t ch) {
        return writes_(UConverter::convert<std::string>(std::u32string(1, ch)));
    }

    /**
     * @brief 流式输出操作符 - 布尔值
     * @param value 要输出的布尔值
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const bool value) {
        return writes_(value ? "true" : "false", value ? 4 : 5);
    }

    /**
     * @brief 流式输出操作符 - 指针地址
     * @param ptr 要输出的指针地址
     * @return 返回当前对象的引用，支持链式调用
     */
    UTF8ConsoleOutput& operator<<(const void* ptr) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%p", ptr);
        return writes_(buf, strlen(buf));
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
     * 重载输出流操作符，用于将算术类型的数据输出到UTF8控制台
     *
     * @tparam T 数据类型，必须是算术类型（整数、浮点数等）
     * @param value 要输出的算术值
     * @return UTF8ConsoleOutput& 返回当前对象的引用，支持链式调用
     *
     * 该函数使用SFINAE技术，只对算术类型启用重载，
     * 通过std::to_string将数值转换为字符串后输出
     */
    template<typename T>
    std::enable_if_t<std::is_arithmetic_v<T>, UTF8ConsoleOutput&>
        operator<<(const T value) {
        // 将算术值转换为字符串并写入输出流
        return writes_(std::to_string(value));
    }

    /**
     * @brief 重载输出流操作符，将字符串向量输出到UTF8控制台
     * @tparam T 字符串类型，必须是std::string、std::wstring或std::u32string之一
     * @param lines 包含字符串的向量容器
     * @return 返回当前对象的引用，支持链式调用
     */
    template<typename T>
    std::enable_if_t<std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring> || std::is_same_v<T,
        std::u32string>, UTF8ConsoleOutput&>
        operator<<(const std::vector<T>& lines) {
        // 第一行不输出换行符
        bool first = true;
        // 遍历并向量中的每个字符串输出到控制台
        for (const auto& line : lines) {
            first ? first = false : fputc('\n', stream);
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
    fputc('\n', out.getStream());
    fflush(out.getStream());
    return out;
}

/**
 * @brief 刷新UTF-8控制台输出流
 *
 * 此函数用于刷新指定的UTF8ConsoleOutput输出流，确保所有缓冲区中的数据都被写出。
 * 它是对标准库fflush函数的封装，专门用于UTF-8编码的控制台输出。
 *
 * @param out 要刷新的UTF8ConsoleOutput对象引用
 * @return 返回刷新后的UTF8ConsoleOutput对象引用，支持链式调用
 */
inline UTF8ConsoleOutput& uflush(UTF8ConsoleOutput& out) {
    fflush(out.getStream());
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

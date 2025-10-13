/**
 * ==================================================================================
 * @file        csv_writer.cpp
 * @brief       CSV文件写入器 - 实现文件
 * @version     1.0.0
 * @date        2025-10-13
 *
 * @description
 * 本文件实现了CsvWriter类的所有方法，包括：
 * - 文件打开和关闭（构造/析构）
 * - CSV表头写入
 * - 数据行写入（支持多种类型）
 * - CSV特殊字符转义
 * - 空值处理
 *
 * CSV格式说明：
 * - 使用逗号作为字段分隔符
 * - 包含特殊字符的字段用双引号包围
 * - 字段内的双引号转义为两个双引号
 * - 行以换行符结束
 *
 * @author      LS-Game-DataGen Team
 * ==================================================================================
 */

#include "csv_writer.h"
#include <sstream>
#include <stdexcept>

// ====================================================================================
// 构造函数和析构函数
// ====================================================================================

/**
 * @brief 构造函数实现 - 打开文件准备写入
 */
CsvWriter::CsvWriter(const std::string& path)
    : ofs_(path, std::ios::out | std::ios::trunc) {
    // std::ios::out: 以写入模式打开
    // std::ios::trunc: 如果文件已存在，清空内容

    if (!ofs_) {
        // 文件打开失败，抛出异常
        throw std::runtime_error("无法打开输出文件: " + path);
    }
}

/**
 * @brief 析构函数实现 - 确保数据写入磁盘
 */
CsvWriter::~CsvWriter() {
    // flush确保缓冲区中的所有数据都写入磁盘
    // 文件流会在对象销毁时自动关闭
    ofs_.flush();
}

// ====================================================================================
// 表头写入
// ====================================================================================

/**
 * @brief 写入CSV表头（仅在第一次调用时执行）
 */
void CsvWriter::writeHeaderIfNeeded() {
    if (!wrote_header_) {
        // 写入固定的表头行
        ofs_ << "section,key,u,v,i,t,value\n";
        wrote_header_ = true;  // 标记表头已写入
    }
}

// ====================================================================================
// CSV转义和格式化辅助函数
// ====================================================================================

/**
 * @brief CSV字符串转义实现
 *
 * @details
 * 根据CSV标准规则对字符串进行转义：
 * 1. 检查是否包含特殊字符（逗号、双引号、换行、回车）
 * 2. 如果包含，则用双引号包围整个字符串
 * 3. 字符串内的双引号转义为两个双引号
 */
std::string CsvWriter::escape(const std::string& s) {
    // 第一步：检查是否需要转义
    bool need_quote = false;
    for (char c : s) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') {
            need_quote = true;
            break;
        }
    }

    // 如果不包含特殊字符，直接返回原字符串
    if (!need_quote) return s;

    // 第二步：构建转义后的字符串
    std::string out;
    out.reserve(s.size() + 2);  // 预分配空间（至少需要原长度+2个双引号）

    out.push_back('"');  // 开始双引号

    for (char c : s) {
        if (c == '"') {
            // 双引号需要转义为两个双引号
            out += "\"\"";
        } else {
            // 其他字符直接添加
            out.push_back(c);
        }
    }

    out.push_back('"');  // 结束双引号

    return out;
}

/**
 * @brief 整数转字符串，-1转为空字符串
 *
 * @details
 * 在我们的CSV schema中，-1表示"不适用"的索引。
 * 在输出时，将-1转换为空字符串，使CSV更易读。
 *
 * 示例：
 * - meta段的行：section=meta时，u,v,i,t都不适用，显示为空
 * - cost段的行：section=cost时，u,v,t不适用，只有i有值
 */
std::string CsvWriter::toStringOrEmpty(int val) {
    return (val < 0) ? "" : std::to_string(val);
}

// ====================================================================================
// 数据行写入方法
// ====================================================================================

/**
 * @brief 写入一行数据（字符串值版本） - 核心实现
 *
 * @details
 * 这是所有writeRow重载的核心实现。
 * 执行步骤：
 * 1. 如果是第一次调用，先写入表头
 * 2. 对section和key进行转义
 * 3. 将索引值转换（-1转为空）
 * 4. 对value进行转义
 * 5. 按CSV格式写入一行
 */
void CsvWriter::writeRow(const std::string& section,
                         const std::string& key,
                         int u, int v, int i, int t,
                         const std::string& value) {
    // 确保表头已写入
    writeHeaderIfNeeded();

    // 按照 "section,key,u,v,i,t,value\n" 格式写入
    // 注意：使用 escape() 转义可能包含特殊字符的字段
    //       使用 toStringOrEmpty() 将-1转换为空字符串
    ofs_  << escape(section) << ','     // section字段（可能包含特殊字符）
          << escape(key)     << ','     // key字段（可能包含特殊字符）
          << toStringOrEmpty(u) << ','  // u索引（-1显示为空）
          << toStringOrEmpty(v) << ','  // v索引（-1显示为空）
          << toStringOrEmpty(i) << ','  // i索引（-1显示为空）
          << toStringOrEmpty(t) << ','  // t索引（-1显示为空）
          << escape(value) << '\n';     // value字段（可能包含特殊字符）
}

/**
 * @brief 写入一行数据（整数值版本） - 便捷重载
 *
 * @details
 * 将整数转换为字符串，然后调用字符串版本的writeRow。
 * 这个重载使得代码更简洁，不需要手动转换类型。
 */
void CsvWriter::writeRow(const std::string& section, const std::string& key,
                         int u, int v, int i, int t,
                         int value) {
    // 将整数转换为字符串后调用字符串版本
    writeRow(section, key, u, v, i, t, std::to_string(value));
}

/**
 * @brief 写入一行数据（浮点数值版本） - 便捷重载
 *
 * @details
 * 将浮点数强制转换为整数后写入。
 *
 * 注意：这个实现会丢失小数部分！
 * 原因：使用 static_cast<int>(value) 进行转换
 *
 * 如果需要保留小数部分，应该：
 * 1. 使用 std::to_string() 或 std::ostringstream 转换
 * 2. 然后调用字符串版本的 writeRow
 *
 * 当前实现适用于：
 * - 已知数据实际为整数，只是用double类型存储
 * - 不需要小数精度的场景
 */
void CsvWriter::writeRow(const std::string& section, const std::string& key,
                         int u, int v, int i, int t,
                         double value) {
    // 浮点数转整数（截断小数部分）
    int int_value = static_cast<int>(value);

    // 调用整数版本
    writeRow(section, key, u, v, i, t, std::to_string(int_value));
}

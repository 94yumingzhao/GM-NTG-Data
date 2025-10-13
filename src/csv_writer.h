/**
 * ==================================================================================
 * @file        csv_writer.h
 * @brief       CSV文件写入器 - 头文件
 * @version     1.0.0
 * @date        2025-10-13
 *
 * @description
 * CsvWriter 是一个专门用于写入固定schema的CSV文件的工具类。
 *
 * 输出格式（固定schema）：
 *   section,key,u,v,i,t,value
 *
 * 主要特性：
 * - 自动写入CSV表头
 * - 自动处理CSV转义（逗号、双引号、换行符）
 * - 支持多种数据类型（字符串、整数、浮点数）
 * - 使用-1表示空值（不适用的索引）
 * - 线程不安全，单线程使用
 * - RAII风格，析构时自动flush
 *
 * 使用示例：
 * @code
 * CsvWriter writer("output.csv");
 * writer.writeRow("meta", "U", -1, -1, -1, -1, 5);           // 整数
 * writer.writeRow("cost", "cX", -1, -1, 0, -1, 10.5);        // 浮点数
 * writer.writeRow("demand", "Demand", 0, -1, 0, 1, 100.0);   // 需求数据
 * // 文件在writer析构时自动关闭并flush
 * @endcode
 *
 * @note 本类禁用拷贝构造和拷贝赋值，确保文件句柄的唯一性
 *
 * @author      LS-Game-DataGen Team
 * ==================================================================================
 */

#pragma once
#include <fstream>
#include <string>

/**
 * @class CsvWriter
 * @brief CSV文件写入器类
 *
 * @details
 * 提供安全、简单的CSV文件写入功能，专门用于写入固定schema的数据行。
 * 每行包含7个字段：section, key, u, v, i, t, value
 *
 * 特点：
 * - 自动管理文件生命周期（RAII）
 * - 自动写入表头（仅在第一次writeRow时）
 * - 自动转义特殊字符（符合CSV标准）
 * - 使用-1表示空值
 * - 不可复制（防止文件句柄冲突）
 */
class CsvWriter {
public:
    /**
     * @brief 构造函数 - 打开CSV文件准备写入
     *
     * @param path 输出文件路径（相对或绝对路径）
     *
     * @throw std::runtime_error 当文件无法打开时抛出异常
     *
     * @details
     * 以截断模式打开文件（如果文件已存在，会被清空）。
     * 构造函数不会立即写入表头，表头会在第一次调用writeRow时自动写入。
     */
    explicit CsvWriter(const std::string& path);

    /**
     * @brief 析构函数 - 确保数据刷新到磁盘
     *
     * @details
     * 析构时会自动flush缓冲区，确保所有数据都写入磁盘。
     * 不会抛出异常（符合析构函数最佳实践）。
     */
    ~CsvWriter();

    // 禁用拷贝构造和拷贝赋值
    // 原因：避免多个对象持有同一个文件句柄，导致重复写入或关闭
    CsvWriter(const CsvWriter&) = delete;
    CsvWriter& operator=(const CsvWriter&) = delete;

    /**
     * @brief 写入一行数据（字符串值）
     *
     * @param section 数据段名称（如"meta", "cost", "demand"等）
     * @param key     数据键名（如"U", "I", "cX", "Demand"等）
     * @param u       节点索引u（不适用时传-1）
     * @param v       节点索引v（不适用时传-1）
     * @param i       物品索引i（不适用时传-1）
     * @param t       时间索引t（不适用时传-1）
     * @param value   数据值（字符串形式）
     *
     * @details
     * 核心写入方法，其他重载最终都会调用此方法。
     * 特殊字符处理：
     * - 如果字段包含逗号、双引号、换行符，会自动加双引号包围
     * - 字段内的双引号会被转义为两个双引号（CSV标准）
     * - -1会被转换为空字符串
     *
     * @note 第一次调用时会自动写入表头
     */
    void writeRow(const std::string& section,
                  const std::string& key,
                  int u, int v, int i, int t,
                  const std::string& value);

    /**
     * @brief 写入一行数据（整数值）
     *
     * @param section 数据段名称
     * @param key     数据键名
     * @param u       节点索引u（不适用时传-1）
     * @param v       节点索引v（不适用时传-1）
     * @param i       物品索引i（不适用时传-1）
     * @param t       时间索引t（不适用时传-1）
     * @param value   数据值（整数）
     *
     * @details
     * 便捷重载：将整数值转换为字符串后调用字符串版本的writeRow。
     */
    void writeRow(const std::string& section, const std::string& key,
                  int u, int v, int i, int t,
                  int value);

    /**
     * @brief 写入一行数据（浮点数值，转为整数）
     *
     * @param section 数据段名称
     * @param key     数据键名
     * @param u       节点索引u（不适用时传-1）
     * @param v       节点索引v（不适用时传-1）
     * @param i       物品索引i（不适用时传-1）
     * @param t       时间索引t（不适用时传-1）
     * @param value   数据值（浮点数）
     *
     * @details
     * 便捷重载：将浮点数强制转换为整数后写入。
     * @warning 会丢失小数部分（不是四舍五入，而是直接截断）
     *
     * @note 如果需要保留小数，请先转为字符串后使用字符串版本的writeRow
     */
    void writeRow(const std::string& section, const std::string& key,
                  int u, int v, int i, int t,
                  double value);

private:
    std::ofstream ofs_;         ///< 输出文件流
    bool wrote_header_ = false; ///< 表头是否已写入的标志

    /**
     * @brief 如果尚未写入表头，则写入表头
     *
     * @details
     * 表头格式：section,key,u,v,i,t,value
     * 此方法会在第一次调用writeRow时被自动调用。
     */
    void writeHeaderIfNeeded();

    /**
     * @brief 对字符串进行CSV转义
     *
     * @param s 待转义的字符串
     * @return std::string 转义后的字符串
     *
     * @details
     * CSV转义规则：
     * - 如果字符串包含逗号、双引号、换行符或回车符，则用双引号包围
     * - 字符串内的双引号需要转义为两个双引号
     * - 如果不包含特殊字符，则原样返回
     *
     * 示例：
     * - "hello" -> "hello"
     * - "hello,world" -> "\"hello,world\""
     * - "say \"hi\"" -> "\"say \"\"hi\"\"\""
     */
    static std::string escape(const std::string& s);

    /**
     * @brief 将整数转换为字符串，-1转换为空字符串
     *
     * @param val 整数值
     * @return std::string 转换后的字符串（-1返回空字符串）
     *
     * @details
     * 在CSV中，我们使用空字段表示"不适用"的索引。
     * 例如：meta段的数据不需要u,v,i,t索引，这些字段会显示为空。
     */
    static std::string toStringOrEmpty(int val);
};

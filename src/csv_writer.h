#pragma once
#include <fstream>
#include <string>

/**
 * CsvWriter
 * 负责安全、简单地写出一行 schema 固定的记录：
 *   section,key,u,v,i,t,value
 * - 提供字符串/数值重载
 * - 自动写出表头
 * - 自动完成 CSV 转义（逗号、双引号、换行）
 * - 使用 -1 表示空值
 */
class CsvWriter {
public:
    explicit CsvWriter(const std::string& path);
    ~CsvWriter();

    CsvWriter(const CsvWriter&) = delete;
    CsvWriter& operator=(const CsvWriter&) = delete;

    // 核心：写出一行（使用 -1 表示空值）
    void writeRow(const std::string& section,
                  const std::string& key,
                  int u, int v, int i, int t,
                  const std::string& value);

    // 便捷重载：整型值
    void writeRow(const std::string& section, const std::string& key,
                  int u, int v, int i, int t,
                  int value);

    // 便捷重载：浮点值（转为整数）
    void writeRow(const std::string& section, const std::string& key,
                  int u, int v, int i, int t,
                  double value);

private:
    std::ofstream ofs_;
    bool wrote_header_ = false;
    void writeHeaderIfNeeded();
    static std::string escape(const std::string& s);
    static std::string toStringOrEmpty(int val);
};

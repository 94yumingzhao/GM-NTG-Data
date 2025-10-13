#include "csv_writer.h"
#include <sstream>
#include <stdexcept>

CsvWriter::CsvWriter(const std::string& path) : ofs_(path, std::ios::out | std::ios::trunc) {
    if (!ofs_) throw std::runtime_error("无法打开输出文件: " + path);
}

CsvWriter::~CsvWriter() {
    ofs_.flush();
}

void CsvWriter::writeHeaderIfNeeded() {
    if (!wrote_header_) {
        ofs_ << "section,key,u,v,i,t,value\n";
        wrote_header_ = true;
    }
}

std::string CsvWriter::escape(const std::string& s) {
    bool need_quote = false;
    for (char c : s) {
        if (c == ',' || c == '"' || c == '\n' || c == '\r') { need_quote = true; break; }
    }
    if (!need_quote) return s;
    std::string out; out.reserve(s.size()+2);
    out.push_back('"');
    for (char c : s) {
        if (c == '"') out += "\"\""; else out.push_back(c);
    }
    out.push_back('"');
    return out;
}

std::string CsvWriter::toStringOrEmpty(int val) {
    return (val < 0) ? "" : std::to_string(val);
}

void CsvWriter::writeRow(const std::string& section,
                         const std::string& key,
                         int u, int v, int i, int t,
                         const std::string& value) {
    writeHeaderIfNeeded();
    ofs_  << escape(section) << ','
          << escape(key)     << ','
          << toStringOrEmpty(u) << ','
          << toStringOrEmpty(v) << ','
          << toStringOrEmpty(i) << ','
          << toStringOrEmpty(t) << ','
          << escape(value) << '\n';
}

void CsvWriter::writeRow(const std::string& section, const std::string& key,
                         int u, int v, int i, int t,
                         int value) {
    writeRow(section, key, u, v, i, t, std::to_string(value));
}

void CsvWriter::writeRow(const std::string& section, const std::string& key,
                         int u, int v, int i, int t,
                         double value) {
    // 将浮点数转换为整数输出
    int int_value = static_cast<int>(value);
    writeRow(section, key, u, v, i, t, std::to_string(int_value));
}

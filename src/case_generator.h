#pragma once
#include "csv_writer.h"
#include <vector>
#include <string>
#include <stdexcept>

/**
 * 数据模型（与既有 GameValues 对齐）
 * 说明：所有索引均为 0-based
 */

struct DemandEntry { int u, i, t; double amount; };
struct CapacityOverride { int u, t; double value; };
struct I0Override { int u, i; double value; };
struct TransferEntry { int u, v, i, t; double cost; };
struct BigMEntry { int i, t; double M; };

struct GeneratorConfig {
    // 规模与开关
    int U = 0, I = 0, T = 0;
    bool enable_transfer = false;

    // 成本（长度必须等于 I）
    std::vector<double> cX, cY, cI;

    // 产能占用（长度等于 I）
    std::vector<double> sX, sY;

    // 产能与初始库存默认值 + 覆盖
    double default_capacity = 0.0;
    double default_i0 = 0.0;
    std::vector<CapacityOverride> capacity_overrides;
    std::vector<I0Override> i0_overrides;

    // 需求（稀疏表，未出现的默认为 0）
    std::vector<DemandEntry> demand;

    // 启用转运时的成本与大 M
    std::vector<TransferEntry> transfer_costs; // cT[u,v,i,t]
    std::vector<BigMEntry> bigM;               // M[i,t]

    // 求解器参数
    double mip_gap = 1e-6;
    int time_limit_sec = 60;
    int threads = 0;
    double sep_violation_eps = 1e-8;
    int max_iters = 50;
};

class CaseGenerator {
public:
    static void Validate(const GeneratorConfig& gc);
    static void GenerateCsv(const GeneratorConfig& gc, CsvWriter& w);
};

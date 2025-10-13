#include "case_generator.h"
#include "logger.h"
#include "demand_generator.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

/**
 * LS-Game-DataGen - 算例生成器（无CLI版本）
 *
 * 使用方法：
 * 1. 在下方 main() 函数中直接修改 GeneratorConfig 的参数
 * 2. 编译并运行程序
 * 3. 算例文件会自动生成到 output/ 目录
 *
 * 所有参数说明请参考 case_generator.h 中的 GeneratorConfig 结构体
 */

int main() {
    Logger logger;

    try {
        logger.log("==================== LS-Game-DataGen 启动 ====================");
        // ==================== 基本规模参数 ====================
        int U = 5;    // 节点数量
        int I = 300;    // 物品种类数量
        int T = 20;    // 时间周期数量

        // 是否启用转运功能
        bool enable_transfer = false;

        // ==================== 单位成本参数 ====================
        double unit_cX = 1.0;    // X方向生产成本（统一值）
        double unit_cY = 60.0;    // Y方向生产成本（统一值）
        double unit_cI = 1.0;    // 库存持有成本（统一值）

        // ==================== 单位产能占用参数 ====================
        double unit_sX = 1.0;    // X方向产能占用（统一值）
        double unit_sY = 0.0;    // Y方向产能占用（统一值）

        // ==================== 默认值 ====================
        double default_capacity = 1440;    // 默认产能
        double default_i0 = 0.0;          // 默认初始库存

        // ==================== 需求生成参数 ====================
        double min_demand = 10.0;         // 最小需求量
        double max_demand = 100.0;        // 最大需求量
        double demand_density = 0.3;      // 需求密度（0.0-1.0）
        unsigned int demand_seed = 42;    // 随机种子

        DemandGenConfig::Mode demand_mode = DemandGenConfig::Mode::PER_ITEM_PER_TIME;

        // ==================== 求解器参数 ====================
        double mip_gap = 1e-6;            // MIP 求解间隙
        int time_limit_sec = 60;          // 时间限制（秒）
        int threads = 0;                  // 线程数（0=自动）
        double sep_violation_eps = 1e-8;  // 分离违反阈值
        int max_iters = 50;               // 最大迭代次数

        // ==================== 自动构建配置 ====================
        GeneratorConfig gc;
        gc.U = U;
        gc.I = I;
        gc.T = T;
        gc.enable_transfer = enable_transfer;

        // 自动填充成本向量（所有物品使用统一成本）
        gc.cX.assign(I, unit_cX);
        gc.cY.assign(I, unit_cY);
        gc.cI.assign(I, unit_cI);

        // 自动填充产能占用向量（所有物品使用统一占用）
        gc.sX.assign(I, unit_sX);
        gc.sY.assign(I, unit_sY);

        gc.default_capacity = default_capacity;
        gc.default_i0 = default_i0;

        // ==================== 覆盖配置（可选）====================
        // 如需覆盖特定节点/时间的产能，添加到 capacity_overrides
        // gc.capacity_overrides.push_back({0, 0, 10.0});  // 节点0，时间0，产能10.0

        // 如需覆盖特定节点/物品的初始库存，添加到 i0_overrides
        // gc.i0_overrides.push_back({0, 0, 5.0});  // 节点0，物品0，初始库存5.0

        // ==================== 需求配置 ====================
        // 自动生成需求
        DemandGenConfig demand_config;
        demand_config.U = U;
        demand_config.I = I;
        demand_config.T = T;
        demand_config.min_demand = min_demand;
        demand_config.max_demand = max_demand;
        demand_config.density = demand_density;
        demand_config.random_seed = demand_seed;
        demand_config.mode = demand_mode;

        logger.log("生成需求数据...");
        logger.log("需求生成模式: " + DemandGenerator::GetModeName(demand_mode));
        logger.log("需求量范围: [" + std::to_string(min_demand) + ", " + std::to_string(max_demand) + "]");
        logger.log("需求密度: " + std::to_string(demand_density));

        gc.demand = DemandGenerator::Generate(demand_config);

        logger.log("生成需求数量: " + std::to_string(gc.demand.size()));

        // ==================== 转运配置（仅当 enable_transfer=true 时需要）====================
        // 转运成本格式：{源节点u, 目标节点v, 物品i, 时间t, 成本}
        // gc.transfer_costs.push_back({0, 1, 0, 0, 2.5});

        // BigM 约束格式：{物品i, 时间t, M值}
        // gc.bigM.push_back({0, 0, 1000.0});

        // ==================== 应用求解器参数 ====================
        gc.mip_gap = mip_gap;
        gc.time_limit_sec = time_limit_sec;
        gc.threads = threads;
        gc.sep_violation_eps = sep_violation_eps;
        gc.max_iters = max_iters;

        // ==================== 生成算例 ====================
        // 生成带时间戳的文件名：case_YYYYMMDD_HHMMSS.csv
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;
        localtime_s(&tm_now, &time_t_now);

        std::ostringstream filename;
        filename << "output/case_"
                 << std::setfill('0')
                 << std::setw(4) << (tm_now.tm_year + 1900)
                 << std::setw(2) << (tm_now.tm_mon + 1)
                 << std::setw(2) << tm_now.tm_mday
                 << "_"
                 << std::setw(2) << tm_now.tm_hour
                 << std::setw(2) << tm_now.tm_min
                 << std::setw(2) << tm_now.tm_sec
                 << ".csv";

        std::string output_file = filename.str();

        logger.log("开始生成算例...");
        logger.log("配置: U=" + std::to_string(gc.U) +
                   ", I=" + std::to_string(gc.I) +
                   ", T=" + std::to_string(gc.T));
        logger.log("转运功能: " + std::string(gc.enable_transfer ? "启用" : "未启用"));

        CsvWriter writer(output_file);
        CaseGenerator::GenerateCsv(gc, writer);

        logger.log("算例生成成功!");
        logger.log("输出文件: " + output_file);
        logger.log("==================== LS-Game-DataGen 完成 ====================");

        logger.saveToFile();

        return 0;
    } catch (const std::exception& ex) {
        logger.log("[错误] " + std::string(ex.what()));
        logger.saveToFile();
        return 1;
    }
}

#include "case_generator.h"
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
    try {
        // ==================== 基本规模参数 ====================
        int U = 2;    // 节点数量
        int I = 2;    // 物品种类数量
        int T = 3;    // 时间周期数量

        // 是否启用转运功能
        bool enable_transfer = false;

        // ==================== 单位成本参数 ====================
        double unit_cX = 2.0;    // X方向生产成本（统一值）
        double unit_cY = 5.0;    // Y方向生产成本（统一值）
        double unit_cI = 0.4;    // 库存持有成本（统一值）

        // ==================== 单位产能占用参数 ====================
        double unit_sX = 1.0;    // X方向产能占用（统一值）
        double unit_sY = 0.0;    // Y方向产能占用（统一值）

        // ==================== 默认值 ====================
        double default_capacity = 5.0;    // 默认产能
        double default_i0 = 0.0;          // 默认初始库存

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
        // 格式：{节点u, 物品i, 时间t, 需求量}

        gc.demand.push_back({0, 0, 0, 10.0});
        gc.demand.push_back({0, 1, 1, 15.0});
        gc.demand.push_back({1, 0, 2, 8.0});

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

        CsvWriter writer(output_file);
        CaseGenerator::GenerateCsv(gc, writer);

        std::cout << "算例生成成功!\n";
        std::cout << "输出文件: " << output_file << "\n";
        std::cout << "规模: U=" << gc.U << ", I=" << gc.I << ", T=" << gc.T << "\n";
        std::cout << "转运功能: " << (gc.enable_transfer ? "启用" : "未启用") << "\n";

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "[错误] " << ex.what() << "\n";
        return 1;
    }
}

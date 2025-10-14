/**
 * ==================================================================================
 * @file        main.cpp
 * @brief       LS-Game-DataGen 主程序 (Version 2.0 - 产能驱动)
 * @version     2.0.0
 * @date        2025-10-14
 *
 * @description
 * LS-Game-DataGen v2.0 采用全新的产能驱动生成策略，从设计上保证生成的
 * 算例必然可行。
 *
 * 主要改进：
 * - 产能驱动：先分配产能，再生成需求
 * - 按时段控制：确保每个时段的产能不被超出
 * - 可行性保证：通过设计保证算例必然可行
 * - 参数直观：所有参数都有清晰的业务含义
 *
 * 使用方法：
 * 1. 在main()函数中配置参数
 * 2. 编译并运行程序
 * 3. 生成的算例保存到 output/ 目录
 *
 * 输出格式：
 * - 算例文件: output/case_YYYYMMDD_HHMMSS.csv
 * - 日志文件: output/log_YYYYMMDD_HHMMSS.txt
 *
 * @author      LS-Game-DataGen Team (v2.0)
 * ==================================================================================
 */

#include "case_generator.h"
#include "logger.h"
#include "demand_generator.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <fstream>

/**
 * @brief 主函数 - 程序入口点
 *
 * @return int 返回0表示成功，返回1表示出现异常
 */
int main() {
    // 创建日志对象，用于记录程序运行过程和结果
    Logger logger;

    try {
        logger.log("==================== LS-Game-DataGen v2.0 启动 ====================");
        logger.log("采用产能驱动生成策略，保证算例可行性");

        //==============================================================================
        // 第一部分：基本规模参数配置
        //==============================================================================

        int U = 5;      // 节点数量
                        // 表示供应链网络中的节点数量
                        // 取值范围：正整数，通常为 3-10

        int I = 300;    // 物品种类数量
                        // 表示需要生产的不同物品种类
                        // 取值范围：正整数，通常为 50-500

        int T = 20;     // 时间周期数量
                        // 表示规划的时间跨度
                        // 取值范围：正整数，通常为 10-50

        bool enable_transfer = false;  // 是否启用节点间转运功能
                                      // false: 不允许转运（简化问题）
                                      // true: 允许转运（增加复杂度）

        //==============================================================================
        // 第二部分：产能参数配置
        //==============================================================================

        double default_capacity = 1440.0;  // 默认产能（单位：时间）
                                          // 每个节点每个时间周期的可用产能
                                          // 示例：1440 = 24小时 * 60分钟

        double unit_sX = 1.0;             // 单位产品产能占用（单位：时间/件）
                                         // 生产1件产品需要的时间
                                         // 示例：1.0 表示生产1件产品需要1分钟

        double unit_sY = 10.0;            // 启动产能占用（单位：时间/次）
                                         // 启动生产一种产品需要的准备时间
                                         // 示例：10.0 表示每次启动需要10分钟

        //==============================================================================
        // 第三部分：需求生成参数配置（核心参数）
        //==============================================================================

        double capacity_utilization = 0.85;  // 目标产能利用率（0.0-1.0）
                                            // 控制生成的需求占用多少产能
                                            // 0.85 表示使用85%的可用产能
                                            // 建议范围：0.70-0.90

        double demand_intensity = 0.15;     // 需求密度（0.0-1.0）
                                           // 控制生成多少个需求点
                                           // 0.15 表示15%的(u,i,t)组合有需求
                                           // 建议范围：0.10-0.30

        double initial_inventory_ratio = 0.0;  // 初始库存比例（0.0-1.0）
                                              // 初始库存占平均需求的比例
                                              // 0.0 = 无初始库存
                                              // 0.5 = 初始库存为平均需求的50%
                                              // 建议范围：0.0-0.3

        //==============================================================================
        // 第四部分：分布控制参数
        //==============================================================================

        double time_concentration = 0.2;    // 时间分布集中度（0.0-1.0）
                                           // 控制需求在时间维度的分布
                                           // 0.0 = 均匀分布在所有时段
                                           // 1.0 = 高度集中在少数时段
                                           // 建议范围：0.1-0.3

        double node_concentration = 0.3;    // 节点分布集中度（0.0-1.0）
                                           // 控制需求在节点维度的分布
                                           // 0.0 = 均匀分布在所有节点
                                           // 1.0 = 集中在少数节点
                                           // 建议范围：0.2-0.4

        double item_concentration = 0.3;    // 物品分布集中度（0.0-1.0）
                                           // 控制需求在物品维度的分布
                                           // 0.0 = 均匀分布在所有物品
                                           // 1.0 = 集中在少数物品
                                           // 建议范围：0.2-0.4

        double demand_size_variance = 0.3;  // 需求量方差（0.0-1.0）
                                           // 控制需求量的离散程度
                                           // 0.0 = 所有需求量相同
                                           // 1.0 = 需求量变化很大
                                           // 建议范围：0.2-0.5

        //==============================================================================
        // 第五部分：成本参数配置
        //==============================================================================

        bool use_varied_costs = true;  // 是否使用变化的成本
                                      // true: 每个物品有不同的成本（更真实）
                                      // false: 所有物品使用统一成本（简化）

        // 统一成本值（当 use_varied_costs = false 时使用）
        double unit_cX = 1.0;    // X方向生产成本（单位：元/件）
        double unit_cY = 1.0;    // Y方向生产成本（单位：元/批次）
        double unit_cI = 1.0;    // 库存持有成本（单位：元/件/周期）

        // 变动成本范围（当 use_varied_costs = true 时使用）
        double cY_min = 1.0;     // Y方向成本最小值
        double cY_max = 1.0;     // Y方向成本最大值
        double cI_min = 1.0;     // 库存成本最小值
        double cI_max = 1.0;     // 库存成本最大值

        //==============================================================================
        // 第六部分：求解器参数配置
        //==============================================================================

        double mip_gap = 1e-6;            // MIP求解间隙
        int time_limit_sec = 60;          // 时间限制（秒）
        int threads = 0;                  // 并行线程数（0=自动）
        double sep_violation_eps = 1e-8;  // 分离违反阈值
        int max_iters = 50;               // 最大迭代次数

        //==============================================================================
        // 第七部分：随机种子
        //==============================================================================

        unsigned int demand_seed = 42;  // 随机种子
                                       // 相同种子产生相同的需求
                                       // 便于实验的可重复性

        //==============================================================================
        // 第八部分：构建配置对象
        //==============================================================================

        // 构建算例生成配置
        GeneratorConfig gc;
        gc.U = U;
        gc.I = I;
        gc.T = T;
        gc.enable_transfer = enable_transfer;

        // 填充成本向量
        if (use_varied_costs) {
            std::mt19937 cost_rng(demand_seed + 1000);
            std::uniform_real_distribution<double> cY_dist(cY_min, cY_max);
            std::uniform_real_distribution<double> cI_dist(cI_min, cI_max);

            gc.cX.assign(I, unit_cX);

            for (int i = 0; i < I; ++i) {
                gc.cY.push_back(cY_dist(cost_rng));
                gc.cI.push_back(cI_dist(cost_rng));
            }
        } else {
            gc.cX.assign(I, unit_cX);
            gc.cY.assign(I, unit_cY);
            gc.cI.assign(I, unit_cI);
        }

        // 填充产能占用向量
        gc.sX.assign(I, unit_sX);
        gc.sY.assign(I, unit_sY);

        // 设置默认值
        gc.default_capacity = default_capacity;

        // 根据initial_inventory_ratio计算初始库存
        // 需要先估算平均需求量
        double total_capacity = U * T * default_capacity;
        double estimated_setup_overhead = U * T * I * demand_intensity * unit_sY;
        double available_production_capacity = total_capacity - estimated_setup_overhead;
        double estimated_total_demand = available_production_capacity * capacity_utilization / unit_sX;
        int estimated_demand_points = static_cast<int>(U * I * T * demand_intensity);
        double avg_demand = (estimated_demand_points > 0) ?
                           (estimated_total_demand / estimated_demand_points) : 0;

        gc.default_i0 = avg_demand * initial_inventory_ratio;

        //==============================================================================
        // 第九部分：使用v2生成器生成需求数据
        //==============================================================================

        logger.log("使用产能驱动生成器生成需求数据...");

        // 构建需求生成配置对象
        DemandGenConfig demand_config;
        demand_config.U = U;
        demand_config.I = I;
        demand_config.T = T;
        demand_config.default_capacity = default_capacity;
        demand_config.unit_sX = unit_sX;
        demand_config.unit_sY = unit_sY;
        demand_config.capacity_utilization = capacity_utilization;
        demand_config.demand_intensity = demand_intensity;
        demand_config.initial_inventory_ratio = initial_inventory_ratio;
        demand_config.time_concentration = time_concentration;
        demand_config.node_concentration = node_concentration;
        demand_config.item_concentration = item_concentration;
        demand_config.random_seed = demand_seed;
        demand_config.demand_size_variance = demand_size_variance;

        // 记录配置参数到日志
        logger.log("配置参数：");
        logger.log("  规模: U=" + std::to_string(U) + ", I=" + std::to_string(I) + ", T=" + std::to_string(T));
        logger.log("  产能: default=" + std::to_string(default_capacity) +
                   ", sX=" + std::to_string(unit_sX) +
                   ", sY=" + std::to_string(unit_sY));
        logger.log("  利用率: " + std::to_string(capacity_utilization * 100) + "%");
        logger.log("  需求密度: " + std::to_string(demand_intensity * 100) + "%");
        logger.log("  时间集中度: " + std::to_string(time_concentration));
        logger.log("  初始库存比例: " + std::to_string(initial_inventory_ratio));

        // 调用生成器生成需求数据
        gc.demand = DemandGenerator::Generate(demand_config);

        // 记录生成的需求数量
        logger.log("生成需求数量: " + std::to_string(gc.demand.size()));

        // 计算并记录统计信息
        if (!gc.demand.empty()) {
            double total_demand_amount = 0.0;
            for (const auto& d : gc.demand) {
                total_demand_amount += d.amount;
            }
            double avg_demand_amount = total_demand_amount / gc.demand.size();
            logger.log("总需求量: " + std::to_string(total_demand_amount));
            logger.log("平均需求量: " + std::to_string(avg_demand_amount));

            // 计算产能使用率
            double total_production_capacity = total_demand_amount * unit_sX;
            double total_available_capacity = U * T * default_capacity;
            double actual_utilization = total_production_capacity / total_available_capacity;
            logger.log("实际产能利用率: " + std::to_string(actual_utilization * 100) + "%");
        }

        //==============================================================================
        // 第十部分：应用求解器参数
        //==============================================================================

        gc.mip_gap = mip_gap;
        gc.time_limit_sec = time_limit_sec;
        gc.threads = threads;
        gc.sep_violation_eps = sep_violation_eps;
        gc.max_iters = max_iters;

        //==============================================================================
        // 第十一部分：生成输出文件名
        //==============================================================================

        // 获取当前系统时间
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;
        localtime_s(&tm_now, &time_t_now);

        // 获取项目根目录路径
        std::string project_root = "";
        std::string current_dir = ".";
        for (int i = 0; i < 5; ++i) {
            std::ifstream cmake_file(current_dir + "/CMakeLists.txt");
            if (cmake_file.good()) {
                project_root = current_dir;
                break;
            }
            current_dir = "../" + current_dir;
        }

        if (project_root.empty()) {
            project_root = ".";
        }

        std::string output_dir = project_root + "/output";

        // 确保output目录存在
        #ifdef _WIN32
            system(("if not exist \"" + output_dir + "\" mkdir \"" + output_dir + "\"").c_str());
        #else
            system(("mkdir -p \"" + output_dir + "\"").c_str());
        #endif

        // 构建文件名
        std::ostringstream filename;
        filename << output_dir << "/case_"
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

        //==============================================================================
        // 第十二部分：生成CSV算例文件
        //==============================================================================

        logger.log("开始生成算例文件...");
        logger.log("转运功能: " + std::string(gc.enable_transfer ? "启用" : "未启用"));

        // 创建CSV写入器
        CsvWriter writer(output_file);

        // 调用生成器生成CSV文件（与v1.0格式兼容）
        CaseGenerator::GenerateCsv(gc, writer);

        // 记录成功信息
        logger.log("算例生成成功!");
        logger.log("输出文件: " + output_file);
        logger.log("==================== LS-Game-DataGen v2.0 完成 ====================");

        // 保存日志到文件
        logger.saveToFile();

        return 0;

    } catch (const std::exception& ex) {
        //==============================================================================
        // 异常处理
        //==============================================================================

        logger.log("[错误] " + std::string(ex.what()));
        logger.saveToFile();
        return 1;
    }
}

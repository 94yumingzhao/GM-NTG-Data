/**
 * ==================================================================================
 * @file        main.cpp
 * @brief       LS-Game-DataGen 主程序入口
 * @version     1.0.0
 * @date        2025-10-13
 *
 * @description
 * LS-Game-DataGen (Lot Sizing Game Data Generator) - 批量大小游戏数据生成器
 *
 * 本程序用于生成批量大小问题(Lot Sizing Problem)的测试算例数据。
 * 通过配置各种参数，可以生成不同规模、不同难度的算例，用于测试优化算法。
 *
 * 主要功能：
 * - 支持多节点、多物品、多时段的复杂场景
 * - 灵活配置成本参数（生产成本、库存成本等）
 * - 支持产能约束和初始库存设置
 * - 提供多种需求生成模式
 * - 可选的节点间转运功能
 * - 自动生成带时间戳的CSV格式输出文件
 *
 * 使用方法：
 * 1. 在 main() 函数中直接修改各项配置参数
 * 2. 编译并运行程序
 * 3. 生成的算例文件会自动保存到 output/ 目录
 * 4. 运行日志会同时保存到 output/ 目录
 *
 * 输出格式：
 * - 算例文件: output/case_YYYYMMDD_HHMMSS.csv
 * - 日志文件: output/log_YYYYMMDD_HHMMSS.txt
 *
 * @note 所有索引均为0-based（从0开始计数）
 * @note 所有参数详细说明请参考 case_generator.h 中的 GeneratorConfig 结构体
 *
 * @author      LS-Game-DataGen Team
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
 *
 * @details
 * 主函数负责以下工作：
 * 1. 初始化日志系统
 * 2. 配置算例生成参数
 * 3. 调用需求生成器生成需求数据
 * 4. 调用算例生成器生成CSV文件
 * 5. 处理异常并保存日志
 *
 * @note 所有配置参数都在函数内直接定义，便于快速修改和测试
 */
int main() {
    // 创建日志对象，用于记录程序运行过程和结果
    Logger logger;

    try {
        logger.log("==================== LS-Game-DataGen 启动 ====================");

        // ====================================================================================
        // 第一部分：基本规模参数配置
        // ====================================================================================
        // 这些参数定义了批量大小问题的基本规模，直接影响问题的复杂度和求解难度

        int U = 5;      // 节点数量 (Number of Nodes)
                        // 表示供应链网络中的节点数量，每个节点可以生产和存储物品
                        // 取值范围：正整数，通常为 1-10
                        // 示例：5 表示有5个生产/仓储节点

        int I = 300;    // 物品种类数量 (Number of Items)
                        // 表示需要规划的不同物品的种类数量
                        // 取值范围：正整数，通常为 1-1000
                        // 示例：300 表示有300种不同的物品需要生产和管理

        int T = 20;     // 时间周期数量 (Number of Time Periods)
                        // 表示规划的时间跨度，每个周期可能代表天、周、月等
                        // 取值范围：正整数，通常为 1-100
                        // 示例：20 表示规划20个时间周期

        // 转运功能开关
        bool enable_transfer = false;  // 是否启用节点间转运功能
                                      // true: 允许物品在节点间转运，增加问题复杂度
                                      // false: 不允许转运，每个节点独立生产和存储

        // ====================================================================================
        // 第二部分：成本参数配置
        // ====================================================================================
        // 成本参数是优化目标的核心，影响最优生产计划的决策

        bool use_varied_costs = true;  // 是否使用变化的成本（增加问题难度）
                                      // true: 每个物品有不同的成本（更真实、更难）
                                      // false: 所有物品使用统一成本（简化版本）

        // 统一成本值（当 use_varied_costs = false 时使用）
        double unit_cX = 1.0;    // X方向生产成本（单位：元/件）
                                // 表示生产单位物品的变动成本（如材料、人工等）

        double unit_cY = 1.0;    // Y方向生产成本（单位：元/批次）
                                // 表示启动一次生产的固定成本（如设备调试、换模等）

        double unit_cI = 1.0;    // 库存持有成本（单位：元/件/周期）
                                // 表示单位物品单位时间的库存成本（如仓储、资金占用等）

        // 变动成本范围（当 use_varied_costs = true 时使用）
        double cY_min = 1.0;     // Y方向成本最小值
        double cY_max = 1.0;     // Y方向成本最大值
                                // 每个物品的cY将在 [cY_min, cY_max] 范围内随机生成

        double cI_min = 1.0;     // 库存成本最小值
        double cI_max = 1.0;     // 库存成本最大值
                                // 每个物品的cI将在 [cI_min, cI_max] 范围内随机生成

        // ====================================================================================
        // 第三部分：产能占用参数配置
        // ====================================================================================
        // 产能约束是批量大小问题的核心约束之一

        double unit_sX = 1.0;    // X方向产能占用（单位：时/件）
                                // 表示生产单位物品消耗的生产时间或资源
                                // 示例：1.0 表示生产1件物品需要1单位时间

        double unit_sY = 10.0;   // Y方向产能占用（单位：时/批次）
                                // 表示启动一次生产消耗的准备时间或资源
                                // 示例：10.0 表示每次启动生产需要10单位时间

        // ====================================================================================
        // 第四部分：默认值配置
        // ====================================================================================

        double default_capacity = 1440;  // 默认产能（单位：时）
                                        // 每个节点每个时间周期的可用产能
                                        // 示例：1440 表示每天24小时 * 60分钟

        double default_i0 = 0.0;        // 默认初始库存（单位：件）
                                        // 规划期初始时刻各节点各物品的库存量
                                        // 通常设为0，表示从零库存开始规划

        // ====================================================================================
        // 第五部分：需求生成参数配置
        // ====================================================================================
        // 需求是批量大小问题的输入，决定了生产计划的目标

        double min_demand = 10.0;        // 最小需求量（单位：件）
                                        // 每个需求点的需求量下限

        double max_demand = 20.0;       // 最大需求量（单位：件）
                                        // 每个需求点的需求量上限
                                        // 实际需求量将在 [min_demand, max_demand] 范围内随机生成

        double demand_density = 0.15;    // 需求密度（取值范围：0.0-1.0）
                                        // 表示有需求的 (节点,物品,时间) 组合占总组合数的比例
                                        // 0.15 表示只有15%的组合有需求（稀疏需求）
                                        // 1.0 表示所有组合都有需求（密集需求）

        double capacity_tightness = 0.8; // 产能紧张度（取值范围：>0）
                                        // 影响需求与产能的关系
                                        // >1.0: 某些时段需求可能超过产能（紧张）
                                        // <1.0: 产能充足（宽松）
                                        // =1.0: 需求与产能基本平衡

        double demand_concentration = 0.3; // 需求集中度（取值范围：0.0-1.0）
                                          // 表示需求在时间维度上的集中程度
                                          // 0.0: 需求均匀分布在各时段
                                          // 1.0: 需求高度集中在少数时段
                                          // 0.3: 30%的需求集中在特定时段

        unsigned int demand_seed = 42;    // 随机种子
                                         // 用于控制需求生成的随机性，相同种子产生相同需求
                                         // 便于实验的可重复性

        // 需求生成模式选择
        DemandGenConfig::Mode demand_mode = DemandGenConfig::Mode::CAPACITY_AWARE;
        // 可选模式：
        // - ALL_COMBINATIONS: 遍历所有(u,i,t)组合，按密度随机决定是否有需求
        // - SPARSE_RANDOM: 从所有组合中随机选择指定比例的组合作为需求点
        // - PER_ITEM_PER_TIME: 每个(物品,时间)组合随机选择一个节点产生需求
        // - PER_NODE_PER_TIME: 每个(节点,时间)组合随机选择若干物品产生需求
        // - CAPACITY_AWARE: 考虑产能约束的智能需求生成（推荐）

        // ====================================================================================
        // 第六部分：求解器参数配置
        // ====================================================================================
        // 这些参数用于控制优化求解器的行为（如果后续使用求解器求解）

        double mip_gap = 1e-6;            // MIP求解间隙（相对误差容忍度）
                                         // 当目标函数的相对误差小于此值时停止求解
                                         // 示例：1e-6 表示0.0001%的误差容忍度

        int time_limit_sec = 60;          // 时间限制（单位：秒）
                                         // 求解器的最大运行时间

        int threads = 0;                  // 并行线程数
                                         // 0 表示自动选择（通常为CPU核心数）
                                         // >0 表示使用指定数量的线程

        double sep_violation_eps = 1e-8;  // 分离违反阈值
                                         // 用于割平面算法的阈值参数

        int max_iters = 50;               // 最大迭代次数
                                         // 某些迭代算法的最大迭代次数限制

        // ====================================================================================
        // 第七部分：构建配置对象
        // ====================================================================================
        // 将上面配置的所有参数组装成 GeneratorConfig 对象

        GeneratorConfig gc;  // 创建配置对象
        gc.U = U;
        gc.I = I;
        gc.T = T;
        gc.enable_transfer = enable_transfer;

        // 填充成本向量
        // 根据 use_varied_costs 标志决定使用统一成本还是随机成本
        if (use_varied_costs) {
            // 使用变化的成本：每个物品有不同的成本值
            std::mt19937 cost_rng(demand_seed + 1000);  // 创建随机数生成器（使用不同种子）
            std::uniform_real_distribution<double> cY_dist(cY_min, cY_max);  // cY的分布
            std::uniform_real_distribution<double> cI_dist(cI_min, cI_max);  // cI的分布

            // X方向成本统一使用 unit_cX
            gc.cX.assign(I, unit_cX);

            // Y方向成本和库存成本随机生成
            for (int i = 0; i < I; ++i) {
                gc.cY.push_back(cY_dist(cost_rng));  // 从分布中采样
                gc.cI.push_back(cI_dist(cost_rng));
            }
        } else {
            // 使用统一成本：所有物品使用相同的成本值
            gc.cX.assign(I, unit_cX);
            gc.cY.assign(I, unit_cY);
            gc.cI.assign(I, unit_cI);
        }

        // 填充产能占用向量
        // 所有物品使用统一的产能占用参数
        gc.sX.assign(I, unit_sX);
        gc.sY.assign(I, unit_sY);

        // 设置默认值
        gc.default_capacity = default_capacity;
        gc.default_i0 = default_i0;

        // ====================================================================================
        // 第八部分：覆盖配置（可选）
        // ====================================================================================
        // 如果需要为特定的节点/时间/物品设置特殊值，可以在这里添加覆盖项

        // 产能覆盖示例：
        // 如需覆盖特定节点在特定时间的产能，添加到 capacity_overrides
        // gc.capacity_overrides.push_back({0, 0, 10.0});  // 节点0，时间0，产能10.0
        // gc.capacity_overrides.push_back({1, 5, 20.0});  // 节点1，时间5，产能20.0

        // 初始库存覆盖示例：
        // 如需覆盖特定节点特定物品的初始库存，添加到 i0_overrides
        // gc.i0_overrides.push_back({0, 0, 5.0});   // 节点0，物品0，初始库存5.0
        // gc.i0_overrides.push_back({1, 2, 10.0});  // 节点1，物品2，初始库存10.0

        // ====================================================================================
        // 第九部分：生成需求数据
        // ====================================================================================
        // 使用需求生成器根据配置参数自动生成需求数据

        // 构建需求生成配置对象
        DemandGenConfig demand_config;
        demand_config.U = U;
        demand_config.I = I;
        demand_config.T = T;
        demand_config.min_demand = min_demand;
        demand_config.max_demand = max_demand;
        demand_config.density = demand_density;
        demand_config.default_capacity = default_capacity;    // 新增：产能参数
        demand_config.unit_sX = unit_sX;                      // 新增：产能占用参数
        demand_config.unit_sY = unit_sY;                      // 新增：启动占用参数
        demand_config.capacity_tightness = capacity_tightness;
        demand_config.demand_concentration = demand_concentration;
        demand_config.random_seed = demand_seed;
        demand_config.mode = demand_mode;

        // 记录需求生成信息到日志
        logger.log("生成需求数据...");
        logger.log("需求生成模式: " + DemandGenerator::GetModeName(demand_mode));
        logger.log("需求量范围: [" + std::to_string(min_demand) + ", " + std::to_string(max_demand) + "]");
        logger.log("需求密度: " + std::to_string(demand_density));
        logger.log("产能紧张度: " + std::to_string(capacity_tightness));
        logger.log("需求集中度: " + std::to_string(demand_concentration));

        // 调用需求生成器生成需求数据
        gc.demand = DemandGenerator::Generate(demand_config);

        // 记录生成的需求数量
        logger.log("生成需求数量: " + std::to_string(gc.demand.size()));

        // ====================================================================================
        // 第十部分：转运配置（可选，仅当 enable_transfer=true 时需要）
        // ====================================================================================
        // 如果启用了转运功能，需要配置转运成本和BigM约束

        // 转运成本配置示例：
        // 格式：{源节点u, 目标节点v, 物品i, 时间t, 转运成本}
        // gc.transfer_costs.push_back({0, 1, 0, 0, 2.5});  // 时间0，物品0从节点0转运到节点1的成本为2.5
        // gc.transfer_costs.push_back({1, 0, 0, 1, 3.0});  // 时间1，物品0从节点1转运到节点0的成本为3.0

        // BigM约束配置示例：
        // 格式：{物品i, 时间t, BigM值}
        // BigM值需要足够大，通常设为需求量的若干倍
        // gc.bigM.push_back({0, 0, 1000.0});  // 物品0在时间0的BigM值为1000.0
        // gc.bigM.push_back({1, 0, 1500.0});  // 物品1在时间0的BigM值为1500.0

        // ====================================================================================
        // 第十一部分：应用求解器参数
        // ====================================================================================
        // 将求解器参数写入配置对象
        gc.mip_gap = mip_gap;
        gc.time_limit_sec = time_limit_sec;
        gc.threads = threads;
        gc.sep_violation_eps = sep_violation_eps;
        gc.max_iters = max_iters;

        // ====================================================================================
        // 第十二部分：生成输出文件名
        // ====================================================================================
        // 生成带时间戳的文件名，确保每次运行生成不同的文件
        // 格式：case_YYYYMMDD_HHMMSS.csv

        // 获取当前系统时间
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        std::tm tm_now;
        localtime_s(&tm_now, &time_t_now);  // 转换为本地时间（线程安全版本）

        // 获取项目根目录路径（向上查找包含CMakeLists.txt的目录）
        std::string project_root = "";
        std::string current_dir = ".";
        for (int i = 0; i < 5; ++i) {  // 最多向上查找5级目录
            std::ifstream cmake_file(current_dir + "/CMakeLists.txt");
            if (cmake_file.good()) {
                project_root = current_dir;
                break;
            }
            current_dir = "../" + current_dir;
        }
        
        // 如果没找到，使用当前目录
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

        // 构建文件名字符串
        std::ostringstream filename;
        filename << output_dir << "/case_"
                 << std::setfill('0')  // 使用0填充
                 << std::setw(4) << (tm_now.tm_year + 1900)  // 年份（4位）
                 << std::setw(2) << (tm_now.tm_mon + 1)      // 月份（2位，注意+1）
                 << std::setw(2) << tm_now.tm_mday           // 日期（2位）
                 << "_"
                 << std::setw(2) << tm_now.tm_hour           // 小时（2位）
                 << std::setw(2) << tm_now.tm_min            // 分钟（2位）
                 << std::setw(2) << tm_now.tm_sec            // 秒（2位）
                 << ".csv";

        std::string output_file = filename.str();

        // ====================================================================================
        // 第十三部分：生成CSV算例文件
        // ====================================================================================
        // 使用CaseGenerator生成标准格式的CSV算例文件

        logger.log("开始生成算例...");
        logger.log("配置: U=" + std::to_string(gc.U) +
                   ", I=" + std::to_string(gc.I) +
                   ", T=" + std::to_string(gc.T));
        logger.log("转运功能: " + std::string(gc.enable_transfer ? "启用" : "未启用"));

        // 创建CSV写入器
        CsvWriter writer(output_file);

        // 调用生成器生成CSV文件
        // 该方法会自动验证配置并按标准格式写出所有数据
        CaseGenerator::GenerateCsv(gc, writer);

        // 记录成功信息
        logger.log("算例生成成功!");
        logger.log("输出文件: " + output_file);
        logger.log("==================== LS-Game-DataGen 完成 ====================");

        // 保存日志到文件
        logger.saveToFile();

        return 0;  // 返回0表示成功

    } catch (const std::exception& ex) {
        // ====================================================================================
        // 异常处理
        // ====================================================================================
        // 如果在执行过程中发生任何异常，记录错误信息并保存日志

        logger.log("[错误] " + std::string(ex.what()));
        logger.saveToFile();
        return 1;  // 返回1表示出现错误
    }
}

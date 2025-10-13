/**
 * ==================================================================================
 * @file        demand_generator.h
 * @brief       需求生成器 - 头文件和实现
 * @version     1.0.0
 * @date        2025-10-13
 *
 * @description
 * DemandGenerator 提供灵活的需求数据自动生成功能。
 *
 * 支持5种生成模式：
 * 1. ALL_COMBINATIONS  - 遍历所有组合，按密度随机选择
 * 2. SPARSE_RANDOM     - 从所有组合中随机抽样
 * 3. PER_ITEM_PER_TIME - 每个(物品,时间)选择一个节点
 * 4. PER_NODE_PER_TIME - 每个(节点,时间)选择若干物品
 * 5. CAPACITY_AWARE    - 考虑产能约束的智能生成（推荐）
 *
 * 核心参数：
 * - 需求量范围：[min_demand, max_demand]
 * - 需求密度：控制有需求的组合比例
 * - 产能紧张度：影响需求与产能的关系
 * - 需求集中度：控制需求在时间维度的分布
 *
 * 设计特点：
 * - Header-only：所有实现都在头文件中
 * - 静态方法：无需创建实例
 * - 随机种子可控：保证可重复性
 * - 模式灵活：适应不同测试场景
 *
 * 使用示例：
 * @code
 * DemandGenConfig config;
 * config.U = 5; config.I = 10; config.T = 20;
 * config.min_demand = 10.0; config.max_demand = 100.0;
 * config.density = 0.3;
 * config.mode = DemandGenConfig::Mode::CAPACITY_AWARE;
 * auto demands = DemandGenerator::Generate(config);
 * @endcode
 *
 * @note 生成的需求可能包含重复的(u,i,t)组合，调用方需要处理
 * @note CAPACITY_AWARE模式最接近真实场景，推荐使用
 *
 * @author      LS-Game-DataGen Team
 * ==================================================================================
 */

#pragma once
#include "case_generator.h"
#include <random>
#include <vector>
#include <algorithm>

// ====================================================================================
// 配置结构体
// ====================================================================================

/**
 * @struct DemandGenConfig
 * @brief  需求生成配置结构体
 *
 * @details
 * 包含需求生成所需的所有参数和模式选择。
 * 通过调整这些参数，可以生成不同特征的需求数据：
 * - 稀疏 vs 密集
 * - 均匀 vs 集中
 * - 轻松 vs 紧张
 */
struct DemandGenConfig {
    // ================================================================================
    // 基本规模参数
    // ================================================================================
    int U;  ///< 节点数量
    int I;  ///< 物品种类数量
    int T;  ///< 时间周期数量

    // ================================================================================
    // 需求量参数
    // ================================================================================
    double min_demand = 1.0;    ///< 最小需求量（单位：件）
    double max_demand = 100.0;  ///< 最大需求量（单位：件）
                                ///  实际需求在 [min_demand, max_demand] 范围内均匀随机

    // ================================================================================
    // 需求分布参数
    // ================================================================================
    double density = 1.0;  ///< 需求密度（取值范围：0.0-1.0）
                          ///  表示有需求的(u,i,t)组合占总组合数的比例
                          ///  0.0: 无需求
                          ///  0.5: 50%的组合有需求
                          ///  1.0: 所有组合都有需求

    // ================================================================================
    // 产能相关参数
    // ================================================================================
    double capacity_tightness = 0.8;  ///< 产能紧张度（取值范围：>0）
                                     ///  影响需求与产能的关系
                                     ///  <1.0: 产能充足（宽松）
                                     ///  =1.0: 需求与产能基本平衡
                                     ///  >1.0: 某些时段需求超产能（紧张）

    double time_window_tightness = 0.3;  ///< 时间窗口紧张度（保留参数，暂未使用）
                                        ///  未来可用于控制交期约束的紧张程度

    // ================================================================================
    // 时间分布参数
    // ================================================================================
    double demand_concentration = 0.5;  ///< 需求集中度（取值范围：0.0-1.0）
                                       ///  控制需求在时间维度上的集中程度
                                       ///  0.0: 需求均匀分布在各时段
                                       ///  0.5: 50%的需求集中在特定时段
                                       ///  1.0: 需求高度集中在少数时段

    // ================================================================================
    // 随机性控制
    // ================================================================================
    unsigned int random_seed = 42;  ///< 随机种子
                                   ///  相同种子产生相同的需求序列
                                   ///  便于实验的可重复性和对比测试

    // ================================================================================
    // 生成模式枚举
    // ================================================================================
    /**
     * @enum Mode
     * @brief 需求生成模式
     *
     * @details
     * 不同模式适用于不同的测试场景：
     * - ALL_COMBINATIONS: 适合密集需求场景
     * - SPARSE_RANDOM: 适合稀疏需求场景
     * - PER_ITEM_PER_TIME: 适合单点需求场景
     * - PER_NODE_PER_TIME: 适合节点集中需求场景
     * - CAPACITY_AWARE: 适合真实场景模拟（推荐）
     */
    enum class Mode {
        ALL_COMBINATIONS,   ///< 模式1：遍历所有(u,i,t)组合，按密度随机决定是否有需求
        SPARSE_RANDOM,      ///< 模式2：从所有组合中随机抽样指定比例的组合作为需求点
        PER_ITEM_PER_TIME,  ///< 模式3：每个(物品,时间)组合随机选择一个节点产生需求
        PER_NODE_PER_TIME,  ///< 模式4：每个(节点,时间)组合随机选择若干物品产生需求
        CAPACITY_AWARE      ///< 模式5：考虑产能约束的智能需求生成（推荐）
    } mode = Mode::ALL_COMBINATIONS;  ///< 默认模式
};

// ====================================================================================
// 需求生成器类
// ====================================================================================

/**
 * @class DemandGenerator
 * @brief 需求生成器静态类
 *
 * @details
 * 提供需求数据生成的静态方法，无需创建实例。
 * 根据配置参数和选定的模式，自动生成符合要求的需求数据。
 *
 * 所有方法都是静态的，直接通过类名调用。
 */
class DemandGenerator {
public:
    /**
     * @brief 生成需求数据
     *
     * @param config 需求生成配置对象
     * @return std::vector<DemandEntry> 生成的需求数据列表
     *
     * @details
     * 根据config中的mode参数，选择相应的生成算法：
     *
     * 模式1 - ALL_COMBINATIONS：
     * - 遍历所有可能的(u,i,t)组合
     * - 对每个组合，以density概率决定是否生成需求
     * - 适合：密集需求场景，density较大时
     * - 优点：分布均匀，可控性强
     * - 缺点：当U*I*T很大且density很小时，效率较低
     *
     * 模式2 - SPARSE_RANDOM：
     * - 先生成所有可能的(u,i,t)组合
     * - 随机打乱后选择前density*总数个作为需求点
     * - 适合：稀疏需求场景，density较小时
     * - 优点：精确控制需求数量，效率高
     * - 缺点：需要额外的内存存储所有组合
     *
     * 模式3 - PER_ITEM_PER_TIME：
     * - 对每个(物品,时间)组合，随机选择一个节点产生需求
     * - 以density概率决定是否生成
     * - 适合：物品导向的需求场景
     * - 特点：同一物品在同一时间最多只有一个节点有需求
     *
     * 模式4 - PER_NODE_PER_TIME：
     * - 对每个(节点,时间)组合，随机选择若干物品产生需求
     * - 物品数量由density控制
     * - 适合：节点导向的需求场景
     * - 特点：同一节点在同一时间可能有多个物品需求
     *
     * 模式5 - CAPACITY_AWARE（推荐）：
     * - 考虑时间维度的负载分布
     * - 支持需求集中度控制
     * - 支持产能紧张度控制
     * - 适合：真实场景模拟，最接近实际需求特征
     * - 优点：生成的需求更真实，更有挑战性
     * - 算法步骤：
     *   1. 为每个时段生成一个负载系数
     *   2. 归一化负载系数为概率分布
     *   3. 生成总需求数量 = U*I*T*density
     *   4. 对每个需求：
     *      a. 随机选择节点和物品
     *      b. 根据集中度和负载分布选择时间
     *      c. 根据时间的负载系数调整需求量
     *
     * @note 生成的需求列表可能包含重复的(u,i,t)，需要调用方处理
     * @note 使用相同的random_seed会产生相同的结果（可重复性）
     */
    static std::vector<DemandEntry> Generate(const DemandGenConfig& config) {
        std::vector<DemandEntry> demands;  // 存储生成的需求数据

        // 初始化随机数生成器
        std::mt19937 rng(config.random_seed);
        std::uniform_real_distribution<double> amount_dist(config.min_demand, config.max_demand);
        std::uniform_real_distribution<double> density_dist(0.0, 1.0);

        // 根据模式选择生成算法
        switch (config.mode) {
            // ========================================================================
            // 模式1：遍历所有组合
            // ========================================================================
            case DemandGenConfig::Mode::ALL_COMBINATIONS:
                // 三层循环遍历所有可能的(u,i,t)组合
                for (int u = 0; u < config.U; ++u) {
                    for (int i = 0; i < config.I; ++i) {
                        for (int t = 0; t < config.T; ++t) {
                            // 以density概率决定是否生成需求
                            if (density_dist(rng) < config.density) {
                                demands.push_back({u, i, t, amount_dist(rng)});
                            }
                        }
                    }
                }
                break;

            // ========================================================================
            // 模式2：稀疏随机
            // ========================================================================
            case DemandGenConfig::Mode::SPARSE_RANDOM: {
                // 计算总组合数和需要生成的需求数量
                int total_combinations = config.U * config.I * config.T;
                int num_demands = static_cast<int>(total_combinations * config.density);

                // 生成所有可能的组合
                std::vector<std::tuple<int, int, int>> all_combinations;
                for (int u = 0; u < config.U; ++u) {
                    for (int i = 0; i < config.I; ++i) {
                        for (int t = 0; t < config.T; ++t) {
                            all_combinations.push_back({u, i, t});
                        }
                    }
                }

                // 随机打乱组合顺序
                std::shuffle(all_combinations.begin(), all_combinations.end(), rng);

                // 选择前num_demands个作为需求点
                for (int idx = 0; idx < num_demands && idx < (int)all_combinations.size(); ++idx) {
                    auto [u, i, t] = all_combinations[idx];
                    demands.push_back({u, i, t, amount_dist(rng)});
                }
                break;
            }

            // ========================================================================
            // 模式3：每个物品每个时间
            // ========================================================================
            case DemandGenConfig::Mode::PER_ITEM_PER_TIME:
                // 遍历所有(物品,时间)组合
                for (int i = 0; i < config.I; ++i) {
                    for (int t = 0; t < config.T; ++t) {
                        // 以density概率决定是否生成需求
                        if (density_dist(rng) < config.density) {
                            // 随机选择一个节点
                            int u = rng() % config.U;
                            demands.push_back({u, i, t, amount_dist(rng)});
                        }
                    }
                }
                break;

            // ========================================================================
            // 模式4：每个节点每个时间
            // ========================================================================
            case DemandGenConfig::Mode::PER_NODE_PER_TIME:
                // 遍历所有(节点,时间)组合
                for (int u = 0; u < config.U; ++u) {
                    for (int t = 0; t < config.T; ++t) {
                        // 以density概率决定是否在此(u,t)生成需求
                        if (density_dist(rng) < config.density) {
                            // 计算要生成的物品数量
                            int num_items = std::max(1, static_cast<int>(config.I * config.density));

                            // 生成物品索引列表并随机打乱
                            std::vector<int> items(config.I);
                            for (int i = 0; i < config.I; ++i) items[i] = i;
                            std::shuffle(items.begin(), items.end(), rng);

                            // 为前num_items个物品生成需求
                            for (int idx = 0; idx < num_items; ++idx) {
                                demands.push_back({u, items[idx], t, amount_dist(rng)});
                            }
                        }
                    }
                }
                break;

            // ========================================================================
            // 模式5：产能感知（推荐）
            // ========================================================================
            case DemandGenConfig::Mode::CAPACITY_AWARE: {
                // 步骤1：为每个时段生成负载系数
                std::vector<double> period_load(config.T, 0.0);
                std::uniform_real_distribution<double> load_dist(0.5, 1.5);

                for (int t = 0; t < config.T; ++t) {
                    period_load[t] = load_dist(rng);
                }

                // 步骤2：归一化为概率分布
                double total_load = 0.0;
                for (double load : period_load) {
                    total_load += load;
                }
                for (int t = 0; t < config.T; ++t) {
                    period_load[t] /= total_load;
                }

                // 步骤3：计算总需求数量
                int total_demands = static_cast<int>(config.U * config.I * config.T * config.density);

                // 步骤4：确定需求集中的时段
                std::vector<int> concentrated_periods;
                int num_concentrated = std::max(1, static_cast<int>(config.T * config.demand_concentration));
                for (int i = 0; i < num_concentrated; ++i) {
                    concentrated_periods.push_back(rng() % config.T);
                }

                // 步骤5：生成每个需求
                for (int idx = 0; idx < total_demands; ++idx) {
                    // 随机选择节点和物品
                    int u = rng() % config.U;
                    int i = rng() % config.I;
                    int t;

                    // 根据集中度选择时间
                    if (density_dist(rng) < config.demand_concentration && !concentrated_periods.empty()) {
                        // 从集中时段中随机选择
                        t = concentrated_periods[rng() % concentrated_periods.size()];
                    } else {
                        // 根据负载分布选择
                        std::discrete_distribution<int> time_dist(period_load.begin(), period_load.end());
                        t = time_dist(rng);
                    }

                    // 根据时间负载和紧张度调整需求量
                    double base_amount = amount_dist(rng);
                    double tightness_factor = 1.0 + config.capacity_tightness * period_load[t];
                    double final_amount = base_amount * tightness_factor;

                    demands.push_back({u, i, t, final_amount});
                }
                break;
            }
        }

        return demands;
    }

    /**
     * @brief 获取模式名称（用于日志输出）
     *
     * @param mode 模式枚举值
     * @return std::string 模式名称字符串
     *
     * @details
     * 将枚举值转换为可读的字符串，便于日志记录和调试。
     *
     * 返回值：
     * - "ALL_COMBINATIONS"
     * - "SPARSE_RANDOM"
     * - "PER_ITEM_PER_TIME"
     * - "PER_NODE_PER_TIME"
     * - "CAPACITY_AWARE"
     * - "UNKNOWN" (未知模式)
     */
    static std::string GetModeName(DemandGenConfig::Mode mode) {
        switch (mode) {
            case DemandGenConfig::Mode::ALL_COMBINATIONS:
                return "ALL_COMBINATIONS";
            case DemandGenConfig::Mode::SPARSE_RANDOM:
                return "SPARSE_RANDOM";
            case DemandGenConfig::Mode::PER_ITEM_PER_TIME:
                return "PER_ITEM_PER_TIME";
            case DemandGenConfig::Mode::PER_NODE_PER_TIME:
                return "PER_NODE_PER_TIME";
            case DemandGenConfig::Mode::CAPACITY_AWARE:
                return "CAPACITY_AWARE";
            default:
                return "UNKNOWN";
        }
    }
};

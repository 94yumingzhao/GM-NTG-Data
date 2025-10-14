/**
 * ==================================================================================
 * @file        demand_generator.cpp
 * @brief       产能驱动的需求生成器实现 (Version 2.0)
 * @version     2.0.0
 * @date        2025-10-14
 *
 * @description
 * 本文件包含产能驱动需求生成器的具体实现，将算法逻辑从头文件中分离出来，
 * 使头文件更加清晰简洁。
 *
 * @author      LS-Game-DataGen Team
 * ==================================================================================
 */

#include "demand_generator.h"
#include <algorithm>
#include <stdexcept>

// ====================================================================================
// DemandGenerator 类实现
// ====================================================================================

/**
 * @brief 使用产能驱动方法生成需求
 */
std::vector<DemandEntry> DemandGenerator::Generate(const DemandGenConfig& config) {
    std::vector<DemandEntry> demands;

    // 步骤1：初始化随机数生成器
    std::mt19937 rng(config.random_seed);

    // 步骤2：计算需要生成的总需求点数
    int total_demand_points = static_cast<int>(
        config.U * config.I * config.T * config.demand_intensity
    );

    if (total_demand_points == 0) {
        return demands;  // 无需求要生成
    }

    // 步骤3：计算每个(节点, 时段)的可用产能
    std::map<std::pair<int,int>, double> available_capacity;
    CalculateAvailableCapacity(config, available_capacity);

    // 步骤4：生成时段权重（控制时间集中度）
    std::vector<double> period_weights;
    GeneratePeriodWeights(config, rng, period_weights);

    // 步骤5：生成节点权重（控制节点集中度）
    std::vector<double> node_weights;
    GenerateNodeWeights(config, rng, node_weights);

    // 步骤6：使用分配的产能生成需求点
    GenerateDemandPoints(config, rng, available_capacity,
                       period_weights, node_weights,
                       total_demand_points, demands);

    // 步骤7：验证可行性（健全性检查）
    VerifyFeasibility(config, demands, available_capacity);

    return demands;
}

//------------------------------------------------------------------------------------
// 产能计算
//------------------------------------------------------------------------------------

/**
 * @brief 计算每个(节点, 时段)的可用生产产能
 */
void DemandGenerator::CalculateAvailableCapacity(
    const DemandGenConfig& config,
    std::map<std::pair<int,int>, double>& available_capacity
) {
    // 估算每个时段的平均启动次数
    // 假设：每种物品类型每个时段可能启动一次
    // 实际启动次数取决于需求分布
    double avg_setups_per_period = config.I * config.demand_intensity;

    // 每个时段的启动开销
    double setup_overhead = avg_setups_per_period * config.unit_sY;

    // 计算每个(节点, 时段)的可用产能
    for (int u = 0; u < config.U; ++u) {
        for (int t = 0; t < config.T; ++t) {
            double total_cap = config.default_capacity;
            double available_cap = total_cap - setup_overhead;

            // 确保非负产能
            if (available_cap < 0) {
                available_cap = 0;
            }

            // 应用目标产能利用率
            available_cap *= config.capacity_utilization;

            available_capacity[{u, t}] = available_cap;
        }
    }
}

//------------------------------------------------------------------------------------
// 分布权重生成
//------------------------------------------------------------------------------------

/**
 * @brief 根据时间集中度生成时段权重
 */
void DemandGenerator::GeneratePeriodWeights(
    const DemandGenConfig& config,
    std::mt19937& rng,
    std::vector<double>& weights
) {
    weights.resize(config.T);

    if (config.time_concentration == 0.0) {
        // 均匀分布
        for (int t = 0; t < config.T; ++t) {
            weights[t] = 1.0 / config.T;
        }
    } else {
        // 生成带集中度的权重
        std::uniform_real_distribution<double> dist(0.5, 1.5);
        double total = 0.0;

        for (int t = 0; t < config.T; ++t) {
            double base_weight = dist(rng);

            // 应用集中度因子
            double concentration_factor = std::pow(base_weight,
                1.0 + config.time_concentration * 3.0);

            weights[t] = concentration_factor;
            total += concentration_factor;
        }

        // 归一化使总和为1.0
        for (int t = 0; t < config.T; ++t) {
            weights[t] /= total;
        }
    }
}

/**
 * @brief 根据节点集中度生成节点权重
 */
void DemandGenerator::GenerateNodeWeights(
    const DemandGenConfig& config,
    std::mt19937& rng,
    std::vector<double>& weights
) {
    weights.resize(config.U);

    if (config.node_concentration == 0.0) {
        // 均匀分布
        for (int u = 0; u < config.U; ++u) {
            weights[u] = 1.0 / config.U;
        }
    } else {
        // 生成带集中度的权重
        std::uniform_real_distribution<double> dist(0.5, 1.5);
        double total = 0.0;

        for (int u = 0; u < config.U; ++u) {
            double base_weight = dist(rng);

            // 应用集中度因子
            double concentration_factor = std::pow(base_weight,
                1.0 + config.node_concentration * 3.0);

            weights[u] = concentration_factor;
            total += concentration_factor;
        }

        // 归一化
        for (int u = 0; u < config.U; ++u) {
            weights[u] /= total;
        }
    }
}

//------------------------------------------------------------------------------------
// 需求点生成
//------------------------------------------------------------------------------------

/**
 * @brief 使用产能分配生成需求点
 */
void DemandGenerator::GenerateDemandPoints(
    const DemandGenConfig& config,
    std::mt19937& rng,
    std::map<std::pair<int,int>, double>& available_capacity,
    const std::vector<double>& period_weights,
    const std::vector<double>& node_weights,
    int total_demand_points,
    std::vector<DemandEntry>& demands
) {
    // 计算总可用产能
    double total_capacity = 0.0;
    for (const auto& entry : available_capacity) {
        total_capacity += entry.second;
    }

    if (total_capacity <= 0) {
        return;  // 无可用产能
    }

    // 计算期望的平均需求量
    double avg_demand_capacity = total_capacity / total_demand_points;
    double avg_demand_amount = avg_demand_capacity / config.unit_sX;

    // 根据方差计算需求量范围
    double min_demand = avg_demand_amount * (1.0 - config.demand_size_variance);
    double max_demand = avg_demand_amount * (1.0 + config.demand_size_variance);

    // 确保正数边界
    min_demand = std::max(1.0, min_demand);
    max_demand = std::max(min_demand + 1.0, max_demand);

    std::uniform_real_distribution<double> amount_dist(min_demand, max_demand);

    // 生成带集中度控制的物品权重
    std::vector<double> item_weights(config.I);
    if (config.item_concentration == 0.0) {
        // 均匀分布
        for (int i = 0; i < config.I; ++i) {
            item_weights[i] = 1.0 / config.I;
        }
    } else {
        // 集中分布
        std::uniform_real_distribution<double> dist(0.5, 1.5);
        double total = 0.0;
        for (int i = 0; i < config.I; ++i) {
            double base_weight = dist(rng);
            double weight = std::pow(base_weight,
                1.0 + config.item_concentration * 3.0);
            item_weights[i] = weight;
            total += weight;
        }
        for (int i = 0; i < config.I; ++i) {
            item_weights[i] /= total;
        }
    }

    // 离散分布用于选择
    std::discrete_distribution<int> time_dist(
        period_weights.begin(), period_weights.end());
    std::discrete_distribution<int> node_dist(
        node_weights.begin(), node_weights.end());
    std::discrete_distribution<int> item_dist_weighted(
        item_weights.begin(), item_weights.end());

    // 跟踪每个(u,t)的产能使用情况
    std::map<std::pair<int,int>, double> used_capacity;

    // 生成需求点
    for (int idx = 0; idx < total_demand_points; ++idx) {
        // 选择时间段
        int t = time_dist(rng);

        // 选择节点
        int u = node_dist(rng);

        // 选择物品
        int i = item_dist_weighted(rng);

        // 检查(u,t)处的可用产能
        auto key = std::make_pair(u, t);
        double avail_cap = available_capacity[key] - used_capacity[key];

        if (avail_cap <= 0) {
            // 此(u,t)处无剩余产能，尝试其他位置
            // 回退：找任何有可用产能的(u,t)
            bool found = false;
            for (const auto& entry : available_capacity) {
                if (entry.second - used_capacity[entry.first] > 0) {
                    u = entry.first.first;
                    t = entry.first.second;
                    key = entry.first;
                    avail_cap = entry.second - used_capacity[key];
                    found = true;
                    break;
                }
            }

            if (!found) {
                // 所有位置都无剩余产能，跳过此需求
                continue;
            }
        }

        // 在可用产能范围内生成需求量
        double max_possible_amount = avail_cap / config.unit_sX;
        double demand_amount = std::min(amount_dist(rng), max_possible_amount);

        // 确保最小需求量
        demand_amount = std::max(1.0, demand_amount);

        // 更新已使用产能
        used_capacity[key] += demand_amount * config.unit_sX;

        // 创建需求条目
        demands.push_back({u, i, t, demand_amount});
    }
}

//------------------------------------------------------------------------------------
// 可行性验证
//------------------------------------------------------------------------------------

/**
 * @brief 验证生成的需求是否可行
 */
void DemandGenerator::VerifyFeasibility(
    const DemandGenConfig& config,
    const std::vector<DemandEntry>& demands,
    const std::map<std::pair<int,int>, double>& available_capacity
) {
    // 计算每个(u,t)的实际产能使用量
    std::map<std::pair<int,int>, double> actual_usage;

    for (const auto& demand : demands) {
        auto key = std::make_pair(demand.u, demand.t);
        actual_usage[key] += demand.amount * config.unit_sX;
    }

    // 检查每个(u,t)
    for (const auto& entry : actual_usage) {
        auto key = entry.first;
        double usage = entry.second;
        double capacity = available_capacity.at(key);

        if (usage > capacity * 1.01) {  // 允许1%容差
            // 这永远不应该发生！
            throw std::runtime_error(
                "可行性检查失败，节点 " +
                std::to_string(key.first) + " 时段 " +
                std::to_string(key.second) + "：使用量=" +
                std::to_string(usage) + " > 产能=" +
                std::to_string(capacity)
            );
        }
    }
}

/**
 * ==================================================================================
 * @file        demand_generator.h
 * @brief       产能驱动的需求生成器 (Version 2.0)
 * @version     2.0.0
 * @date        2025-10-14
 *
 * @description
 * 本文件定义产能驱动需求生成策略的接口，确保生成的算例从设计上必然可行。
 *
 * 核心思想：
 * 1. 先计算每个(节点, 时段)的可用产能
 * 2. 在生成需求时确保不超出可用产能
 * 3. 通过集中度参数控制需求在各维度的分布
 *
 * 主要特性：
 * - 产能约束驱动生成，从设计保证可行性
 * - 按时段控制产能分配
 * - 支持集中度控制的分布生成
 *
 * @author      LS-Game-DataGen Team
 * ==================================================================================
 */

#pragma once

#include "case_generator.h"
#include <random>
#include <vector>
#include <map>

// ====================================================================================
// 配置结构体
// ====================================================================================

/**
 * @struct DemandGenConfig
 * @brief 产能驱动需求生成器的配置参数
 */
struct DemandGenConfig {
    //--------------------------------------------------------------------------------
    // 问题规模
    //--------------------------------------------------------------------------------
    int U;                           ///< 节点数量
    int I;                           ///< 物品种类数量
    int T;                           ///< 时间周期数量

    //--------------------------------------------------------------------------------
    // 产能参数
    //--------------------------------------------------------------------------------
    double default_capacity = 1440.0;  ///< 每节点每时段的默认产能
    double unit_sX = 1.0;              ///< 单位产品的产能占用
    double unit_sY = 10.0;             ///< 启动一次的产能占用

    //--------------------------------------------------------------------------------
    // 需求生成参数
    //--------------------------------------------------------------------------------
    double capacity_utilization = 0.85;  ///< 目标产能利用率 (0.0-1.0)
                                        ///< 0.85 表示使用85%的可用产能

    double demand_intensity = 0.15;     ///< 需求密度（(U,I,T)空间的占比）
                                        ///< 控制生成多少个需求点

    double initial_inventory_ratio = 0.0;  ///< 初始库存占平均需求的比例
                                          ///< 0.0 = 无初始库存
                                          ///< 0.5 = 初始库存 = 50%平均需求

    //--------------------------------------------------------------------------------
    // 分布控制参数
    //--------------------------------------------------------------------------------
    double time_concentration = 0.2;    ///< 时间分布集中度 (0.0-1.0)
                                       ///< 0.0 = 均匀分布，1.0 = 高度集中

    double node_concentration = 0.3;    ///< 节点分布集中度 (0.0-1.0)
                                       ///< 0.0 = 均匀分布，1.0 = 集中在少数节点

    double item_concentration = 0.3;    ///< 物品分布集中度 (0.0-1.0)
                                       ///< 0.0 = 均匀分布，1.0 = 集中在少数物品

    //--------------------------------------------------------------------------------
    // 随机性控制
    //--------------------------------------------------------------------------------
    unsigned int random_seed = 42;     ///< 随机种子，用于可重复性

    double demand_size_variance = 0.3;  ///< 需求量大小的方差 (0.0-1.0)
                                       ///< 控制需求量的离散程度
};

// ====================================================================================
// 产能驱动需求生成器
// ====================================================================================

/**
 * @class DemandGenerator
 * @brief 产能驱动的需求生成器，保证生成可行算例
 *
 * @details
 * 算法概述：
 * 1. 计算每个(节点, 时段)的可用产能
 * 2. 根据需求密度估算启动开销
 * 3. 计算生产产能 = 总产能 - 启动开销
 * 4. 将生产产能分配给需求点
 * 5. 生成与产能分配精确匹配的需求量
 *
 * 可行性保证：
 * 通过构造确保：sum(需求 * sX + 启动 * sY) <= C[u][t] 对所有u,t成立
 */
class DemandGenerator {
public:
    /**
     * @brief 使用产能驱动方法生成需求
     *
     * @param config 配置参数
     * @return vector<DemandEntry> 生成的需求列表（保证可行）
     *
     * @details
     * 步骤1：初始化随机数生成器
     * 步骤2：计算目标需求点数量
     * 步骤3：跨时段分配产能
     * 步骤4：使用分配的产能生成需求点
     * 步骤5：验证可行性（设计上应该总能通过）
     */
    static std::vector<DemandEntry> Generate(const DemandGenConfig& config);

private:
    //--------------------------------------------------------------------------------
    // 产能计算
    //--------------------------------------------------------------------------------

    /**
     * @brief 计算每个(节点, 时段)的可用生产产能
     *
     * @param config 配置参数
     * @param available_capacity 输出映射: (u,t) -> 可用产能
     *
     * @details
     * 可用产能 = 总产能 - 启动开销
     * 启动开销根据需求密度和物品数量估算
     */
    static void CalculateAvailableCapacity(
        const DemandGenConfig& config,
        std::map<std::pair<int,int>, double>& available_capacity
    );

    //--------------------------------------------------------------------------------
    // 分布权重生成
    //--------------------------------------------------------------------------------

    /**
     * @brief 根据时间集中度生成时段权重
     *
     * @param config 配置参数
     * @param rng 随机数生成器
     * @param weights 输出的时段权重向量（大小为T）
     *
     * @details
     * time_concentration = 0.0：均匀分布
     * time_concentration = 1.0：集中在少数时段
     */
    static void GeneratePeriodWeights(
        const DemandGenConfig& config,
        std::mt19937& rng,
        std::vector<double>& weights
    );

    /**
     * @brief 根据节点集中度生成节点权重
     *
     * @param config 配置参数
     * @param rng 随机数生成器
     * @param weights 输出的节点权重向量（大小为U）
     */
    static void GenerateNodeWeights(
        const DemandGenConfig& config,
        std::mt19937& rng,
        std::vector<double>& weights
    );

    //--------------------------------------------------------------------------------
    // 需求点生成
    //--------------------------------------------------------------------------------

    /**
     * @brief 使用产能分配生成需求点
     *
     * @param config 配置参数
     * @param rng 随机数生成器
     * @param available_capacity 可用产能映射
     * @param period_weights 时段权重
     * @param node_weights 节点权重
     * @param total_demand_points 需生成的总需求点数
     * @param demands 输出的需求列表
     *
     * @details
     * 算法步骤：
     * 1. 计算所有(u,t)的总可用产能
     * 2. 计算平均需求大小 = 总产能 / 需求数量
     * 3. 对于每个需求点：
     *    a. 按照产能比例选择(u,t)
     *    b. 随机选择物品i（考虑集中度）
     *    c. 从产能预算中生成需求量
     *    d. 更新剩余产能
     */
    static void GenerateDemandPoints(
        const DemandGenConfig& config,
        std::mt19937& rng,
        std::map<std::pair<int,int>, double>& available_capacity,
        const std::vector<double>& period_weights,
        const std::vector<double>& node_weights,
        int total_demand_points,
        std::vector<DemandEntry>& demands
    );

    //--------------------------------------------------------------------------------
    // 可行性验证
    //--------------------------------------------------------------------------------

    /**
     * @brief 验证生成的需求是否可行
     *
     * @param config 配置参数
     * @param demands 生成的需求列表
     * @param available_capacity 可用产能映射
     *
     * @details
     * 这是一个健全性检查。设计上需求应该总是可行的。
     * 如果此检查失败，说明生成逻辑存在bug。
     */
    static void VerifyFeasibility(
        const DemandGenConfig& config,
        const std::vector<DemandEntry>& demands,
        const std::map<std::pair<int,int>, double>& available_capacity
    );
};

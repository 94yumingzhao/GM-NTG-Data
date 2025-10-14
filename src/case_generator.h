/**
 * ==================================================================================
 * @file        case_generator.h
 * @brief       算例生成器 - 数据模型和生成接口定义
 * @version     1.0.0
 * @date        2025-10-13
 *
 * @description
 * 本文件定义了批量大小问题(Lot Sizing Problem)的数据结构和算例生成接口。
 * 包含所有必要的数据模型：需求、成本、产能、库存等。
 *
 * 核心数据结构：
 * - GeneratorConfig: 算例生成的完整配置信息
 * - DemandEntry:     需求数据条目
 * - CapacityOverride: 产能覆盖配置
 * - I0Override:      初始库存覆盖配置
 * - TransferEntry:   转运成本数据
 * - BigMEntry:       BigM约束数据
 *
 * @author      LS-Game-DataGen Team
 * @note        所有索引均为0-based（从0开始计数）
 * ==================================================================================
 */

#pragma once
#include "csv_writer.h"
#include <vector>
#include <string>
#include <stdexcept>

// ====================================================================================
// 数据结构定义
// ====================================================================================

/**
 * @struct DemandEntry
 * @brief  需求数据条目
 *
 * @details
 * 表示某个节点在某个时间对某个物品的需求量。
 * 需求是批量大小问题的核心输入，决定了生产计划的目标。
 *
 * @note 索引均为0-based
 */
struct DemandEntry {
    int u;            // 节点索引 (0-based)
    int i;            // 物品索引 (0-based)
    int t;            // 时间索引 (0-based)
    double amount;    // 需求量（非负实数）
};

/**
 * @struct CapacityOverride
 * @brief  产能覆盖配置
 *
 * @details
 * 用于覆盖特定节点在特定时间的默认产能。
 * 当某些节点在某些时段有特殊的产能限制时使用。
 *
 * @note 如果不配置覆盖，将使用 default_capacity
 */
struct CapacityOverride {
    int u;           // 节点索引 (0-based)
    int t;           // 时间索引 (0-based)
    double value;    // 产能值（非负实数）
};

/**
 * @struct I0Override
 * @brief  初始库存覆盖配置
 *
 * @details
 * 用于设置特定节点特定物品的初始库存。
 * 当某些物品在某些节点有初始库存时使用。
 *
 * @note 如果不配置覆盖，将使用 default_i0
 */
struct I0Override {
    int u;           // 节点索引 (0-based)
    int i;           // 物品索引 (0-based)
    double value;    // 初始库存值（非负实数）
};

/**
 * @struct TransferEntry
 * @brief  转运成本数据
 *
 * @details
 * 表示在某个时间从某个节点转运某个物品到另一个节点的成本。
 * 仅在启用转运功能时使用。
 *
 * @note 仅当 enable_transfer = true 时需要配置
 */
struct TransferEntry {
    int u;           // 源节点索引 (0-based)
    int v;           // 目标节点索引 (0-based)
    int i;           // 物品索引 (0-based)
    int t;           // 时间索引 (0-based)
    double cost;     // 转运成本（非负实数）
};

/**
 * @struct BigMEntry
 * @brief  BigM约束数据
 *
 * @details
 * 用于建模0-1整数规划中的BigM约束。
 * BigM是一个足够大的常数，用于激活或关闭某些约束。
 *
 * @note 仅当 enable_transfer = true 时需要配置
 */
struct BigMEntry {
    int i;           // 物品索引 (0-based)
    int t;           // 时间索引 (0-based)
    double M;        // BigM值（必须为正数且足够大）
};

/**
 * @struct GeneratorConfig
 * @brief  算例生成器的完整配置
 */
struct GeneratorConfig {
    // ================================================================================
    // 规模与功能开关
    // ================================================================================
    int U = 0;                     // 节点数量（必须为正整数）
    int I = 0;                     // 物品种类数量（必须为正整数）
    int T = 0;                     // 时间周期数量（必须为正整数）
    bool enable_transfer = false;  // 是否启用节点间转运功能

    // ================================================================================
    // 成本参数（向量长度必须等于 I）
    // ================================================================================
    std::vector<double> cX;        // X方向生产成本向量
                                   // cX[i] 表示物品i的X方向生产成本

    std::vector<double> cY;        // Y方向生产成本向量
                                   // cY[i] 表示物品i的Y方向生产成本（如启动成本）

    std::vector<double> cI;        // 库存持有成本向量
                                   // cI[i] 表示物品i的单位库存单位时间成本

    // ================================================================================
    // 产能占用参数（向量长度等于 I）
    // ================================================================================
    std::vector<double> sX;        // X方向产能占用向量
                                   // sX[i] 表示生产单位物品i消耗的X方向资源

    std::vector<double> sY;        // Y方向产能占用向量
                                   // sY[i] 表示生产单位物品i消耗的Y方向资源

    // ================================================================================
    // 产能与初始库存配置
    // ================================================================================
    double default_capacity = 0.0; // 默认产能
                                   // 所有节点所有时段的默认产能值

    double default_i0 = 0.0;       // 默认初始库存
                                   // 所有节点所有物品的默认初始库存值

    std::vector<CapacityOverride> capacity_overrides;  // 产能覆盖列表
                                                       // 用于设置特定(u,t)的产能

    std::vector<I0Override> i0_overrides;              // 初始库存覆盖列表
                                                       // 用于设置特定(u,i)的初始库存

    // ================================================================================
    // 需求数据（稀疏表示，未出现的默认为0）
    // ================================================================================
    std::vector<DemandEntry> demand;  // 需求数据列表
                                      // 只需添加非零需求点
                                      // 未添加的(u,i,t)组合默认需求为0

    // ================================================================================
    // 转运配置（仅当 enable_transfer=true 时需要）
    // ================================================================================
    std::vector<TransferEntry> transfer_costs;  // 转运成本列表
                                                // cT[u,v,i,t] 表示转运成本

    std::vector<BigMEntry> bigM;                // BigM约束列表
                                                // M[i,t] 表示BigM值
};

// ====================================================================================
// 算例生成器类
// ====================================================================================

/**
 * @class CaseGenerator
 * @brief 算例生成器静态类
 *
 * @details
 * 提供算例生成和验证的静态方法。
 * 主要功能：
 * 1. 验证配置的合法性
 * 2. 根据配置生成完整的CSV算例文件
 *
 * @note 所有方法都是静态的，不需要创建实例
 */
class CaseGenerator {
public:
    /**
     * @brief 验证配置的合法性
     *
     * @param gc 要验证的配置对象
     *
     * @throw std::runtime_error 当配置不合法时抛出异常
     *
     * @details
     * 验证内容包括：
     * - 规模参数是否为正
     * - 成本向量长度是否正确
     * - 产能占用向量长度是否正确
     * - 默认值是否非负
     * - 需求数据索引是否越界
     * - 产能覆盖索引是否越界
     * - 库存覆盖索引是否越界
     * - 转运配置是否合法（当启用转运时）
     * - BigM配置是否合法（当启用转运时）
     */
    static void Validate(const GeneratorConfig& gc);

    /**
     * @brief 生成CSV算例文件
     *
     * @param gc 算例生成配置
     * @param w  CSV写入器对象
     *
     * @throw std::runtime_error 当配置验证失败时抛出异常
     *
     * @details
     * 按照标准CSV schema顺序写出所有数据：
     * 1. meta      - 元数据（U, I, T, enable_transfer）
     * 2. cost      - 成本数据（cX, cY, cI）
     * 3. cap_usage - 产能占用（sX, sY）
     * 4. capacity  - 产能数据（默认值 + 覆盖）
     * 5. init      - 初始库存（默认值 + 覆盖）
     * 6. demand    - 需求数据（稀疏表示）
     * 7. transfer  - 转运数据（可选，仅当enable_transfer=true）
     * 8. bigM      - BigM约束（可选，仅当enable_transfer=true）
     *
     * @note 生成前会自动调用Validate()验证配置
     * @note 求解器参数由求解器项目自行配置，不在CSV中生成
     */
    static void GenerateCsv(const GeneratorConfig& gc, CsvWriter& w);
};

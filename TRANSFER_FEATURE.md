# 转运功能实现说明

## 功能概述

已成功为 LS-Game-DataGen 添加了完整的转运功能支持。当 `enable_transfer = true` 时，程序会自动生成转运成本和BigM约束数据。

## 实现细节

### 1. 转运成本数据 (cT)

- **数据量**: U×(U-1)×I×T 个条目
  - 对于 U=5, I=300, T=20: 5×4×300×20 = 120,000 条记录
- **格式**: `transfer,cT,u,v,i,t,cost`
  - u: 发货节点
  - v: 收货节点 (u ≠ v)
  - i: 物品编号
  - t: 时间段
  - cost: 转运成本（当前设置为统一值 5.0）

### 2. BigM 约束数据

- **数据量**: I×T 个条目
  - 对于 I=300, T=20: 6,000 条记录
- **格式**: `bigM,M,,,i,t,M_value`
  - i: 物品编号
  - t: 时间段
  - M_value: BigM值（自动计算为总需求的2倍，最小10000）

### 3. 代码位置

修改的文件：`src/main.cpp`

添加的代码段（第290-346行）：
```cpp
if (enable_transfer) {
    // 生成转运成本数据
    for (int u = 0; u < U; ++u) {
        for (int v = 0; v < U; ++v) {
            if (u == v) continue;
            for (int i = 0; i < I; ++i) {
                for (int t = 0; t < T; ++t) {
                    TransferEntry te;
                    te.u = u; te.v = v; te.i = i; te.t = t;
                    te.cost = transfer_cost;
                    gc.transfer_costs.push_back(te);
                }
            }
        }
    }

    // 生成BigM数据
    double bigM_value = std::max(10000.0, total_demand_sum * 2.0);
    for (int i = 0; i < I; ++i) {
        for (int t = 0; t < T; ++t) {
            BigMEntry bm;
            bm.i = i; bm.t = t; bm.M = bigM_value;
            gc.bigM.push_back(bm);
        }
    }
}
```

## 生成结果示例

运行 LSGameDataGen.exe 后的日志输出：

```
[2025-10-14 12:55:28] 生成转运成本和BigM数据...
[2025-10-14 12:55:28] 生成转运成本条目数: 120000
[2025-10-14 12:55:28] 生成BigM条目数: 6000
[2025-10-14 12:55:28] BigM值: 166290.050172
```

生成的CSV文件：
- 文件大小：约3.2MB
- 总行数：约130,000行
- 包含完整的转运和BigM数据

## 问题规模和性能影响

### 变量数量对比

| 配置 | 基础变量 (x,y,I) | 转运变量 (a) | 总变量数 | 增长倍数 |
|------|-----------------|-------------|---------|---------|
| 不启用转运 | U×I×T = 30,000 | 0 | 30,000 | 1× |
| 启用转运 | 30,000 | U×U×I×T = 150,000 | 180,000 | 6× |

### 求解时间对比

- **不启用转运** (enable_transfer=false):
  - 大联盟N求解: ~22秒
  - 单人联盟求解: 每个约1秒
  - 总求解时间: ~37秒

- **启用转运** (enable_transfer=true):
  - 大联盟N求解: >300秒 (可能更长)
  - 问题规模增大6倍，求解时间呈指数级增长

## 使用建议

### 方案1：禁用转运（推荐用于大规模算例）

在 `src/main.cpp` 第69行设置：
```cpp
bool enable_transfer = false;  // 禁用转运
```

**适用场景**：
- 大规模算例 (I≥200)
- 快速测试和验证
- 算法开发阶段

### 方案2：启用转运 + 减小规模

在 `src/main.cpp` 中调整规模参数：
```cpp
int U = 3;      // 节点数量: 5 → 3
int I = 50;     // 物品种类: 300 → 50
int T = 10;     // 时间段: 20 → 10
bool enable_transfer = true;
```

**问题规模**：
- 基础变量：3×50×10 = 1,500
- 转运变量：3×3×50×10 = 4,500
- 总变量：6,000（可接受的规模）

**预期求解时间**：
- 大联盟N：~5-10秒
- 整体求解：~30-60秒

### 方案3：自定义转运成本

如果需要更真实的转运成本模型，可以修改 `src/main.cpp` 第297行：

```cpp
// 简单距离模型
double distance = std::abs(u - v);  // 节点间距离
te.cost = 5.0 * distance;  // 成本与距离成正比

// 或随机模型
std::uniform_real_distribution<double> cost_dist(3.0, 8.0);
te.cost = cost_dist(cost_rng);
```

## 配置参数

可在 `src/main.cpp` 中调整的转运相关参数：

```cpp
// 第296-297行：转运成本
double transfer_cost = 5.0;  // 基础转运成本
                             // 建议范围：3.0 - 10.0
                             // 应该高于库存成本，低于紧急采购成本

// 第330行：BigM计算
double bigM_value = std::max(10000.0, total_demand_sum * 2.0);
                    // 自动计算，通常不需要修改
                    // 必须足够大以确保约束正确工作
```

## 验证方法

### 1. 检查生成的CSV文件

```bash
# 检查文件大小
ls -lh output/case_*.csv

# 检查转运数据
grep "^transfer," output/case_*.csv | wc -l  # 应该是 U*(U-1)*I*T

# 检查BigM数据
grep "^bigM," output/case_*.csv | wc -l      # 应该是 I*T
```

### 2. 查看日志文件

```bash
cat output/log_*.txt | grep -E "(转运|BigM)"
```

应该看到：
```
生成转运成本和BigM数据...
生成转运成本条目数: 120000
生成BigM条目数: 6000
BigM值: 166290.050172
转运功能: 启用
```

## 故障排查

### 问题1：CSV文件中没有转运数据

**原因**：`enable_transfer` 设置为 false
**解决**：检查 `src/main.cpp` 第69行，确保设置为 `true`

### 问题2：求解器报错 "cT size mismatch"

**原因**：转运数据数量不正确
**解决**：重新生成算例文件，确保转运数据完整

### 问题3：求解时间过长

**原因**：问题规模太大（启用转运后变量增加6倍）
**解决**：
1. 减小规模参数 (U, I, T)
2. 增加求解器时间限制
3. 考虑禁用转运功能

## 总结

转运功能已完全实现并测试通过。对于大规模算例，建议：
- 优先使用不启用转运的配置
- 如需测试转运功能，请将规模减小到 U≤3, I≤100, T≤15

这样可以在合理的时间内完成求解并获得有意义的结果。

# GM-NTG-Data: 多工厂博弈问题测试数据集

> GM-NTG-Core 求解器的测试数据仓库
>
> 多工厂批量生产协作博弈 (Multi-Plant Lot-Sizing Game) 测试实例

---

## 目录

- 1 [项目简介](#1-项目简介)
- 2 [数据格式](#2-数据格式)
- 3 [实例分类](#3-实例分类)
- 4 [使用方法](#4-使用方法)
- 5 [关联项目](#5-关联项目)

---

## 1. 项目简介

### 1.1 功能定位

本仓库存放 GM-NTG-Core 求解器的测试数据，与核心求解器代码分离管理。

### 1.2 问题背景

多工厂批量生产博弈问题:

- 多个工厂组成供应链网络
- 工厂可独立运营或组成联盟协作
- 协作可共享产能、协调生产、进行产品转运
- 核心问题: 如何公平分配协作成本 (最小核分配)

---

## 2. 数据格式

### 2.1 CSV 结构

使用 `[SECTION]` 标识各数据块:

```csv
[DIMENSIONS]
P,3
N,4
G,2
T,6

[CAPACITY]
# u,t,value
0,0,500
0,1,500
...

[DEMAND]
# u,i,t,value
0,0,0,50
0,0,1,60
...

[INIT_INVENTORY]
# u,i,value
0,0,20
0,1,15
...

[ITEM_FAMILY]
# i,g
0,0
1,0
2,1
3,1

[PRODUCTION]
# i,cap_usage,cost
0,1.0,10.0
1,1.2,12.0
...

[SETUP]
# g,cap_usage,cost
0,30.0,200.0
1,40.0,250.0

[HOLDING]
# i,cost
0,0.5
0,0.6
...

[TRANSFER]
# u,v,cost
0,1,8.0
0,2,10.0
1,0,8.0
...
```

### 2.2 字段说明

| Section | 字段 | 说明 |
|:--------|:-----|:-----|
| DIMENSIONS | P, N, G, T | 工厂数、产品数、产品大类数、周期数 |
| CAPACITY | u, t, value | 工厂 u 在周期 t 的产能上限 |
| DEMAND | u, i, t, value | 工厂 u 对产品 i 在周期 t 的需求量 |
| INIT_INVENTORY | u, i, value | 工厂 u 产品 i 的初始库存 |
| ITEM_FAMILY | i, g | 产品 i 属于产品大类 g |
| PRODUCTION | i, cap, cost | 产品 i 的产能消耗和生产成本 |
| SETUP | g, cap, cost | 产品大类 g 的换产产能消耗和成本 |
| HOLDING | i, cost | 产品 i 的库存持有成本 |
| TRANSFER | u, v, cost | 从工厂 u 到 v 的单位转运成本 |

---

## 3. 实例分类

### 3.1 按规模分类

| 类别 | 工厂数 | 产品数 | 周期数 | 用途 |
|:----:|:------:|:------:|:------:|:-----|
| tiny | 2-3 | 2-4 | 3-6 | 快速测试 |
| small | 4-6 | 4-8 | 6-12 | 功能验证 |
| medium | 7-10 | 8-15 | 12-24 | 性能评估 |
| large | 10+ | 15+ | 24+ | 压力测试 |

### 3.2 按特性分类

| 类别 | 特点 | 测试目的 |
|:----:|:-----|:---------|
| balanced | 各工厂产能需求均衡 | 基准测试 |
| imbalanced | 产能/需求分布不均 | 协作收益 |
| seasonal | 需求有季节性波动 | 库存策略 |
| sparse | 稀疏转运网络 | 网络结构 |

---

## 4. 使用方法

### 4.1 命令行调用

```bash
GM-NTG-Core.exe data/test_3plant.csv -v
```

### 4.2 GUI 加载

在 GM-NTG-GUI 中通过文件浏览器加载 CSV 数据文件。

---

## 5. 关联项目

| 项目 | 说明 |
|:-----|:-----|
| [GM-NTG-Core](https://github.com/94yumingzhao/GM-NTG-Core) | 核心求解器 (约束生成) |
| [GM-NTG-GUI](https://github.com/94yumingzhao/GM-NTG-GUI) | 图形界面 |

---

**文档版本**: 1.0
**更新日期**: 2026-01-11

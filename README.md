# LS-Game-DataGen

Lot Sizing Game Data Generator - 批量大小游戏数据生成器

## 项目简介

LS-Game-DataGen 是一个用C++编写的批量大小游戏数据生成器，用于生成优化问题的测试算例。该项目支持多种配置参数，包括节点数量、物品种类、时间周期等，可以生成符合特定格式的CSV数据文件。

## 功能特性

- 🎯 **灵活的规模配置**: 支持自定义节点数量(U)、物品种类数量(I)、时间周期数量(T)
- 💰 **成本参数设置**: 支持X方向生产成本、Y方向生产成本、库存持有成本配置
- 📊 **产能管理**: 支持产能占用参数和产能覆盖设置
- 🔄 **转运功能**: 可选的转运功能支持
- 📈 **需求配置**: 灵活的需求数据配置
- ⚡ **求解器集成**: 内置求解器参数配置
- 📁 **自动文件命名**: 生成带时间戳的输出文件

## 系统要求

- **编译器**: 支持C++20的编译器 (如GCC 10+, Clang 12+, MSVC 2019+)
- **构建系统**: CMake 3.24+
- **操作系统**: Windows, Linux, macOS

## 安装和构建

### 1. 克隆仓库
```bash
git clone https://github.com/yourusername/LS-Game-DataGen.git
cd LS-Game-DataGen
```

### 2. 构建项目

#### Windows (Visual Studio)
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release
```

#### Linux/macOS
```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### 3. 运行程序
```bash
# Windows
./bin/Release/LSGameDataGen.exe

# Linux/macOS
./bin/LSGameDataGen
```

## 使用方法

### 配置参数

在 `src/main.cpp` 文件中修改以下参数来配置算例生成：

```cpp
// 基本规模参数
int U = 2;    // 节点数量
int I = 2;    // 物品种类数量  
int T = 3;    // 时间周期数量

// 成本参数
double unit_cX = 2.0;    // X方向生产成本
double unit_cY = 5.0;    // Y方向生产成本
double unit_cI = 0.4;    // 库存持有成本

// 产能参数
double unit_sX = 1.0;    // X方向产能占用
double unit_sY = 0.0;    // Y方向产能占用
double default_capacity = 5.0;  // 默认产能
```

### 需求配置

在 `main.cpp` 中添加需求数据：

```cpp
gc.demand.push_back({0, 0, 0, 10.0});  // 节点0，物品0，时间0，需求量10.0
gc.demand.push_back({0, 1, 1, 15.0});  // 节点0，物品1，时间1，需求量15.0
gc.demand.push_back({1, 0, 2, 8.0});   // 节点1，物品0，时间2，需求量8.0
```

### 输出文件

程序会在 `output/` 目录下生成带时间戳的CSV文件，格式为：`case_YYYYMMDD_HHMMSS.csv`

## 项目结构

```
LS-Game-DataGen/
├── src/                    # 源代码目录
│   ├── main.cpp           # 主程序入口
│   ├── case_generator.h   # 算例生成器头文件
│   ├── case_generator.cpp # 算例生成器实现
│   ├── csv_writer.h       # CSV写入器头文件
│   └── csv_writer.cpp     # CSV写入器实现
├── build/                 # 构建目录
├── output/                # 输出文件目录
├── data/                  # 数据目录
├── CMakeLists.txt         # CMake配置文件
├── CMakePresets.json      # CMake预设配置
└── README.md             # 项目说明文档
```

## 数据格式

生成的CSV文件包含以下列：
- `section`: 数据类型段
- `key`: 数据键名
- `u`: 节点索引
- `v`: 目标节点索引（转运时使用）
- `i`: 物品索引
- `t`: 时间周期索引
- `value`: 数据值

## 开发

### 编译选项

项目支持多种构建类型：
- `Debug`: 调试版本，包含调试信息
- `Release`: 发布版本，优化性能
- `RelWithDebInfo`: 发布版本带调试信息
- `MinSizeRel`: 最小体积版本

### 测试

运行测试：
```bash
cd build
ctest
```

## 贡献

欢迎提交Issue和Pull Request来改进这个项目！

## 许可证

本项目采用 MIT 许可证 - 查看 [LICENSE](LICENSE) 文件了解详情。

## 联系方式

如有问题或建议，请通过以下方式联系：
- 提交 [GitHub Issue](https://github.com/yourusername/LS-Game-DataGen/issues)
- 邮箱: your.email@example.com

---

⭐ 如果这个项目对你有帮助，请给它一个星标！

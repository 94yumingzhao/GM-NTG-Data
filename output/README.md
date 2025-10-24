# LS-Game-NTG-Data 输出目录结构说明

## 目录结构

```
output/
├── cases/      # CSV算例文件
├── logs/       # 生成日志文件
└── archives/   # 旧文件归档（可选）
```

## 文件说明

### cases/ 目录
存放数据生成器生成的CSV算例文件。

**文件命名格式**：`case_YYYYMMDD_HHMMSS.csv`
- YYYYMMDD：生成日期
- HHMMSS：生成时间

**示例**：
- `case_20251024_074953.csv`

### logs/ 目录
存放数据生成器的运行日志。

**文件命名格式**：`log_YYYYMMDD_HHMMSS.txt`
- YYYYMMDD：生成日期
- HHMMSS：生成时间

**示例**：
- `log_20251024_074953.txt`

### archives/ 目录
用于归档不再使用的旧文件，保持cases/和logs/目录整洁。

## 使用说明

### 数据生成器 (LS-Game-NTG-Data)
运行数据生成器会自动创建所需的子目录，并将文件输出到对应位置：
```bash
cd D:\LS-Game-NTG-Data
.\build\release\bin\Release\LSGameDataGen.exe
```

输出：
- CSV算例 → `output/cases/case_YYYYMMDD_HHMMSS.csv`
- 日志文件 → `output/logs/log_YYYYMMDD_HHMMSS.txt`

### 主程序 (LS-Game-NTG)
主程序会从 `D:\LS-Game-NTG-Data\output\cases\` 读取CSV算例：
```bash
cd D:\LS-Game-NTG
.\build\release\bin\Release\LSGame.exe case_20251024_074953.csv
```

主程序输出（结果报告）保存在：
- `D:\LS-Game-NTG\outputs\reports\report_YYYYMMDD_HHMMSS.json`

## 维护建议

1. **定期清理**：定期将旧的CSV和log文件移动到 `archives/` 目录
2. **保留有效算例**：将有用的算例保留在 `cases/` 目录中
3. **文件命名**：不要修改文件的时间戳命名，便于追溯

## 版本历史

**2025-10-24**：
- 重新组织目录结构
- 分离CSV算例和日志文件
- 更新数据生成器和主程序的路径配置

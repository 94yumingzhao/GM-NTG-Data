/**
 * ==================================================================================
 * @file        logger.h
 * @brief       日志记录器 - 头文件和实现
 * @version     1.0.0
 * @date        2025-10-13
 *
 * @description
 * Logger 是一个简单但功能完整的日志记录工具类。
 *
 * 主要功能：
 * - 实时输出日志到控制台（带时间戳）
 * - 缓存所有日志到内存
 * - 支持将日志保存到文件
 * - 自动生成带时间戳的日志文件名
 * - 线程安全（使用互斥锁保护）
 *
 * 日志格式：
 *   [YYYY-MM-DD HH:MM:SS] 日志消息内容
 *
 * 文件名格式：
 *   output/log_YYYYMMDD_HHMMSS.txt
 *
 * 使用示例：
 * @code
 * Logger logger;
 * logger.log("程序启动");
 * logger.log("配置: U=5, I=10, T=20");
 * logger.log("生成完成");
 * logger.saveToFile();  // 保存到文件
 * @endcode
 *
 * 线程安全性：
 * - 所有公共方法都使用互斥锁保护
 * - 可以在多线程环境中安全使用
 *
 * @note 本类采用header-only设计，所有实现都在头文件中
 * @note 日志文件名在构造时生成，确保每次运行有唯一的日志文件
 *
 * @author      LS-Game-DataGen Team
 * ==================================================================================
 */

#pragma once
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <mutex>

/**
 * @class Logger
 * @brief 日志记录器类
 *
 * @details
 * 提供简单易用的日志记录功能：
 * - 实时控制台输出（方便查看进度）
 * - 内存缓存（便于最后一次性保存）
 * - 文件持久化（便于事后分析）
 * - 线程安全（支持多线程场景）
 *
 * 设计特点：
 * - Header-only：所有实现都在头文件中，无需单独的.cpp文件
 * - RAII风格：构造时初始化，析构时自动清理
 * - 懒保存：只在显式调用saveToFile()时写入文件
 */
class Logger {
private:
    std::ostringstream buffer;  ///< 日志缓冲区，存储所有日志消息
    std::string log_filename;   ///< 日志文件名（在构造时生成）
    std::mutex mutex;           ///< 互斥锁，保护buffer和log_filename的并发访问

    /**
     * @brief 获取当前时间戳字符串（用于日志消息）
     *
     * @return std::string 格式化的时间戳 "[YYYY-MM-DD HH:MM:SS]"
     *
     * @details
     * 时间格式：
     * - 年：4位数字
     * - 月日：2位数字（不足补0）
     * - 时分秒：2位数字（不足补0）
     * - 使用本地时区
     *
     * 示例输出：[2025-10-13 17:30:45]
     *
     * @note 使用 localtime_s（Windows）而非 localtime（线程安全）
     */
    static std::string getCurrentTimestamp() {
        // 获取当前系统时间
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);

        // 转换为本地时间（线程安全版本）
        std::tm tm_now;
        localtime_s(&tm_now, &time_t_now);

        // 格式化时间戳
        std::ostringstream oss;
        oss << "["
            << std::setfill('0')
            << std::setw(4) << (tm_now.tm_year + 1900) << "-"  // 年份（加1900）
            << std::setw(2) << (tm_now.tm_mon + 1) << "-"      // 月份（加1）
            << std::setw(2) << tm_now.tm_mday << " "           // 日期
            << std::setw(2) << tm_now.tm_hour << ":"           // 小时
            << std::setw(2) << tm_now.tm_min << ":"            // 分钟
            << std::setw(2) << tm_now.tm_sec                   // 秒
            << "]";
        return oss.str();
    }

    /**
     * @brief 生成日志文件名（用于文件保存）
     *
     * @param output_dir 输出目录路径
     * @return std::string 日志文件名 "output_dir/log_YYYYMMDD_HHMMSS.txt"
     *
     * @details
     * 文件名格式：
     * - 目录：指定的output_dir
     * - 前缀：log_
     * - 日期：YYYYMMDD（8位数字）
     * - 时间：HHMMSS（6位数字）
     * - 后缀：.txt
     *
     * 示例输出：D:/LS-Game-DataGen/output/log_20251013_173045.txt
     *
     * @note 在构造函数中调用，确保每个Logger实例有唯一的文件名
     */
    static std::string generateLogFilename(const std::string& output_dir) {
        // 获取当前系统时间
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);

        // 转换为本地时间（线程安全版本）
        std::tm tm_now;
        localtime_s(&tm_now, &time_t_now);

        // 构建文件名
        std::ostringstream oss;
        oss << output_dir << "/log_"
            << std::setfill('0')
            << std::setw(4) << (tm_now.tm_year + 1900)  // 年份
            << std::setw(2) << (tm_now.tm_mon + 1)      // 月份
            << std::setw(2) << tm_now.tm_mday           // 日期
            << "_"
            << std::setw(2) << tm_now.tm_hour           // 小时
            << std::setw(2) << tm_now.tm_min            // 分钟
            << std::setw(2) << tm_now.tm_sec            // 秒
            << ".txt";
        return oss.str();
    }

public:
    /**
     * @brief 构造函数 - 初始化日志器并生成日志文件名
     *
     * @details
     * 在构造时生成唯一的日志文件名，但不立即创建文件。
     * 文件会在调用 saveToFile() 时创建和写入。
     */
    Logger() {
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
        
        log_filename = generateLogFilename(output_dir);
    }

    /**
     * @brief 记录一条日志消息
     *
     * @param message 日志消息内容
     *
     * @details
     * 执行步骤：
     * 1. 加锁（确保线程安全）
     * 2. 获取当前时间戳
     * 3. 格式化日志行：[时间戳] 消息
     * 4. 输出到控制台（实时显示）
     * 5. 追加到内存缓冲区（用于后续保存）
     * 6. 解锁
     *
     * 日志格式：
     *   [YYYY-MM-DD HH:MM:SS] 消息内容
     *
     * 线程安全：使用互斥锁保护，多线程调用安全
     *
     * @note 控制台输出后会立即flush，确保实时显示
     */
    void log(const std::string& message) {
        // 加锁保护共享资源
        std::lock_guard<std::mutex> lock(mutex);

        // 获取时间戳
        std::string timestamp = getCurrentTimestamp();

        // 构建完整的日志行
        std::string line = timestamp + " " + message;

        // 输出到控制台（实时显示）
        std::cout << line << std::endl;

        // 追加到缓冲区（用于后续保存）
        buffer << line << "\n";
    }

    /**
     * @brief 将所有日志保存到文件
     *
     * @details
     * 执行步骤：
     * 1. 加锁（确保线程安全）
     * 2. 打开日志文件（覆盖模式）
     * 3. 写入缓冲区中的所有日志
     * 4. 关闭文件
     * 5. 在控制台和缓冲区中记录保存成功信息
     * 6. 如果失败，输出错误信息到stderr
     * 7. 解锁
     *
     * 文件位置：由构造函数生成的 log_filename
     * 文件编码：默认（通常为系统默认编码）
     *
     * 线程安全：使用互斥锁保护，多线程调用安全
     *
     * @note 如果文件打开失败，会输出错误到stderr但不会抛出异常
     * @note 调用后会在日志中追加一条"日志已保存"的消息
     */
    void saveToFile() {
        // 加锁保护共享资源
        std::lock_guard<std::mutex> lock(mutex);

        // 尝试打开文件
        std::ofstream file(log_filename);

        if (file.is_open()) {
            // 文件打开成功，写入所有日志
            file << buffer.str();
            file.close();

            // 记录保存成功的消息
            std::string timestamp = getCurrentTimestamp();
            std::string msg = timestamp + " 日志已保存到: " + log_filename;

            // 输出到控制台
            std::cout << msg << std::endl;

            // 追加到缓冲区（记录这个操作本身）
            buffer << msg << "\n";
        } else {
            // 文件打开失败，输出错误
            std::cerr << getCurrentTimestamp()
                      << " [错误] 无法保存日志文件: "
                      << log_filename << std::endl;
        }
    }

    /**
     * @brief 获取日志文件名
     *
     * @return std::string 日志文件名（包含路径）
     *
     * @details
     * 返回在构造时生成的日志文件名。
     * 即使尚未调用 saveToFile()，也会返回预定的文件名。
     *
     * 线程安全：只读操作，理论上安全但未加锁
     *
     * @note 如果在多线程环境中使用，建议在构造后立即调用并保存结果
     */
    std::string getLogFilename() const {
        return log_filename;
    }
};

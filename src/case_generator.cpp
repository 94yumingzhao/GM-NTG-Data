#include "case_generator.h"
#include <unordered_set>
#include <sstream>

/** 工具：范围检查与统一报错 **/
static void CHECK(bool cond, const std::string& msg) {
    if (!cond) throw std::runtime_error("配置不合法: " + msg);
}
static std::string triple(int a, int b, int c) {
    return "(" + std::to_string(a) + "," + std::to_string(b) + "," + std::to_string(c) + ")";
}
static std::string quad(int a, int b, int c, int d) {
    return "(" + std::to_string(a) + "," + std::to_string(b) + "," + std::to_string(c) + "," + std::to_string(d) + ")";
}

/** 校验所有维度、索引与长度 **/
void CaseGenerator::Validate(const GeneratorConfig& g) {
    CHECK(g.U > 0 && g.I > 0 && g.T > 0, "U/I/T 必须为正整数");
    CHECK((int)g.cX.size() == g.I, "cX 长度必须等于 I");
    CHECK((int)g.cY.size() == g.I, "cY 长度必须等于 I");
    CHECK((int)g.cI.size() == g.I, "cI 长度必须等于 I");
    CHECK((int)g.sX.size() == g.I, "sX 长度必须等于 I");
    CHECK((int)g.sY.size() == g.I, "sY 长度必须等于 I");
    CHECK(g.default_capacity >= 0.0, "default_capacity 需为非负");
    CHECK(g.default_i0 >= 0.0, "default_i0 需为非负");
    CHECK(g.mip_gap >= 0.0, "mip_gap 非负");
    CHECK(g.time_limit_sec > 0, "time_limit_sec 必须 > 0");
    CHECK(g.max_iters > 0, "max_iters 必须 > 0");

    for (const auto& d : g.demand) {
        CHECK(0 <= d.u && d.u < g.U, "Demand.u 越界: u=" + std::to_string(d.u));
        CHECK(0 <= d.i && d.i < g.I, "Demand.i 越界: i=" + std::to_string(d.i));
        CHECK(0 <= d.t && d.t < g.T, "Demand.t 越界: t=" + std::to_string(d.t));
        CHECK(d.amount >= 0.0, "Demand.amount 需为非负, at " + triple(d.u,d.i,d.t));
    }
    for (const auto& c : g.capacity_overrides) {
        CHECK(0 <= c.u && c.u < g.U, "Capacity.u 越界");
        CHECK(0 <= c.t && c.t < g.T, "Capacity.t 越界");
        CHECK(c.value >= 0.0, "Capacity.value 需为非负");
    }
    for (const auto& z : g.i0_overrides) {
        CHECK(0 <= z.u && z.u < g.U, "I0.u 越界");
        CHECK(0 <= z.i && z.i < g.I, "I0.i 越界");
        CHECK(z.value >= 0.0, "I0.value 需为非负");
    }

    if (g.enable_transfer) {
        for (const auto& e : g.transfer_costs) {
            CHECK(0 <= e.u && e.u < g.U, "cT.u 越界");
            CHECK(0 <= e.v && e.v < g.U, "cT.v 越界");
            CHECK(0 <= e.i && e.i < g.I, "cT.i 越界");
            CHECK(0 <= e.t && e.t < g.T, "cT.t 越界");
            CHECK(e.cost >= 0.0, "cT.cost 需为非负, at " + quad(e.u,e.v,e.i,e.t));
        }
        for (const auto& m : g.bigM) {
            CHECK(0 <= m.i && m.i < g.I, "M.i 越界");
            CHECK(0 <= m.t && m.t < g.T, "M.t 越界");
            CHECK(m.M > 0.0, "M 值需为正");
        }
    } else {
        CHECK(g.transfer_costs.empty(), "enable_transfer=0 时不应提供 transfer_costs");
        CHECK(g.bigM.empty(),           "enable_transfer=0 时不应提供 bigM");
    }
}

/** 实际生成 CSV（严格按 schema 顺序写出） **/
void CaseGenerator::GenerateCsv(const GeneratorConfig& g, CsvWriter& w) {
    Validate(g);

    // 1) meta
    w.writeRow("meta", "U", -1, -1, -1, -1, g.U);
    w.writeRow("meta", "I", -1, -1, -1, -1, g.I);
    w.writeRow("meta", "T", -1, -1, -1, -1, g.T);
    w.writeRow("meta", "enable_transfer", -1, -1, -1, -1, g.enable_transfer ? 1 : 0);

    // 2) cost
    for (int i = 0; i < g.I; ++i) w.writeRow("cost", "cX", -1, -1, i, -1, g.cX[i]);
    for (int i = 0; i < g.I; ++i) w.writeRow("cost", "cY", -1, -1, i, -1, g.cY[i]);
    for (int i = 0; i < g.I; ++i) w.writeRow("cost", "cI", -1, -1, i, -1, g.cI[i]);

    // 3) cap_usage
    for (int i = 0; i < g.I; ++i) w.writeRow("cap_usage", "sX", -1, -1, i, -1, g.sX[i]);
    for (int i = 0; i < g.I; ++i) w.writeRow("cap_usage", "sY", -1, -1, i, -1, g.sY[i]);

    // 4) capacity（先写默认值覆盖全表，再写覆盖项）
    for (int u = 0; u < g.U; ++u)
        for (int t = 0; t < g.T; ++t)
            w.writeRow("capacity", "C", u, -1, -1, t, g.default_capacity);
    for (const auto& c : g.capacity_overrides)
        w.writeRow("capacity", "C", c.u, -1, -1, c.t, c.value);

    // 5) init I0（同理：默认 + 覆盖）
    for (int u = 0; u < g.U; ++u)
        for (int i = 0; i < g.I; ++i)
            w.writeRow("init", "I0", u, -1, i, -1, g.default_i0);
    for (const auto& z : g.i0_overrides)
        w.writeRow("init", "I0", z.u, -1, z.i, -1, z.value);

    // 6) demand（仅写非零/显式给定项；未出现默认为 0）
    for (const auto& d : g.demand)
        w.writeRow("demand", "Demand", d.u, -1, d.i, d.t, d.amount);

    // 7) transfer（可选）
    if (g.enable_transfer) {
        for (const auto& e : g.transfer_costs)
            w.writeRow("transfer", "cT", e.u, e.v, e.i, e.t, e.cost);
        for (const auto& m : g.bigM)
            w.writeRow("bigM", "M", -1, -1, m.i, m.t, m.M);
    }

    // 8) solver
    w.writeRow("solver", "mip_gap",           -1, -1, -1, -1, g.mip_gap);
    w.writeRow("solver", "time_limit_sec",    -1, -1, -1, -1, g.time_limit_sec);
    w.writeRow("solver", "threads",           -1, -1, -1, -1, g.threads);
    w.writeRow("solver", "sep_violation_eps", -1, -1, -1, -1, g.sep_violation_eps);
    w.writeRow("solver", "max_iters",         -1, -1, -1, -1, g.max_iters);
}

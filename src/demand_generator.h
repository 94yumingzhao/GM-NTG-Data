#pragma once
#include "case_generator.h"
#include <random>
#include <vector>
#include <algorithm>

struct DemandGenConfig {
    int U;
    int I;
    int T;

    double min_demand = 1.0;
    double max_demand = 100.0;

    double density = 1.0;

    unsigned int random_seed = 42;

    enum class Mode {
        ALL_COMBINATIONS,
        SPARSE_RANDOM,
        PER_ITEM_PER_TIME,
        PER_NODE_PER_TIME
    } mode = Mode::ALL_COMBINATIONS;
};

class DemandGenerator {
public:
    static std::vector<DemandEntry> Generate(const DemandGenConfig& config) {
        std::vector<DemandEntry> demands;
        std::mt19937 rng(config.random_seed);
        std::uniform_real_distribution<double> amount_dist(config.min_demand, config.max_demand);
        std::uniform_real_distribution<double> density_dist(0.0, 1.0);

        switch (config.mode) {
            case DemandGenConfig::Mode::ALL_COMBINATIONS:
                for (int u = 0; u < config.U; ++u) {
                    for (int i = 0; i < config.I; ++i) {
                        for (int t = 0; t < config.T; ++t) {
                            if (density_dist(rng) < config.density) {
                                demands.push_back({u, i, t, amount_dist(rng)});
                            }
                        }
                    }
                }
                break;

            case DemandGenConfig::Mode::SPARSE_RANDOM: {
                int total_combinations = config.U * config.I * config.T;
                int num_demands = static_cast<int>(total_combinations * config.density);

                std::vector<std::tuple<int, int, int>> all_combinations;
                for (int u = 0; u < config.U; ++u) {
                    for (int i = 0; i < config.I; ++i) {
                        for (int t = 0; t < config.T; ++t) {
                            all_combinations.push_back({u, i, t});
                        }
                    }
                }

                std::shuffle(all_combinations.begin(), all_combinations.end(), rng);

                for (int idx = 0; idx < num_demands && idx < (int)all_combinations.size(); ++idx) {
                    auto [u, i, t] = all_combinations[idx];
                    demands.push_back({u, i, t, amount_dist(rng)});
                }
                break;
            }

            case DemandGenConfig::Mode::PER_ITEM_PER_TIME:
                for (int i = 0; i < config.I; ++i) {
                    for (int t = 0; t < config.T; ++t) {
                        if (density_dist(rng) < config.density) {
                            int u = rng() % config.U;
                            demands.push_back({u, i, t, amount_dist(rng)});
                        }
                    }
                }
                break;

            case DemandGenConfig::Mode::PER_NODE_PER_TIME:
                for (int u = 0; u < config.U; ++u) {
                    for (int t = 0; t < config.T; ++t) {
                        if (density_dist(rng) < config.density) {
                            int num_items = std::max(1, static_cast<int>(config.I * config.density));
                            std::vector<int> items(config.I);
                            for (int i = 0; i < config.I; ++i) items[i] = i;
                            std::shuffle(items.begin(), items.end(), rng);

                            for (int idx = 0; idx < num_items; ++idx) {
                                demands.push_back({u, items[idx], t, amount_dist(rng)});
                            }
                        }
                    }
                }
                break;
        }

        return demands;
    }

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
            default:
                return "UNKNOWN";
        }
    }
};

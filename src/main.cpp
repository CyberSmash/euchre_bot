#include <iostream>
#include <chrono>
#include <array>
#include <cstdint>
#include "Env.hpp"
#include "bots/RandomBot.hpp"
#include "bots/HeuristicBot.hpp"
#include "bots/MinBot.hpp"
#include "bots/MaxBot.hpp"

struct BenchResult {
    int team0_wins;
    int team1_wins;
    int stalled;
    double seconds;
};

BenchResult run_benchmark(std::array<IBot*, 4> players, int num_games, int max_steps) {
    int team0_wins = 0;
    int team1_wins = 0;
    int stalled_games = 0;

    auto start = std::chrono::high_resolution_clock::now();

    for (int g = 0; g < num_games; g++) {
        Env env{static_cast<unsigned int>(g), players};
        int steps = 0;

        while (env.state.status != GameState::GameStatus::GameOver && steps < max_steps) {
            env.step_game();
            steps++;
        }

        if (env.state.status != GameState::GameStatus::GameOver) {
            stalled_games++;
            continue;
        }

        if (env.state.scores[0] >= 10) {
            team0_wins++;
        } else {
            team1_wins++;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    return {team0_wins, team1_wins, stalled_games, elapsed};
}

void print_result(const char* label, BenchResult r, int num_games) {
    int completed = r.team0_wins + r.team1_wins;
    std::cout << "--- " << label << " ---" << '\n';
    std::cout << "Games:      " << num_games << '\n';
    std::cout << "Completed:  " << completed << '\n';
    if (r.stalled > 0) {
        std::cout << "Stalled:    " << r.stalled << '\n';
    }
    std::cout << "Team 0 wins: " << r.team0_wins
              << " (" << (100.0 * r.team0_wins / completed) << "%)" << '\n';
    std::cout << "Team 1 wins: " << r.team1_wins
              << " (" << (100.0 * r.team1_wins / completed) << "%)" << '\n';
    std::cout << "Time:       " << r.seconds << "s" << '\n';
    std::cout << "Games/sec:  " << static_cast<int>(completed / r.seconds) << '\n';
    std::cout << '\n';
}

int main() {

    constexpr int num_games = 100000;
    constexpr int max_steps = 10000;

    std::cout << "=== Euchre Benchmark ===" << '\n' << '\n';

    // Random vs Random
    {
        RandomBot a{"A"}, b{"B"}, c{"C"}, d{"D"};
        std::array<IBot*, 4> players = {&a, &b, &c, &d};
        auto r = run_benchmark(players, num_games, max_steps);
        print_result("Random vs Random", r, num_games);
    }

    // Heuristic vs Random (Heuristic = team 0, Random = team 1)
    {
        HeuristicBot a{"H0"}, c{"H1"};
        RandomBot b{"R0"}, d{"R1"};
        std::array<IBot*, 4> players = {&a, &b, &c, &d};
        auto r = run_benchmark(players, num_games, max_steps);
        print_result("Heuristic(T0) vs Random(T1)", r, num_games);
    }

    // Heuristic vs Heuristic
    {
        HeuristicBot a{"H0"}, b{"H1"}, c{"H2"}, d{"H3"};
        std::array<IBot*, 4> players = {&a, &b, &c, &d};
        auto r = run_benchmark(players, num_games, max_steps);
        print_result("Heuristic vs Heuristic", r, num_games);
    }

    // MaxBot vs Random
    {
        MaxBot a{"Max0"}, c{"Max1"};
        RandomBot b{"R0"}, d{"R1"};
        std::array<IBot*, 4> players = {&a, &b, &c, &d};
        auto r = run_benchmark(players, num_games, max_steps);
        print_result("MaxBot(T0) vs Random(T1)", r, num_games);
    }

    // MinBot vs Random
    {
        MinBot a{"Min0"}, c{"Min1"};
        RandomBot b{"R0"}, d{"R1"};
        std::array<IBot*, 4> players = {&a, &b, &c, &d};
        auto r = run_benchmark(players, num_games, max_steps);
        print_result("MinBot(T0) vs Random(T1)", r, num_games);
    }

    // MaxBot vs Heuristic
    {
        MaxBot a{"Max0"}, c{"Max1"};
        HeuristicBot b{"H0"}, d{"H1"};
        std::array<IBot*, 4> players = {&a, &b, &c, &d};
        auto r = run_benchmark(players, num_games, max_steps);
        print_result("MaxBot(T0) vs Heuristic(T1)", r, num_games);
    }

    // --- Weak link benchmarks: effect of one Random teammate ---

    std::cout << "=== Weak Link: One Random Teammate ===" << '\n' << '\n';

    // Baseline: Heuristic vs Heuristic (already above, repeated for grouping)
    {
        HeuristicBot a{"H0"}, b{"H1"}, c{"H2"}, d{"H3"};
        std::array<IBot*, 4> players = {&a, &b, &c, &d};
        auto r = run_benchmark(players, num_games, max_steps);
        print_result("Baseline: HH vs HH", r, num_games);
    }

    // Team 0 has one Random partner: H+R vs H+H
    {
        HeuristicBot a{"H0"};
        RandomBot c{"R0"};       // Team 0's weak link
        HeuristicBot b{"H1"}, d{"H2"};
        std::array<IBot*, 4> players = {&a, &b, &c, &d};
        auto r = run_benchmark(players, num_games, max_steps);
        print_result("H+Random(T0) vs HH(T1)", r, num_games);
    }

    // Both teams have one Random partner: H+R vs H+R
    {
        HeuristicBot a{"H0"}, b{"H1"};
        RandomBot c{"R0"}, d{"R1"};  // Both teams' weak links
        std::array<IBot*, 4> players = {&a, &b, &c, &d};
        auto r = run_benchmark(players, num_games, max_steps);
        print_result("H+Random(T0) vs H+Random(T1)", r, num_games);
    }

    // Team 0 has one MaxBot partner instead: H+Max vs H+H
    {
        HeuristicBot a{"H0"};
        MaxBot c{"Max0"};        // Team 0's weaker partner
        HeuristicBot b{"H1"}, d{"H2"};
        std::array<IBot*, 4> players = {&a, &b, &c, &d};
        auto r = run_benchmark(players, num_games, max_steps);
        print_result("H+MaxBot(T0) vs HH(T1)", r, num_games);
    }

    // Team 0 has one MinBot partner: H+Min vs H+H
    {
        HeuristicBot a{"H0"};
        MinBot c{"Min0"};        // Team 0's weaker partner
        HeuristicBot b{"H1"}, d{"H2"};
        std::array<IBot*, 4> players = {&a, &b, &c, &d};
        auto r = run_benchmark(players, num_games, max_steps);
        print_result("H+MinBot(T0) vs HH(T1)", r, num_games);
    }

    return 0;
}

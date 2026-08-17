# coin_toss.cpp
/**
 * 🪙 Coin Toss – Advanced Flip Series Analyzer (C++ Edition)
 * Features: biased probability, streaks, ASCII chart, export, colorful CLI
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <random>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cctype>

#ifdef _WIN32
#include <windows.h>
#endif

// ─── Colors ──────────────────────────────────────────────────────────────────

#ifdef _WIN32
HANDLE hConsole;
void setColor(int color) { SetConsoleTextAttribute(hConsole, color); }
#define RESET_COLOR setColor(7)
#define COLOR_RED setColor(12)
#define COLOR_GREEN setColor(10)
#define COLOR_YELLOW setColor(14)
#define COLOR_BLUE setColor(9)
#define COLOR_MAGENTA setColor(13)
#define COLOR_CYAN setColor(11)
#define COLOR_BRIGHT setColor(15)
#define COLOR_DIM setColor(8)
#else
#define RESET_COLOR std::cout << "\x1b[0m"
#define COLOR_RED std::cout << "\x1b[31m"
#define COLOR_GREEN std::cout << "\x1b[32m"
#define COLOR_YELLOW std::cout << "\x1b[33m"
#define COLOR_BLUE std::cout << "\x1b[34m"
#define COLOR_MAGENTA std::cout << "\x1b[35m"
#define COLOR_CYAN std::cout << "\x1b[36m"
#define COLOR_BRIGHT std::cout << "\x1b[1m"
#define COLOR_DIM std::cout << "\x1b[2m"
#endif

#define C(str, color) color << str << RESET_COLOR

// ─── Helpers ─────────────────────────────────────────────────────────────────

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// ─── Core Logic ──────────────────────────────────────────────────────────────

class CoinToss {
public:
    CoinToss(double prob = 0.5) : prob(prob), rng(std::random_device{}()) {}
    std::string flip() {
        std::uniform_real_distribution<> dist(0.0, 1.0);
        return dist(rng) < prob ? "H" : "T";
    }
    std::vector<std::string> flipSeries(int n) {
        std::vector<std::string> res;
        res.reserve(n);
        for (int i = 0; i < n; ++i) res.push_back(flip());
        return res;
    }
private:
    double prob;
    std::mt19937 rng;
};

// ─── Statistics ─────────────────────────────────────────────────────────────

struct Stats {
    int heads, tails, total;
    double headsPercent, tailsPercent;
    int longestRunHeads, longestRunTails;
    int longestStreakHeads, longestStreakTails;
    int runs;
};

Stats analyzeSeries(const std::vector<std::string>& series) {
    Stats s{};
    if (series.empty()) return s;
    for (const auto& c : series) {
        if (c == "H") s.heads++;
    }
    s.total = series.size();
    s.tails = s.total - s.heads;
    s.headsPercent = s.total ? (double)s.heads / s.total * 100.0 : 0.0;
    s.tailsPercent = s.total ? (double)s.tails / s.total * 100.0 : 0.0;

    int currentRun = 1;
    for (size_t i = 1; i < series.size(); ++i) {
        if (series[i] == series[i-1]) {
            currentRun++;
        } else {
            if (series[i-1] == "H") s.longestRunHeads = std::max(s.longestRunHeads, currentRun);
            else s.longestRunTails = std::max(s.longestRunTails, currentRun);
            currentRun = 1;
        }
    }
    if (!series.empty()) {
        if (series.back() == "H") s.longestRunHeads = std::max(s.longestRunHeads, currentRun);
        else s.longestRunTails = std::max(s.longestRunTails, currentRun);
    }
    s.longestStreakHeads = s.longestRunHeads;
    s.longestStreakTails = s.longestRunTails;
    s.runs = 1;
    for (size_t i = 1; i < series.size(); ++i) {
        if (series[i] != series[i-1]) s.runs++;
    }
    return s;
}

std::string drawAsciiChart(const Stats& stats, int width = 40) {
    int hBar = (int)(stats.headsPercent / 100.0 * width);
    int tBar = (int)(stats.tailsPercent / 100.0 * width);
    std::string line = "H ";
    line += C(std::string(hBar, '█'), COLOR_GREEN);
    line += std::string(width - hBar, ' ');
    line += " ";
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f%%\n", stats.headsPercent);
    line += buf;
    line += "T ";
    line += C(std::string(tBar, '█'), COLOR_RED);
    line += std::string(width - tBar, ' ');
    line += " ";
    snprintf(buf, sizeof(buf), "%.1f%%", stats.tailsPercent);
    line += buf;
    return line;
}

// ─── Export ─────────────────────────────────────────────────────────────────

void exportResults(const std::vector<std::string>& series, const Stats& stats, const std::string& filename = "") {
    std::string fname = filename;
    if (fname.empty()) {
        std::time_t t = std::time(nullptr);
        std::tm* tm = std::localtime(&t);
        std::ostringstream oss;
        oss << std::put_time(tm, "coin_toss_%Y%m%d_%H%M%S.json");
        fname = oss.str();
    }
    std::ofstream file(fname);
    if (!file) {
        std::cerr << C("❌ Failed to export.", COLOR_RED) << std::endl;
        return;
    }
    file << "{\n";
    file << "  \"flips\": [";
    for (size_t i = 0; i < series.size(); ++i) {
        file << "\"" << series[i] << "\"";
        if (i+1 < series.size()) file << ",";
    }
    file << "],\n";
    file << "  \"stats\": {\n";
    file << "    \"heads\": " << stats.heads << ",\n";
    file << "    \"tails\": " << stats.tails << ",\n";
    file << "    \"total\": " << stats.total << ",\n";
    file << "    \"headsPercent\": " << stats.headsPercent << ",\n";
    file << "    \"tailsPercent\": " << stats.tailsPercent << ",\n";
    file << "    \"longestRunHeads\": " << stats.longestRunHeads << ",\n";
    file << "    \"longestRunTails\": " << stats.longestRunTails << ",\n";
    file << "    \"longestStreakHeads\": " << stats.longestStreakHeads << ",\n";
    file << "    \"longestStreakTails\": " << stats.longestStreakTails << ",\n";
    file << "    \"runs\": " << stats.runs << "\n";
    file << "  },\n";
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);
    std::ostringstream ts;
    ts << std::put_time(tm, "%Y-%m-%dT%H:%M:%SZ");
    file << "  \"timestamp\": \"" << ts.str() << "\"\n";
    file << "}\n";
    file.close();
    std::cout << C("✅ Results exported to " + fname, COLOR_GREEN) << std::endl;
}

// ─── Main App ──────────────────────────────────────────────────────────────

class CoinApp {
public:
    CoinApp() : rng(std::random_device{}()), prob(0.5), toss(prob) {}

    void run() {
        std::cout << "\033[2J\033[1;1H";
        std::cout << C("\n🪙 Coin Toss – Advanced Flip Analyzer", COLOR_BRIGHT) << C("", COLOR_CYAN) << std::endl;
        std::cout << C("Flip, analyze, and master probability!", COLOR_DIM) << std::endl;

        while (true) {
            showMenu();
            std::string choice = ask("Your choice: ");
            if (choice == "1") singleFlip();
            else if (choice == "2") seriesFlips();
            else if (choice == "3") showStats();
            else if (choice == "4") setProbability();
            else if (choice == "5") exportResults();
            else if (choice == "6") continuousFlip();
            else if (choice == "0") {
                std::cout << C("👋 Goodbye!", COLOR_CYAN) << std::endl;
                break;
            } else {
                std::cout << C("❌ Invalid choice.", COLOR_RED) << std::endl;
            }
            if (choice != "0" && choice != "6") {
                std::cout << "\nPress Enter to continue...";
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cin.get();
            }
        }
    }

private:
    std::mt19937 rng;
    double prob;
    CoinToss toss;
    std::vector<std::string> lastSeries;
    Stats lastStats;

    std::string ask(const std::string& prompt) {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        return trim(line);
    }

    int askInt(const std::string& prompt, int def) {
        while (true) {
            std::string ans = ask(prompt);
            if (ans.empty()) return def;
            try { return std::stoi(ans); }
            catch (...) { std::cout << C("Please enter a number.", COLOR_YELLOW) << std::endl; }
        }
    }

    double askDouble(const std::string& prompt, double def) {
        while (true) {
            std::string ans = ask(prompt);
            if (ans.empty()) return def;
            try { return std::stod(ans); }
            catch (...) { std::cout << C("Please enter a number.", COLOR_YELLOW) << std::endl; }
        }
    }

    void showMenu() {
        std::cout << "\n" << C(std::string(50, '═'), COLOR_CYAN) << std::endl;
        std::cout << C("🪙 COIN TOSS ANALYZER", COLOR_BRIGHT) << C("", COLOR_CYAN) << std::endl;
        std::cout << C(std::string(50, '═'), COLOR_CYAN) << std::endl;
        std::cout << "  Probability of Heads: " << std::fixed << std::setprecision(2) << prob << std::endl;
        std::cout << C(std::string(50, '═'), COLOR_CYAN) << std::endl;
        std::cout << "  1. 🪙 Single Flip" << std::endl;
        std::cout << "  2. 📊 Series of Flips" << std::endl;
        std::cout << "  3. 📈 Show Statistics" << std::endl;
        std::cout << "  4. 🎯 Set Probability" << std::endl;
        std::cout << "  5. 💾 Export Results" << std::endl;
        std::cout << "  6. 🔁 Continuous Flip Mode" << std::endl;
        std::cout << "  0. 🚪 Exit" << std::endl;
        std::cout << C(std::string(50, '═'), COLOR_CYAN) << std::endl;
    }

    void singleFlip() {
        std::string res = toss.flip();
        std::cout << "\n🪙 Result: " << C(res, res=="H"?COLOR_GREEN:COLOR_RED) << " (Heads=" << std::fixed << std::setprecision(2) << prob << ")" << std::endl;
        lastSeries.push_back(res);
        lastStats = analyzeSeries(lastSeries);
    }

    void seriesFlips() {
        int n = askInt("Number of flips (default 10): ", 10);
        if (n <= 0) { std::cout << C("Number must be positive.", COLOR_RED) << std::endl; return; }
        std::cout << C("Flipping " + std::to_string(n) + " times...", COLOR_DIM) << std::endl;
        lastSeries = toss.flipSeries(n);
        lastStats = analyzeSeries(lastSeries);
        showStats();
    }

    void showStats() {
        if (lastStats.total == 0) {
            std::cout << C("No data yet. Do some flips!", COLOR_YELLOW) << std::endl;
            return;
        }
        std::cout << "\n📊 STATISTICS" << std::endl;
        std::cout << C(std::string(30, '─'), COLOR_DIM) << std::endl;
        std::cout << "  Total Flips: " << lastStats.total << std::endl;
        std::cout << "  Heads: " << lastStats.heads << " (" << std::fixed << std::setprecision(1) << lastStats.headsPercent << "%)" << std::endl;
        std::cout << "  Tails: " << lastStats.tails << " (" << lastStats.tailsPercent << "%)" << std::endl;
        std::cout << "  Longest Run (H): " << lastStats.longestRunHeads << std::endl;
        std::cout << "  Longest Run (T): " << lastStats.longestRunTails << std::endl;
        std::cout << "  Number of Runs: " << lastStats.runs << std::endl;
        std::cout << "\n📊 Distribution:" << std::endl;
        std::cout << drawAsciiChart(lastStats) << std::endl;
    }

    void setProbability() {
        double p = askDouble("Probability of Heads (0.0–1.0): ", 0.5);
        if (p >= 0.0 && p <= 1.0) {
            prob = p;
            toss = CoinToss(p);
            std::cout << C("✅ Probability set to " + std::to_string(p).substr(0,4), COLOR_GREEN) << std::endl;
        } else {
            std::cout << C("Invalid probability.", COLOR_RED) << std::endl;
        }
    }

    void exportResults() {
        if (lastStats.total == 0) {
            std::cout << C("No data to export.", COLOR_YELLOW) << std::endl;
            return;
        }
        std::string fname = ask("Filename (leave empty for auto): ");
        ::exportResults(lastSeries, lastStats, fname);
    }

    void continuousFlip() {
        std::cout << C("\n🔁 Continuous Flip Mode (press Ctrl+C to stop)", COLOR_CYAN) << std::endl;
        int count = 0, heads = 0, tails = 0;
        std::vector<std::string> series;
        while (true) {
            std::string res = toss.flip();
            series.push_back(res);
            if (res == "H") heads++; else tails++;
            count++;
            std::cout << "\r " << count << " flips: H=" << heads << " (" << std::fixed << std::setprecision(1) << (double)heads/count*100 << "%) T=" << tails << " (" << (double)tails/count*100 << "%)" << std::flush;
        }
    }
};

int main() {
#ifdef _WIN32
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
    try {
        CoinApp app;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << C("❌ Unexpected error: ", COLOR_RED) << e.what() << std::endl;
        return 1;
    }
    return 0;
}

# CoinToss.java
/**
 * 🪙 Coin Toss – Advanced Flip Series Analyzer (Java Edition)
 * Features: biased probability, streaks, ASCII chart, export, colorful CLI
 * Requires: Java 17+
 */

import java.io.*;
import java.nio.file.*;
import java.time.*;
import java.util.*;
import java.util.stream.Collectors;

// ─── Core Logic ──────────────────────────────────────────────────────────────

class CoinToss {
    private final double prob;
    private final Random random;

    public CoinToss(double prob) {
        this.prob = prob;
        this.random = new Random();
    }

    public String flip() {
        return random.nextDouble() < prob ? "H" : "T";
    }

    public List<String> flipSeries(int n) {
        List<String> res = new ArrayList<>(n);
        for (int i = 0; i < n; i++) res.add(flip());
        return res;
    }
}

// ─── Statistics ─────────────────────────────────────────────────────────────

class Stats {
    int heads, tails, total;
    double headsPercent, tailsPercent;
    int longestRunHeads, longestRunTails;
    int longestStreakHeads, longestStreakTails;
    int runs;

    Stats() {}

    Stats(int heads, int tails, int total, double headsPercent, double tailsPercent,
          int longestRunHeads, int longestRunTails, int longestStreakHeads, int longestStreakTails, int runs) {
        this.heads = heads;
        this.tails = tails;
        this.total = total;
        this.headsPercent = headsPercent;
        this.tailsPercent = tailsPercent;
        this.longestRunHeads = longestRunHeads;
        this.longestRunTails = longestRunTails;
        this.longestStreakHeads = longestStreakHeads;
        this.longestStreakTails = longestStreakTails;
        this.runs = runs;
    }
}

class Analyzer {
    static Stats analyze(List<String> series) {
        if (series.isEmpty()) return new Stats();
        int heads = (int) series.stream().filter(s -> s.equals("H")).count();
        int total = series.size();
        int tails = total - heads;
        int longestRunHeads = 0, longestRunTails = 0;
        int currentRun = 1;
        for (int i = 1; i < series.size(); i++) {
            if (series.get(i).equals(series.get(i-1))) {
                currentRun++;
            } else {
                if (series.get(i-1).equals("H")) longestRunHeads = Math.max(longestRunHeads, currentRun);
                else longestRunTails = Math.max(longestRunTails, currentRun);
                currentRun = 1;
            }
        }
        if (!series.isEmpty()) {
            if (series.get(series.size()-1).equals("H")) longestRunHeads = Math.max(longestRunHeads, currentRun);
            else longestRunTails = Math.max(longestRunTails, currentRun);
        }
        int runs = 1;
        for (int i = 1; i < series.size(); i++) {
            if (!series.get(i).equals(series.get(i-1))) runs++;
        }
        double hP = total > 0 ? (double)heads / total * 100 : 0;
        double tP = total > 0 ? (double)tails / total * 100 : 0;
        return new Stats(heads, tails, total, hP, tP,
                         longestRunHeads, longestRunTails,
                         longestRunHeads, longestRunTails, runs);
    }

    static String drawAsciiChart(Stats stats, int width) {
        int hBar = (int)(stats.headsPercent / 100 * width);
        int tBar = (int)(stats.tailsPercent / 100 * width);
        String line = "H " + colorString("█".repeat(hBar), "green") + " ".repeat(width - hBar) + " " + String.format("%.1f%%\n", stats.headsPercent);
        line += "T " + colorString("█".repeat(tBar), "red") + " ".repeat(width - tBar) + " " + String.format("%.1f%%", stats.tailsPercent);
        return line;
    }

    private static String colorString(String s, String color) {
        // Simple color mapping for cross-platform; we'll use ANSI codes
        String code = switch (color) {
            case "green" -> "\u001B[32m";
            case "red" -> "\u001B[31m";
            default -> "\u001B[0m";
        };
        return code + s + "\u001B[0m";
    }
}

// ─── Export ─────────────────────────────────────────────────────────────────

class Exporter {
    static void exportResults(List<String> series, Stats stats, String filename) throws IOException {
        if (filename == null || filename.isEmpty()) {
            String ts = LocalDateTime.now().format(DateTimeFormatter.ofPattern("yyyyMMdd_HHmmss"));
            filename = "coin_toss_" + ts + ".json";
        }
        Map<String, Object> data = new LinkedHashMap<>();
        data.put("flips", series);
        data.put("stats", Map.of(
            "heads", stats.heads,
            "tails", stats.tails,
            "total", stats.total,
            "headsPercent", stats.headsPercent,
            "tailsPercent", stats.tailsPercent,
            "longestRunHeads", stats.longestRunHeads,
            "longestRunTails", stats.longestRunTails,
            "longestStreakHeads", stats.longestStreakHeads,
            "longestStreakTails", stats.longestStreakTails,
            "runs", stats.runs
        ));
        data.put("timestamp", Instant.now().toString());
        String json = new com.google.gson.GsonBuilder().setPrettyPrinting().create().toJson(data);
        Files.writeString(Paths.get(filename), json);
        System.out.println(colorString("✅ Results exported to " + filename, "green"));
    }

    private static String colorString(String s, String color) {
        String code = switch (color) {
            case "green" -> "\u001B[32m";
            default -> "\u001B[0m";
        };
        return code + s + "\u001B[0m";
    }
}

// ─── Main App ──────────────────────────────────────────────────────────────

public class CoinTossApp {
    private static final String RESET = "\u001B[0m";
    private static final String BRIGHT = "\u001B[1m";
    private static final String DIM = "\u001B[2m";
    private static final String RED = "\u001B[31m";
    private static final String GREEN = "\u001B[32m";
    private static final String YELLOW = "\u001B[33m";
    private static final String CYAN = "\u001B[36m";

    private static String c(String text, String color) {
        return color + text + RESET;
    }

    private final Scanner scanner;
    private double prob;
    private CoinToss toss;
    private List<String> lastSeries;
    private Stats lastStats;

    public CoinTossApp() {
        scanner = new Scanner(System.in);
        prob = 0.5;
        toss = new CoinToss(prob);
        lastSeries = new ArrayList<>();
        lastStats = new Stats();
    }

    private String ask(String prompt) {
        System.out.print(prompt);
        return scanner.nextLine().trim();
    }

    private int askInt(String prompt, int def) {
        while (true) {
            String ans = ask(prompt);
            if (ans.isEmpty()) return def;
            try { return Integer.parseInt(ans); }
            catch (NumberFormatException e) { System.out.println(c("Please enter a number.", YELLOW)); }
        }
    }

    private double askDouble(String prompt, double def) {
        while (true) {
            String ans = ask(prompt);
            if (ans.isEmpty()) return def;
            try { return Double.parseDouble(ans); }
            catch (NumberFormatException e) { System.out.println(c("Please enter a number.", YELLOW)); }
        }
    }

    private void showMenu() {
        System.out.println("\n" + c("═".repeat(50), CYAN));
        System.out.println(c("🪙 COIN TOSS ANALYZER", BRIGHT + CYAN));
        System.out.println(c("═".repeat(50), CYAN));
        System.out.printf("  Probability of Heads: %.2f\n", prob);
        System.out.println(c("═".repeat(50), CYAN));
        System.out.println("  1. 🪙 Single Flip");
        System.out.println("  2. 📊 Series of Flips");
        System.out.println("  3. 📈 Show Statistics");
        System.out.println("  4. 🎯 Set Probability");
        System.out.println("  5. 💾 Export Results");
        System.out.println("  6. 🔁 Continuous Flip Mode");
        System.out.println("  0. 🚪 Exit");
        System.out.println(c("═".repeat(50), CYAN));
    }

    private void singleFlip() {
        String res = toss.flip();
        String color = res.equals("H") ? GREEN : RED;
        System.out.printf("\n🪙 Result: %s (Heads=%.2f)\n", c(res, color), prob);
        lastSeries.add(res);
        lastStats = Analyzer.analyze(lastSeries);
    }

    private void seriesFlips() {
        int n = askInt("Number of flips (default 10): ", 10);
        if (n <= 0) { System.out.println(c("Number must be positive.", RED)); return; }
        System.out.println(c("Flipping " + n + " times...", DIM));
        lastSeries = toss.flipSeries(n);
        lastStats = Analyzer.analyze(lastSeries);
        showStats();
    }

    private void showStats() {
        if (lastStats.total == 0) {
            System.out.println(c("No data yet. Do some flips!", YELLOW));
            return;
        }
        System.out.println("\n📊 STATISTICS");
        System.out.println(c("─".repeat(30), DIM));
        System.out.printf("  Total Flips: %d\n", lastStats.total);
        System.out.printf("  Heads: %d (%.1f%%)\n", lastStats.heads, lastStats.headsPercent);
        System.out.printf("  Tails: %d (%.1f%%)\n", lastStats.tails, lastStats.tailsPercent);
        System.out.printf("  Longest Run (H): %d\n", lastStats.longestRunHeads);
        System.out.printf("  Longest Run (T): %d\n", lastStats.longestRunTails);
        System.out.printf("  Number of Runs: %d\n", lastStats.runs);
        System.out.println("\n📊 Distribution:");
        System.out.println(Analyzer.drawAsciiChart(lastStats, 40));
    }

    private void setProbability() {
        double p = askDouble("Probability of Heads (0.0–1.0): ", 0.5);
        if (p >= 0 && p <= 1) {
            prob = p;
            toss = new CoinToss(p);
            System.out.println(c(String.format("✅ Probability set to %.2f", p), GREEN));
        } else {
            System.out.println(c("Invalid probability.", RED));
        }
    }

    private void exportResults() {
        if (lastStats.total == 0) {
            System.out.println(c("No data to export.", YELLOW));
            return;
        }
        String fname = ask("Filename (leave empty for auto): ");
        if (fname.isEmpty()) fname = null;
        try {
            Exporter.exportResults(lastSeries, lastStats, fname);
        } catch (IOException e) {
            System.out.println(c("❌ Export failed: " + e.getMessage(), RED));
        }
    }

    private void continuousFlip() {
        System.out.println(c("\n🔁 Continuous Flip Mode (press Ctrl+C to stop)", CYAN));
        int count = 0, heads = 0, tails = 0;
        List<String> series = new ArrayList<>();
        while (true) {
            String res = toss.flip();
            series.add(res);
            if (res.equals("H")) heads++; else tails++;
            count++;
            System.out.printf("\r %d flips: H=%d (%.1f%%) T=%d (%.1f%%)", count, heads, (double)heads/count*100, tails, (double)tails/count*100);
        }
    }

    public void run() {
        System.out.print("\033[H\033[2J");
        System.out.flush();
        System.out.println(c("\n🪙 Coin Toss – Advanced Flip Analyzer", BRIGHT + CYAN));
        System.out.println(c("Flip, analyze, and master probability!", DIM));

        while (true) {
            showMenu();
            String choice = ask("Your choice: ");
            switch (choice) {
                case "1": singleFlip(); break;
                case "2": seriesFlips(); break;
                case "3": showStats(); break;
                case "4": setProbability(); break;
                case "5": exportResults(); break;
                case "6": continuousFlip(); break;
                case "0":
                    System.out.println(c("👋 Goodbye!", CYAN));
                    return;
                default:
                    System.out.println(c("❌ Invalid choice.", RED));
            }
            if (!choice.equals("0") && !choice.equals("6")) {
                System.out.print("\nPress Enter to continue...");
                scanner.nextLine();
            }
        }
    }

    public static void main(String[] args) {
        try {
            new CoinTossApp().run();
        } catch (Exception e) {
            System.err.println(c("❌ Unexpected error: " + e.getMessage(), RED));
            e.printStackTrace();
            System.exit(1);
        }
    }
}

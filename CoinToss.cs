# CoinToss.cs
/**
 * 🪙 Coin Toss – Advanced Flip Series Analyzer (C# Edition)
 * Features: biased probability, streaks, ASCII chart, export, colorful CLI
 * Requires: .NET 6.0+
 */

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.Json;
using System.Text.Json.Serialization;

// ─── Core Logic ──────────────────────────────────────────────────────────────

class CoinToss
{
    private readonly double prob;
    private readonly Random random = new();

    public CoinToss(double prob = 0.5) => this.prob = prob;

    public string Flip() => random.NextDouble() < prob ? "H" : "T";

    public List<string> FlipSeries(int n)
    {
        var res = new List<string>(n);
        for (int i = 0; i < n; i++) res.Add(Flip());
        return res;
    }
}

// ─── Statistics ─────────────────────────────────────────────────────────────

class Stats
{
    public int Heads { get; set; }
    public int Tails { get; set; }
    public int Total { get; set; }
    public double HeadsPercent { get; set; }
    public double TailsPercent { get; set; }
    public int LongestRunHeads { get; set; }
    public int LongestRunTails { get; set; }
    public int LongestStreakHeads { get; set; }
    public int LongestStreakTails { get; set; }
    public int Runs { get; set; }
}

static class Analyzer
{
    public static Stats Analyze(List<string> series)
    {
        if (series.Count == 0) return new Stats();
        int heads = series.Count(s => s == "H");
        int total = series.Count;
        int tails = total - heads;
        int longestRunHeads = 0, longestRunTails = 0;
        int currentRun = 1;
        for (int i = 1; i < series.Count; i++)
        {
            if (series[i] == series[i-1]) currentRun++;
            else
            {
                if (series[i-1] == "H") longestRunHeads = Math.Max(longestRunHeads, currentRun);
                else longestRunTails = Math.Max(longestRunTails, currentRun);
                currentRun = 1;
            }
        }
        if (series.Count > 0)
        {
            if (series[^1] == "H") longestRunHeads = Math.Max(longestRunHeads, currentRun);
            else longestRunTails = Math.Max(longestRunTails, currentRun);
        }
        int runs = 1;
        for (int i = 1; i < series.Count; i++)
            if (series[i] != series[i-1]) runs++;
        return new Stats
        {
            Heads = heads,
            Tails = tails,
            Total = total,
            HeadsPercent = total > 0 ? (double)heads / total * 100 : 0,
            TailsPercent = total > 0 ? (double)tails / total * 100 : 0,
            LongestRunHeads = longestRunHeads,
            LongestRunTails = longestRunTails,
            LongestStreakHeads = longestRunHeads,
            LongestStreakTails = longestRunTails,
            Runs = runs
        };
    }

    public static string DrawAsciiChart(Stats stats, int width = 40)
    {
        int hBar = (int)(stats.HeadsPercent / 100 * width);
        int tBar = (int)(stats.TailsPercent / 100 * width);
        string line = $"H {ColorString(new string('█', hBar), "green")}{new string(' ', width - hBar)} {stats.HeadsPercent:F1}%\n";
        line += $"T {ColorString(new string('█', tBar), "red")}{new string(' ', width - tBar)} {stats.TailsPercent:F1}%";
        return line;
    }

    private static string ColorString(string s, string color)
    {
        string code = color switch
        {
            "green" => "\u001B[32m",
            "red" => "\u001B[31m",
            _ => "\u001B[0m"
        };
        return code + s + "\u001B[0m";
    }
}

// ─── Export ─────────────────────────────────────────────────────────────────

class Exporter
{
    public static void ExportResults(List<string> series, Stats stats, string filename = null)
    {
        if (string.IsNullOrEmpty(filename))
        {
            string ts = DateTime.Now.ToString("yyyyMMdd_HHmmss");
            filename = $"coin_toss_{ts}.json";
        }
        var data = new
        {
            flips = series,
            stats = new
            {
                stats.Heads,
                stats.Tails,
                stats.Total,
                stats.HeadsPercent,
                stats.TailsPercent,
                stats.LongestRunHeads,
                stats.LongestRunTails,
                stats.LongestStreakHeads,
                stats.LongestStreakTails,
                stats.Runs
            },
            timestamp = DateTime.Now.ToString("o")
        };
        string json = JsonSerializer.Serialize(data, new JsonSerializerOptions { WriteIndented = true });
        File.WriteAllText(filename, json);
        Console.WriteLine(ColorString($"✅ Results exported to {filename}", "green"));
    }

    private static string ColorString(string s, string color)
    {
        string code = color switch
        {
            "green" => "\u001B[32m",
            _ => "\u001B[0m"
        };
        return code + s + "\u001B[0m";
    }
}

// ─── Main App ──────────────────────────────────────────────────────────────

class CoinTossApp
{
    private static readonly string Reset = "\u001B[0m";
    private static readonly string Bright = "\u001B[1m";
    private static readonly string Dim = "\u001B[2m";
    private static readonly string Red = "\u001B[31m";
    private static readonly string Green = "\u001B[32m";
    private static readonly string Yellow = "\u001B[33m";
    private static readonly string Cyan = "\u001B[36m";

    private static string C(string text, string color) => color + text + Reset;

    private double prob = 0.5;
    private CoinToss toss;
    private List<string> lastSeries = new();
    private Stats lastStats = new();

    public CoinTossApp() => toss = new CoinToss(prob);

    private string Ask(string prompt)
    {
        Console.Write(prompt);
        return Console.ReadLine()?.Trim() ?? "";
    }

    private int AskInt(string prompt, int def)
    {
        while (true)
        {
            string ans = Ask(prompt);
            if (string.IsNullOrEmpty(ans)) return def;
            if (int.TryParse(ans, out int val)) return val;
            Console.WriteLine(C("Please enter a number.", Yellow));
        }
    }

    private double AskDouble(string prompt, double def)
    {
        while (true)
        {
            string ans = Ask(prompt);
            if (string.IsNullOrEmpty(ans)) return def;
            if (double.TryParse(ans, out double val)) return val;
            Console.WriteLine(C("Please enter a number.", Yellow));
        }
    }

    private void ShowMenu()
    {
        Console.WriteLine("\n" + C(new string('═', 50), Cyan));
        Console.WriteLine(C("🪙 COIN TOSS ANALYZER", Bright + Cyan));
        Console.WriteLine(C(new string('═', 50), Cyan));
        Console.WriteLine($"  Probability of Heads: {prob:F2}");
        Console.WriteLine(C(new string('═', 50), Cyan));
        Console.WriteLine("  1. 🪙 Single Flip");
        Console.WriteLine("  2. 📊 Series of Flips");
        Console.WriteLine("  3. 📈 Show Statistics");
        Console.WriteLine("  4. 🎯 Set Probability");
        Console.WriteLine("  5. 💾 Export Results");
        Console.WriteLine("  6. 🔁 Continuous Flip Mode");
        Console.WriteLine("  0. 🚪 Exit");
        Console.WriteLine(C(new string('═', 50), Cyan));
    }

    private void SingleFlip()
    {
        string res = toss.Flip();
        string color = res == "H" ? Green : Red;
        Console.WriteLine($"\n🪙 Result: {C(res, color)} (Heads={prob:F2})");
        lastSeries.Add(res);
        lastStats = Analyzer.Analyze(lastSeries);
    }

    private void SeriesFlips()
    {
        int n = AskInt("Number of flips (default 10): ", 10);
        if (n <= 0) { Console.WriteLine(C("Number must be positive.", Red)); return; }
        Console.WriteLine(C($"Flipping {n} times...", Dim));
        lastSeries = toss.FlipSeries(n);
        lastStats = Analyzer.Analyze(lastSeries);
        ShowStats();
    }

    private void ShowStats()
    {
        if (lastStats.Total == 0)
        {
            Console.WriteLine(C("No data yet. Do some flips!", Yellow));
            return;
        }
        Console.WriteLine("\n📊 STATISTICS");
        Console.WriteLine(C(new string('─', 30), Dim));
        Console.WriteLine($"  Total Flips: {lastStats.Total}");
        Console.WriteLine($"  Heads: {lastStats.Heads} ({lastStats.HeadsPercent:F1}%)");
        Console.WriteLine($"  Tails: {lastStats.Tails} ({lastStats.TailsPercent:F1}%)");
        Console.WriteLine($"  Longest Run (H): {lastStats.LongestRunHeads}");
        Console.WriteLine($"  Longest Run (T): {lastStats.LongestRunTails}");
        Console.WriteLine($"  Number of Runs: {lastStats.Runs}");
        Console.WriteLine("\n📊 Distribution:");
        Console.WriteLine(Analyzer.DrawAsciiChart(lastStats));
    }

    private void SetProbability()
    {
        double p = AskDouble("Probability of Heads (0.0–1.0): ", 0.5);
        if (p >= 0 && p <= 1)
        {
            prob = p;
            toss = new CoinToss(p);
            Console.WriteLine(C($"✅ Probability set to {p:F2}", Green));
        }
        else Console.WriteLine(C("Invalid probability.", Red));
    }

    private void ExportResults()
    {
        if (lastStats.Total == 0)
        {
            Console.WriteLine(C("No data to export.", Yellow));
            return;
        }
        string fname = Ask("Filename (leave empty for auto): ");
        if (string.IsNullOrEmpty(fname)) fname = null;
        Exporter.ExportResults(lastSeries, lastStats, fname);
    }

    private void ContinuousFlip()
    {
        Console.WriteLine(C("\n🔁 Continuous Flip Mode (press Ctrl+C to stop)", Cyan));
        int count = 0, heads = 0, tails = 0;
        var series = new List<string>();
        while (true)
        {
            string res = toss.Flip();
            series.Add(res);
            if (res == "H") heads++; else tails++;
            count++;
            Console.Write($"\r {count} flips: H={heads} ({(double)heads/count*100:F1}%) T={tails} ({(double)tails/count*100:F1}%)");
        }
    }

    public void Run()
    {
        Console.Clear();
        Console.WriteLine(C("\n🪙 Coin Toss – Advanced Flip Analyzer", Bright + Cyan));
        Console.WriteLine(C("Flip, analyze, and master probability!", Dim));

        while (true)
        {
            ShowMenu();
            string choice = Ask("Your choice: ");
            switch (choice)
            {
                case "1": SingleFlip(); break;
                case "2": SeriesFlips(); break;
                case "3": ShowStats(); break;
                case "4": SetProbability(); break;
                case "5": ExportResults(); break;
                case "6": ContinuousFlip(); break;
                case "0":
                    Console.WriteLine(C("👋 Goodbye!", Cyan));
                    return;
                default:
                    Console.WriteLine(C("❌ Invalid choice.", Red));
                    break;
            }
            if (choice != "0" && choice != "6")
            {
                Console.Write("\nPress Enter to continue...");
                Console.ReadLine();
            }
        }
    }

    public static void Main()
    {
        try
        {
            new CoinTossApp().Run();
        }
        catch (Exception ex)
        {
            Console.WriteLine(C($"❌ Unexpected error: {ex.Message}", Red));
            Environment.Exit(1);
        }
    }
}

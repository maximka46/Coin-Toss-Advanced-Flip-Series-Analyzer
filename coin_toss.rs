# coin_toss.rs
/**
 * 🪙 Coin Toss – Advanced Flip Series Analyzer (Rust Edition)
 * Features: biased probability, streaks, ASCII chart, export, colorful CLI
 * Dependencies: rand, colored, serde, serde_json, chrono
 */

use rand::Rng;
use rand::SeedableRng;
use rand::rngs::StdRng;
use std::io::{self, Write, BufRead};
use std::fs;
use chrono::Utc;
use serde::{Deserialize, Serialize};
use colored::*;

// ─── Types ──────────────────────────────────────────────────────────────────

#[derive(Debug, Serialize, Deserialize, Clone)]
struct Stats {
    heads: u32,
    tails: u32,
    total: u32,
    heads_percent: f64,
    tails_percent: f64,
    longest_run_heads: u32,
    longest_run_tails: u32,
    longest_streak_heads: u32,
    longest_streak_tails: u32,
    runs: u32,
}

#[derive(Debug, Serialize, Deserialize)]
struct ExportData {
    flips: Vec<String>,
    stats: Stats,
    timestamp: String,
}

// ─── Core Logic ──────────────────────────────────────────────────────────────

struct CoinToss {
    prob: f64,
    rng: StdRng,
}

impl CoinToss {
    fn new(prob: f64) -> Self {
        let seed = rand::thread_rng().gen();
        Self { prob, rng: StdRng::seed_from_u64(seed) }
    }
    fn flip(&mut self) -> String {
        if self.rng.gen::<f64>() < self.prob {
            "H".to_string()
        } else {
            "T".to_string()
        }
    }
    fn flip_series(&mut self, n: usize) -> Vec<String> {
        let mut v = Vec::with_capacity(n);
        for _ in 0..n {
            v.push(self.flip());
        }
        v
    }
}

// ─── Statistics ─────────────────────────────────────────────────────────────

fn analyze_series(series: &[String]) -> Stats {
    if series.is_empty() {
        return Stats { heads: 0, tails: 0, total: 0, heads_percent: 0.0, tails_percent: 0.0,
                       longest_run_heads: 0, longest_run_tails: 0, longest_streak_heads: 0,
                       longest_streak_tails: 0, runs: 0 };
    }
    let heads = series.iter().filter(|&s| s == "H").count() as u32;
    let total = series.len() as u32;
    let tails = total - heads;
    let mut longest_run_heads = 0;
    let mut longest_run_tails = 0;
    let mut current_run = 1;
    for i in 1..series.len() {
        if series[i] == series[i-1] {
            current_run += 1;
        } else {
            if series[i-1] == "H" {
                longest_run_heads = std::cmp::max(longest_run_heads, current_run);
            } else {
                longest_run_tails = std::cmp::max(longest_run_tails, current_run);
            }
            current_run = 1;
        }
    }
    if !series.is_empty() {
        if series[series.len()-1] == "H" {
            longest_run_heads = std::cmp::max(longest_run_heads, current_run);
        } else {
            longest_run_tails = std::cmp::max(longest_run_tails, current_run);
        }
    }
    let mut runs = 1;
    for i in 1..series.len() {
        if series[i] != series[i-1] { runs += 1; }
    }
    Stats {
        heads,
        tails,
        total,
        heads_percent: if total > 0 { heads as f64 / total as f64 * 100.0 } else { 0.0 },
        tails_percent: if total > 0 { tails as f64 / total as f64 * 100.0 } else { 0.0 },
        longest_run_heads,
        longest_run_tails,
        longest_streak_heads: longest_run_heads,
        longest_streak_tails: longest_run_tails,
        runs,
    }
}

fn draw_ascii_chart(stats: &Stats, width: usize) -> String {
    let h = stats.heads_percent;
    let t = stats.tails_percent;
    let h_bar = (h / 100.0 * width as f64) as usize;
    let t_bar = (t / 100.0 * width as f64) as usize;
    let mut line = format!("H {}{} {:.1}%\n",
        "█".repeat(h_bar).green(),
        " ".repeat(width - h_bar),
        h);
    line.push_str(&format!("T {}{} {:.1}%",
        "█".repeat(t_bar).red(),
        " ".repeat(width - t_bar),
        t));
    line
}

// ─── Export ──────────────────────────────────────────────────────────────────

fn export_results(series: &[String], stats: &Stats, filename: Option<&str>) -> Result<(), Box<dyn std::error::Error>> {
    let fname = match filename {
        Some(s) if !s.is_empty() => s.to_string(),
        _ => format!("coin_toss_{}.json", Utc::now().format("%Y%m%d_%H%M%S")),
    };
    let data = ExportData {
        flips: series.to_vec(),
        stats: stats.clone(),
        timestamp: Utc::now().to_rfc3339(),
    };
    let json = serde_json::to_string_pretty(&data)?;
    fs::write(&fname, json)?;
    println!("{}", format!("✅ Results exported to {}", fname).green());
    Ok(())
}

// ─── Main App ──────────────────────────────────────────────────────────────

struct CoinApp {
    prob: f64,
    toss: CoinToss,
    last_series: Vec<String>,
    last_stats: Stats,
}

impl CoinApp {
    fn new() -> Self {
        let prob = 0.5;
        Self {
            prob,
            toss: CoinToss::new(prob),
            last_series: Vec::new(),
            last_stats: Stats { heads: 0, tails: 0, total: 0, heads_percent: 0.0, tails_percent: 0.0,
                                longest_run_heads: 0, longest_run_tails: 0, longest_streak_heads: 0,
                                longest_streak_tails: 0, runs: 0 },
        }
    }

    fn ask(&self, prompt: &str) -> String {
        print!("{}", prompt);
        io::stdout().flush().unwrap();
        let mut line = String::new();
        io::stdin().read_line(&mut line).unwrap();
        line.trim().to_string()
    }

    fn ask_int(&mut self, prompt: &str, def: usize) -> usize {
        loop {
            let ans = self.ask(prompt);
            if ans.is_empty() { return def; }
            if let Ok(n) = ans.parse::<usize>() {
                return n;
            }
            println!("{}", "Please enter a number.".yellow());
        }
    }

    fn ask_float(&mut self, prompt: &str, def: f64) -> f64 {
        loop {
            let ans = self.ask(prompt);
            if ans.is_empty() { return def; }
            if let Ok(n) = ans.parse::<f64>() {
                return n;
            }
            println!("{}", "Please enter a number.".yellow());
        }
    }

    fn show_menu(&self) {
        println!("\n{}", "═".repeat(50).cyan());
        println!("{}", "🪙 COIN TOSS ANALYZER".bright().cyan());
        println!("{}", "═".repeat(50).cyan());
        println!("  Probability of Heads: {:.2}", self.prob);
        println!("{}", "═".repeat(50).cyan());
        println!("  1. 🪙 Single Flip");
        println!("  2. 📊 Series of Flips");
        println!("  3. 📈 Show Statistics");
        println!("  4. 🎯 Set Probability");
        println!("  5. 💾 Export Results");
        println!("  6. 🔁 Continuous Flip Mode");
        println!("  0. 🚪 Exit");
        println!("{}", "═".repeat(50).cyan());
    }

    fn single_flip(&mut self) {
        let res = self.toss.flip();
        let color = if res == "H" { "green" } else { "red" };
        let colored_res = if res == "H" { res.green() } else { res.red() };
        println!("\n🪙 Result: {} (Heads={:.2})", colored_res, self.prob);
        self.last_series.push(res);
        self.last_stats = analyze_series(&self.last_series);
    }

    fn series_flips(&mut self) {
        let n = self.ask_int("Number of flips (default 10): ", 10);
        if n == 0 {
            println!("{}", "Number must be positive.".red());
            return;
        }
        println!("{}", format!("Flipping {} times...", n).dimmed());
        let series = self.toss.flip_series(n);
        self.last_series = series;
        self.last_stats = analyze_series(&self.last_series);
        self.show_stats();
    }

    fn show_stats(&self) {
        if self.last_stats.total == 0 {
            println!("{}", "No data yet. Do some flips!".yellow());
            return;
        }
        let s = &self.last_stats;
        println!("\n📊 STATISTICS");
        println!("{}", "─".repeat(30).dimmed());
        println!("  Total Flips: {}", s.total);
        println!("  Heads: {} ({:.1}%)", s.heads, s.heads_percent);
        println!("  Tails: {} ({:.1}%)", s.tails, s.tails_percent);
        println!("  Longest Run (H): {}", s.longest_run_heads);
        println!("  Longest Run (T): {}", s.longest_run_tails);
        println!("  Number of Runs: {}", s.runs);
        println!("\n📊 Distribution:");
        println!("{}", draw_ascii_chart(s, 40));
    }

    fn set_probability(&mut self) {
        let p = self.ask_float("Probability of Heads (0.0–1.0): ", 0.5);
        if (0.0..=1.0).contains(&p) {
            self.prob = p;
            self.toss = CoinToss::new(p);
            println!("{}", format!("✅ Probability set to {:.2}", p).green());
        } else {
            println!("{}", "Invalid probability.".red());
        }
    }

    fn export_results(&self) {
        if self.last_stats.total == 0 {
            println!("{}", "No data to export.".yellow());
            return;
        }
        let filename = self.ask("Filename (leave empty for auto): ");
        let fname = if filename.is_empty() { None } else { Some(filename.as_str()) };
        if let Err(e) = export_results(&self.last_series, &self.last_stats, fname) {
            println!("{}", format!("❌ Export failed: {}", e).red());
        }
    }

    fn continuous_flip(&mut self) {
        println!("{}", "\n🔁 Continuous Flip Mode (press Ctrl+C to stop)".cyan());
        let mut count = 0;
        let mut heads = 0;
        let mut tails = 0;
        let mut series = Vec::new();
        loop {
            let res = self.toss.flip();
            series.push(res.clone());
            if res == "H" { heads += 1; } else { tails += 1; }
            count += 1;
            print!("\r {} flips: H={} ({:.1}%) T={} ({:.1}%)", count, heads, heads as f64 / count as f64 * 100.0, tails, tails as f64 / count as f64 * 100.0);
            io::stdout().flush().unwrap();
        }
    }

    fn run(&mut self) {
        println!("{}", "\n🪙 Coin Toss – Advanced Flip Analyzer".bright().cyan());
        println!("{}", "Flip, analyze, and master probability!".dimmed());

        loop {
            self.show_menu();
            let choice = self.ask("Your choice: ");
            match choice.as_str() {
                "1" => self.single_flip(),
                "2" => self.series_flips(),
                "3" => self.show_stats(),
                "4" => self.set_probability(),
                "5" => self.export_results(),
                "6" => { self.continuous_flip(); },
                "0" => {
                    println!("{}", "👋 Goodbye!".cyan());
                    return;
                }
                _ => println!("{}", "❌ Invalid choice.".red()),
            }
            if choice != "0" && choice != "6" {
                print!("\nPress Enter to continue...");
                io::stdout().flush().unwrap();
                let mut _dummy = String::new();
                io::stdin().read_line(&mut _dummy).unwrap();
            }
        }
    }
}

// ─── Main ────────────────────────────────────────────────────────────────────

fn main() {
    let mut app = CoinApp::new();
    app.run();
}

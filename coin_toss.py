# coin_toss.py
#!/usr/bin/env python3
"""
🪙 Coin Toss – Advanced Flip Series Analyzer (Python Edition)
Features: biased probability, streaks, ASCII chart, export, colorful UI
"""

import json
import os
import random
import sys
from datetime import datetime
from pathlib import Path
from typing import List, Tuple, Dict

try:
    from rich.console import Console
    from rich.table import Table
    from rich.panel import Panel
    from rich.prompt import Prompt, Confirm, FloatPrompt, IntPrompt
    from rich.progress import Progress, BarColumn, TextColumn
    from rich import box
    RICH_AVAILABLE = True
except ImportError:
    RICH_AVAILABLE = False
    print("⚠️  Install 'rich' for enhanced UI: pip install rich")


# ─── Colors ──────────────────────────────────────────────────────────────────

def c(text: str, color: str) -> str:
    colors = {
        "reset": "\033[0m", "bright": "\033[1m", "dim": "\033[2m",
        "red": "\033[31m", "green": "\033[32m", "yellow": "\033[33m",
        "blue": "\033[34m", "magenta": "\033[35m", "cyan": "\033[36m"
    }
    return f"{colors.get(color, '')}{text}{colors['reset']}"


# ─── Core Logic ──────────────────────────────────────────────────────────────

class CoinToss:
    def __init__(self, heads_probability: float = 0.5):
        self.prob = heads_probability

    def flip(self) -> str:
        return "H" if random.random() < self.prob else "T"

    def flip_series(self, n: int) -> List[str]:
        return [self.flip() for _ in range(n)]


# ─── Statistics ─────────────────────────────────────────────────────────────

def analyze_series(series: List[str]) -> Dict:
    if not series:
        return {}
    heads = series.count("H")
    tails = len(series) - heads
    total = len(series)
    # longest runs
    longest_run_heads = 0
    longest_run_tails = 0
    current_run = 1
    for i in range(1, len(series)):
        if series[i] == series[i-1]:
            current_run += 1
        else:
            if series[i-1] == "H":
                longest_run_heads = max(longest_run_heads, current_run)
            else:
                longest_run_tails = max(longest_run_tails, current_run)
            current_run = 1
    # last run
    if series:
        if series[-1] == "H":
            longest_run_heads = max(longest_run_heads, current_run)
        else:
            longest_run_tails = max(longest_run_tails, current_run)
    # number of runs (changes)
    runs = 1
    for i in range(1, len(series)):
        if series[i] != series[i-1]:
            runs += 1
    return {
        "heads": heads,
        "tails": tails,
        "total": total,
        "heads_percent": heads / total * 100 if total else 0,
        "tails_percent": tails / total * 100 if total else 0,
        "longest_run_heads": longest_run_heads,
        "longest_run_tails": longest_run_tails,
        "longest_streak_heads": longest_run_heads,  # alias
        "longest_streak_tails": longest_run_tails,
        "runs": runs
    }


def draw_ascii_chart(stats: Dict, width: int = 40) -> str:
    """Simple bar chart of heads vs tails."""
    h = stats.get("heads_percent", 0)
    t = stats.get("tails_percent", 0)
    h_bar = int(h / 100 * width)
    t_bar = int(t / 100 * width)
    line = f"H {c('█' * h_bar, 'green')}{' ' * (width - h_bar)} {h:.1f}%\n"
    line += f"T {c('█' * t_bar, 'red')}{' ' * (width - t_bar)} {t:.1f}%"
    return line


# ─── Export ──────────────────────────────────────────────────────────────────

def export_results(series: List[str], stats: Dict, filename: str = None):
    if not filename:
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        filename = f"coin_toss_{timestamp}.json"
    data = {
        "flips": series,
        "stats": stats,
        "timestamp": datetime.now().isoformat()
    }
    with open(filename, 'w') as f:
        json.dump(data, f, indent=2)
    print(c(f"✅ Results exported to {filename}", "green"))


# ─── Main App ──────────────────────────────────────────────────────────────

class CoinApp:
    def __init__(self):
        self.console = Console() if RICH_AVAILABLE else None
        self.prob = 0.5
        self.toss = CoinToss(self.prob)
        self.last_series: List[str] = []
        self.last_stats: Dict = {}

    def show_menu(self):
        if self.console:
            panel = Panel(
                f"[bold cyan]🪙 Coin Toss Analyzer[/bold cyan]\n"
                f"  Probability of Heads: {self.prob:.2f}",
                title="📋 Main Menu",
                border_style="blue"
            )
            self.console.print(panel)
            self.console.print(" [1] 🪙 Single Flip")
            self.console.print(" [2] 📊 Series of Flips")
            self.console.print(" [3] 📈 Show Statistics")
            self.console.print(" [4] 🎯 Set Probability")
            self.console.print(" [5] 💾 Export Results")
            self.console.print(" [6] 🔁 Continuous Flip Mode")
            self.console.print(" [0] 🚪 Exit")
        else:
            print("\n" + "="*50)
            print(c("🪙 COIN TOSS ANALYZER", "bright"))
            print("="*50)
            print(f"  Probability of Heads: {self.prob:.2f}")
            print("="*50)
            print("  1. 🪙 Single Flip")
            print("  2. 📊 Series of Flips")
            print("  3. 📈 Show Statistics")
            print("  4. 🎯 Set Probability")
            print("  5. 💾 Export Results")
            print("  6. 🔁 Continuous Flip Mode")
            print("  0. 🚪 Exit")
            print("="*50)

    def single_flip(self):
        result = self.toss.flip()
        color = "green" if result == "H" else "red"
        symbol = c("H" if result == "H" else "T", color)
        if self.console:
            self.console.print(f"\n🪙 Result: {symbol} (Heads={self.prob:.2f})")
        else:
            print(f"\n🪙 Result: {symbol} (Heads={self.prob:.2f})")
        # add to series?
        if not self.last_series:
            self.last_series = [result]
        else:
            self.last_series.append(result)
        self.last_stats = analyze_series(self.last_series)

    def series_flips(self):
        if self.console:
            n = IntPrompt.ask("Number of flips", default=10)
        else:
            try:
                n = int(input("Number of flips (default 10): ") or "10")
            except ValueError:
                n = 10
        if n <= 0:
            print(c("Number must be positive.", "red"))
            return
        if self.console:
            with Progress(BarColumn(), TextColumn("{task.percentage:>3.0f}%")) as progress:
                task = progress.add_task("[cyan]Flipping...", total=n)
                series = []
                for _ in range(n):
                    series.append(self.toss.flip())
                    progress.update(task, advance=1)
        else:
            print(c(f"Flipping {n} times...", "dim"))
            series = self.toss.flip_series(n)
        self.last_series = series
        self.last_stats = analyze_series(series)
        self.show_stats()

    def show_stats(self):
        if not self.last_stats:
            print(c("No data yet. Do some flips!", "yellow"))
            return
        stats = self.last_stats
        if self.console:
            table = Table(title="📊 Statistics", box=box.ROUNDED)
            table.add_column("Metric", style="cyan")
            table.add_column("Value", style="green")
            table.add_row("Total Flips", str(stats["total"]))
            table.add_row("Heads", f"{stats['heads']} ({stats['heads_percent']:.1f}%)")
            table.add_row("Tails", f"{stats['tails']} ({stats['tails_percent']:.1f}%)")
            table.add_row("Longest Run (H)", str(stats["longest_run_heads"]))
            table.add_row("Longest Run (T)", str(stats["longest_run_tails"]))
            table.add_row("Number of Runs", str(stats["runs"]))
            self.console.print(table)
            # Chart
            chart = draw_ascii_chart(stats)
            self.console.print(Panel(chart, title="📊 Distribution", border_style="green"))
        else:
            print("\n📊 STATISTICS")
            print(c("─"*30, "dim"))
            print(f"  Total Flips: {stats['total']}")
            print(f"  Heads: {stats['heads']} ({stats['heads_percent']:.1f}%)")
            print(f"  Tails: {stats['tails']} ({stats['tails_percent']:.1f}%)")
            print(f"  Longest Run (H): {stats['longest_run_heads']}")
            print(f"  Longest Run (T): {stats['longest_run_tails']}")
            print(f"  Number of Runs: {stats['runs']}")
            print("\n📊 Distribution:")
            print(draw_ascii_chart(stats))

    def set_probability(self):
        if self.console:
            p = FloatPrompt.ask("Probability of Heads (0.0–1.0)", default=0.5)
        else:
            try:
                p = float(input("Probability of Heads (0.0–1.0): ") or "0.5")
            except ValueError:
                p = 0.5
        if 0.0 <= p <= 1.0:
            self.prob = p
            self.toss = CoinToss(self.prob)
            print(c(f"✅ Probability set to {p:.2f}", "green"))
        else:
            print(c("Invalid probability.", "red"))

    def export(self):
        if not self.last_stats:
            print(c("No data to export.", "yellow"))
            return
        if self.console:
            filename = Prompt.ask("Filename (leave empty for auto)", default="")
        else:
            filename = input("Filename (leave empty for auto): ").strip()
        export_results(self.last_series, self.last_stats, filename or None)

    def continuous_flip(self):
        print(c("\n🔁 Continuous Flip Mode (press Ctrl+C to stop)", "cyan"))
        try:
            count = 0
            heads = 0
            tails = 0
            series = []
            while True:
                res = self.toss.flip()
                series.append(res)
                if res == "H":
                    heads += 1
                else:
                    tails += 1
                count += 1
                # Show live update
                sys.stdout.write(f"\r {count} flips: H={heads} ({heads/count*100:.1f}%) T={tails} ({tails/count*100:.1f}%)")
                sys.stdout.flush()
        except KeyboardInterrupt:
            print("\n")
            self.last_series = series
            self.last_stats = analyze_series(series)
            self.show_stats()

    def run(self):
        if self.console:
            self.console.print(Panel.fit("[bold cyan]🪙 Coin Toss – Advanced Flip Analyzer[/bold cyan]", border_style="cyan"))
        else:
            print(c("\n🪙 Coin Toss – Advanced Flip Analyzer", "bright"))
            print(c("Flip, analyze, and master probability!", "dim"))

        while True:
            self.show_menu()
            if self.console:
                choice = Prompt.ask("Your choice", choices=["0","1","2","3","4","5","6"])
            else:
                choice = input("Your choice: ").strip()

            if choice == "1":
                self.single_flip()
            elif choice == "2":
                self.series_flips()
            elif choice == "3":
                self.show_stats()
            elif choice == "4":
                self.set_probability()
            elif choice == "5":
                self.export()
            elif choice == "6":
                self.continuous_flip()
            elif choice == "0":
                print(c("👋 Goodbye!", "cyan"))
                break
            else:
                print(c("❌ Invalid choice.", "red"))

            if choice != "0" and choice != "6":  # continuous mode already waits
                if self.console:
                    self.console.print("\n[dim]Press Enter to continue...[/dim]")
                    input()
                else:
                    input("\nPress Enter to continue...")


def main():
    try:
        app = CoinApp()
        app.run()
    except KeyboardInterrupt:
        print("\n👋 Goodbye!")
        sys.exit(0)
    except Exception as e:
        print(c(f"❌ Unexpected error: {e}", "red"))
        sys.exit(1)

if __name__ == "__main__":
    main()

# coin_toss.go
/**
 * 🪙 Coin Toss – Advanced Flip Series Analyzer (Go Edition)
 * Features: biased probability, streaks, ASCII chart, export, colorful CLI
 */

package main

import (
	"bufio"
	"encoding/json"
	"fmt"
	"math/rand"
	"os"
	"strconv"
	"strings"
	"time"
)

// ─── Colors ──────────────────────────────────────────────────────────────────

const (
	reset  = "\x1b[0m"
	bright = "\x1b[1m"
	dim    = "\x1b[2m"
	red    = "\x1b[31m"
	green  = "\x1b[32m"
	yellow = "\x1b[33m"
	blue   = "\x1b[34m"
	magenta = "\x1b[35m"
	cyan   = "\x1b[36m"
)

func c(str, color string) string {
	return color + str + reset
}

// ─── Core Logic ──────────────────────────────────────────────────────────────

type CoinToss struct {
	prob float64
}

func NewCoinToss(prob float64) *CoinToss {
	return &CoinToss{prob: prob}
}

func (ct *CoinToss) Flip() string {
	if rand.Float64() < ct.prob {
		return "H"
	}
	return "T"
}

func (ct *CoinToss) FlipSeries(n int) []string {
	series := make([]string, n)
	for i := 0; i < n; i++ {
		series[i] = ct.Flip()
	}
	return series
}

// ─── Statistics ─────────────────────────────────────────────────────────────

type Stats struct {
	Heads               int     `json:"heads"`
	Tails               int     `json:"tails"`
	Total               int     `json:"total"`
	HeadsPercent        float64 `json:"headsPercent"`
	TailsPercent        float64 `json:"tailsPercent"`
	LongestRunHeads     int     `json:"longestRunHeads"`
	LongestRunTails     int     `json:"longestRunTails"`
	LongestStreakHeads  int     `json:"longestStreakHeads"`
	LongestStreakTails  int     `json:"longestStreakTails"`
	Runs                int     `json:"runs"`
}

func analyzeSeries(series []string) Stats {
	if len(series) == 0 {
		return Stats{}
	}
	heads := 0
	for _, s := range series {
		if s == "H" {
			heads++
		}
	}
	total := len(series)
	tails := total - heads
	longestRunHeads, longestRunTails := 0, 0
	currentRun := 1
	for i := 1; i < len(series); i++ {
		if series[i] == series[i-1] {
			currentRun++
		} else {
			if series[i-1] == "H" {
				if currentRun > longestRunHeads {
					longestRunHeads = currentRun
				}
			} else {
				if currentRun > longestRunTails {
					longestRunTails = currentRun
				}
			}
			currentRun = 1
		}
	}
	if len(series) > 0 {
		if series[len(series)-1] == "H" {
			if currentRun > longestRunHeads {
				longestRunHeads = currentRun
			}
		} else {
			if currentRun > longestRunTails {
				longestRunTails = currentRun
			}
		}
	}
	runs := 1
	for i := 1; i < len(series); i++ {
		if series[i] != series[i-1] {
			runs++
		}
	}
	return Stats{
		Heads:              heads,
		Tails:              tails,
		Total:              total,
		HeadsPercent:       float64(heads) / float64(total) * 100,
		TailsPercent:       float64(tails) / float64(total) * 100,
		LongestRunHeads:    longestRunHeads,
		LongestRunTails:    longestRunTails,
		LongestStreakHeads: longestRunHeads,
		LongestStreakTails: longestRunTails,
		Runs:               runs,
	}
}

func drawAsciiChart(stats Stats, width int) string {
	if width <= 0 {
		width = 40
	}
	h := stats.HeadsPercent
	t := stats.TailsPercent
	hBar := int(h / 100 * float64(width))
	tBar := int(t / 100 * float64(width))
	line := fmt.Sprintf("H %s%s %.1f%%\n",
		c(strings.Repeat("█", hBar), green),
		strings.Repeat(" ", width-hBar),
		h)
	line += fmt.Sprintf("T %s%s %.1f%%",
		c(strings.Repeat("█", tBar), red),
		strings.Repeat(" ", width-tBar),
		t)
	return line
}

// ─── Export ──────────────────────────────────────────────────────────────────

type ExportData struct {
	Flips     []string `json:"flips"`
	Stats     Stats    `json:"stats"`
	Timestamp string   `json:"timestamp"`
}

func exportResults(series []string, stats Stats, filename string) {
	if filename == "" {
		ts := time.Now().Format("20060102_150405")
		filename = fmt.Sprintf("coin_toss_%s.json", ts)
	}
	data := ExportData{
		Flips:     series,
		Stats:     stats,
		Timestamp: time.Now().Format(time.RFC3339),
	}
	raw, _ := json.MarshalIndent(data, "", "  ")
	os.WriteFile(filename, raw, 0644)
	fmt.Printf("%s\n", c(fmt.Sprintf("✅ Results exported to %s", filename), green))
}

// ─── Main App ──────────────────────────────────────────────────────────────

type CoinApp struct {
	reader      *bufio.Reader
	prob        float64
	toss        *CoinToss
	lastSeries  []string
	lastStats   Stats
}

func NewCoinApp() *CoinApp {
	rand.Seed(time.Now().UnixNano())
	return &CoinApp{
		reader: bufio.NewReader(os.Stdin),
		prob:   0.5,
		toss:   NewCoinToss(0.5),
	}
}

func (app *CoinApp) ask(prompt string) string {
	fmt.Print(prompt)
	line, _ := app.reader.ReadString('\n')
	return strings.TrimSpace(line)
}

func (app *CoinApp) askInt(prompt string, def int) int {
	for {
		ans := app.ask(prompt)
		if ans == "" {
			return def
		}
		if n, err := strconv.Atoi(ans); err == nil {
			return n
		}
		fmt.Println(c("Please enter a number.", yellow))
	}
}

func (app *CoinApp) askFloat(prompt string, def float64) float64 {
	for {
		ans := app.ask(prompt)
		if ans == "" {
			return def
		}
		if n, err := strconv.ParseFloat(ans, 64); err == nil {
			return n
		}
		fmt.Println(c("Please enter a number.", yellow))
	}
}

func (app *CoinApp) showMenu() {
	fmt.Println("\n" + c(strings.Repeat("═", 50), cyan))
	fmt.Println(c("🪙 COIN TOSS ANALYZER", bright+cyan))
	fmt.Println(c(strings.Repeat("═", 50), cyan))
	fmt.Printf("  Probability of Heads: %.2f\n", app.prob)
	fmt.Println(c(strings.Repeat("═", 50), cyan))
	fmt.Println("  1. 🪙 Single Flip")
	fmt.Println("  2. 📊 Series of Flips")
	fmt.Println("  3. 📈 Show Statistics")
	fmt.Println("  4. 🎯 Set Probability")
	fmt.Println("  5. 💾 Export Results")
	fmt.Println("  6. 🔁 Continuous Flip Mode")
	fmt.Println("  0. 🚪 Exit")
	fmt.Println(c(strings.Repeat("═", 50), cyan))
}

func (app *CoinApp) singleFlip() {
	res := app.toss.Flip()
	color := green
	if res == "T" {
		color = red
	}
	fmt.Printf("\n🪙 Result: %s (Heads=%.2f)\n", c(res, color), app.prob)
	app.lastSeries = append(app.lastSeries, res)
	app.lastStats = analyzeSeries(app.lastSeries)
}

func (app *CoinApp) seriesFlips() {
	n := app.askInt("Number of flips (default 10): ", 10)
	if n <= 0 {
		fmt.Println(c("Number must be positive.", red))
		return
	}
	fmt.Printf(c("Flipping %d times...\n", dim), n)
	series := app.toss.FlipSeries(n)
	app.lastSeries = series
	app.lastStats = analyzeSeries(series)
	app.showStats()
}

func (app *CoinApp) showStats() {
	if app.lastStats.Total == 0 {
		fmt.Println(c("No data yet. Do some flips!", yellow))
		return
	}
	s := app.lastStats
	fmt.Println("\n📊 STATISTICS")
	fmt.Println(c(strings.Repeat("─", 30), dim))
	fmt.Printf("  Total Flips: %d\n", s.Total)
	fmt.Printf("  Heads: %d (%.1f%%)\n", s.Heads, s.HeadsPercent)
	fmt.Printf("  Tails: %d (%.1f%%)\n", s.Tails, s.TailsPercent)
	fmt.Printf("  Longest Run (H): %d\n", s.LongestRunHeads)
	fmt.Printf("  Longest Run (T): %d\n", s.LongestRunTails)
	fmt.Printf("  Number of Runs: %d\n", s.Runs)
	fmt.Println("\n📊 Distribution:")
	fmt.Println(drawAsciiChart(s, 40))
}

func (app *CoinApp) setProbability() {
	p := app.askFloat("Probability of Heads (0.0–1.0): ", 0.5)
	if p >= 0 && p <= 1 {
		app.prob = p
		app.toss = NewCoinToss(p)
		fmt.Printf("%s\n", c(fmt.Sprintf("✅ Probability set to %.2f", p), green))
	} else {
		fmt.Println(c("Invalid probability.", red))
	}
}

func (app *CoinApp) exportResults() {
	if app.lastStats.Total == 0 {
		fmt.Println(c("No data to export.", yellow))
		return
	}
	filename := app.ask("Filename (leave empty for auto): ")
	exportResults(app.lastSeries, app.lastStats, filename)
}

func (app *CoinApp) continuousFlip() {
	fmt.Printf("%s\n", c("\n🔁 Continuous Flip Mode (press Ctrl+C to stop)", cyan))
	count := 0
	heads := 0
	tails := 0
	series := []string{}
	for {
		res := app.toss.Flip()
		series = append(series, res)
		if res == "H" {
			heads++
		} else {
			tails++
		}
		count++
		fmt.Printf("\r %d flips: H=%d (%.1f%%) T=%d (%.1f%%)", count, heads, float64(heads)/float64(count)*100, tails, float64(tails)/float64(count)*100)
	}
}

func (app *CoinApp) run() {
	fmt.Print("\033[H\033[2J")
	fmt.Printf("%s\n", c("\n🪙 Coin Toss – Advanced Flip Analyzer", bright+cyan))
	fmt.Printf("%s\n", c("Flip, analyze, and master probability!", dim))

	for {
		app.showMenu()
		choice := app.ask("Your choice: ")
		switch choice {
		case "1":
			app.singleFlip()
		case "2":
			app.seriesFlips()
		case "3":
			app.showStats()
		case "4":
			app.setProbability()
		case "5":
			app.exportResults()
		case "6":
			app.continuousFlip()
			// After continuous mode, we won't return due to infinite loop, but we can handle exit
		case "0":
			fmt.Printf("%s\n", c("👋 Goodbye!", cyan))
			return
		default:
			fmt.Println(c("❌ Invalid choice.", red))
		}
		if choice != "0" && choice != "6" {
			fmt.Print("\nPress Enter to continue...")
			app.reader.ReadString('\n')
		}
	}
}

func main() {
	app := NewCoinApp()
	app.run()
}

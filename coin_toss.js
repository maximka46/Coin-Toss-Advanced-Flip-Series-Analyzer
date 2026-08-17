# coin_toss.js
/**
 * 🪙 Coin Toss – Advanced Flip Series Analyzer (Node.js Edition)
 * Features: biased probability, streaks, ASCII chart, export, colorful CLI
 */

const fs = require('fs');
const path = require('path');
const os = require('os');
const readline = require('readline');

// ─── Colors ──────────────────────────────────────────────────────────────────

const colors = {
  reset: '\x1b[0m',
  bright: '\x1b[1m',
  dim: '\x1b[2m',
  red: '\x1b[31m',
  green: '\x1b[32m',
  yellow: '\x1b[33m',
  blue: '\x1b[34m',
  magenta: '\x1b[35m',
  cyan: '\x1b[36m',
};

const c = (str, color) => `${color}${str}${colors.reset}`;

// ─── Core Logic ──────────────────────────────────────────────────────────────

class CoinToss {
  constructor(prob = 0.5) {
    this.prob = prob;
  }
  flip() {
    return Math.random() < this.prob ? 'H' : 'T';
  }
  flipSeries(n) {
    const arr = [];
    for (let i = 0; i < n; i++) arr.push(this.flip());
    return arr;
  }
}

// ─── Statistics ─────────────────────────────────────────────────────────────

function analyzeSeries(series) {
  if (!series || series.length === 0) return {};
  const heads = series.filter(s => s === 'H').length;
  const total = series.length;
  const tails = total - heads;
  let longestRunHeads = 0, longestRunTails = 0;
  let currentRun = 1;
  for (let i = 1; i < series.length; i++) {
    if (series[i] === series[i-1]) currentRun++;
    else {
      if (series[i-1] === 'H') longestRunHeads = Math.max(longestRunHeads, currentRun);
      else longestRunTails = Math.max(longestRunTails, currentRun);
      currentRun = 1;
    }
  }
  if (series.length > 0) {
    if (series[series.length-1] === 'H') longestRunHeads = Math.max(longestRunHeads, currentRun);
    else longestRunTails = Math.max(longestRunTails, currentRun);
  }
  let runs = 1;
  for (let i = 1; i < series.length; i++) {
    if (series[i] !== series[i-1]) runs++;
  }
  return {
    heads,
    tails,
    total,
    headsPercent: total ? heads / total * 100 : 0,
    tailsPercent: total ? tails / total * 100 : 0,
    longestRunHeads,
    longestRunTails,
    longestStreakHeads: longestRunHeads,
    longestStreakTails: longestRunTails,
    runs
  };
}

function drawAsciiChart(stats, width = 40) {
  const h = stats.headsPercent || 0;
  const t = stats.tailsPercent || 0;
  const hBar = Math.floor(h / 100 * width);
  const tBar = Math.floor(t / 100 * width);
  let line = `H ${c('█'.repeat(hBar), colors.green)}${' '.repeat(width - hBar)} ${h.toFixed(1)}%\n`;
  line += `T ${c('█'.repeat(tBar), colors.red)}${' '.repeat(width - tBar)} ${t.toFixed(1)}%`;
  return line;
}

// ─── Export ──────────────────────────────────────────────────────────────────

function exportResults(series, stats, filename = null) {
  if (!filename) {
    const ts = new Date().toISOString().replace(/[:.]/g, '').slice(0,14);
    filename = `coin_toss_${ts}.json`;
  }
  const data = {
    flips: series,
    stats,
    timestamp: new Date().toISOString()
  };
  fs.writeFileSync(filename, JSON.stringify(data, null, 2));
  console.log(c(`✅ Results exported to ${filename}`, colors.green));
}

// ─── Main App ──────────────────────────────────────────────────────────────

class CoinApp {
  constructor() {
    this.rl = readline.createInterface({ input: process.stdin, output: process.stdout });
    this.prob = 0.5;
    this.toss = new CoinToss(this.prob);
    this.lastSeries = [];
    this.lastStats = {};
  }

  _ask(prompt) {
    return new Promise(resolve => this.rl.question(prompt, resolve));
  }

  _askInt(prompt, def) {
    return this._ask(prompt).then(ans => {
      const n = parseInt(ans.trim());
      return isNaN(n) ? def : n;
    });
  }

  _askFloat(prompt, def) {
    return this._ask(prompt).then(ans => {
      const n = parseFloat(ans.trim());
      return isNaN(n) ? def : n;
    });
  }

  async showMenu() {
    console.log('\n' + c('═'.repeat(50), colors.cyan));
    console.log(c('🪙 COIN TOSS ANALYZER', colors.bright + colors.cyan));
    console.log(c('═'.repeat(50), colors.cyan));
    console.log(`  Probability of Heads: ${this.prob.toFixed(2)}`);
    console.log(c('═'.repeat(50), colors.cyan));
    console.log('  1. 🪙 Single Flip');
    console.log('  2. 📊 Series of Flips');
    console.log('  3. 📈 Show Statistics');
    console.log('  4. 🎯 Set Probability');
    console.log('  5. 💾 Export Results');
    console.log('  6. 🔁 Continuous Flip Mode');
    console.log('  0. 🚪 Exit');
    console.log(c('═'.repeat(50), colors.cyan));
  }

  async singleFlip() {
    const res = this.toss.flip();
    const color = res === 'H' ? colors.green : colors.red;
    console.log(`\n🪙 Result: ${c(res, color)} (Heads=${this.prob.toFixed(2)})`);
    this.lastSeries.push(res);
    this.lastStats = analyzeSeries(this.lastSeries);
  }

  async seriesFlips() {
    const n = await this._askInt('Number of flips (default 10): ', 10);
    if (n <= 0) { console.log(c('Number must be positive.', colors.red)); return; }
    console.log(c(`Flipping ${n} times...`, colors.dim));
    const series = this.toss.flipSeries(n);
    this.lastSeries = series;
    this.lastStats = analyzeSeries(series);
    this.showStats();
  }

  showStats() {
    if (!this.lastStats || !this.lastStats.total) {
      console.log(c('No data yet. Do some flips!', colors.yellow));
      return;
    }
    const s = this.lastStats;
    console.log('\n📊 STATISTICS');
    console.log(c('─'.repeat(30), colors.dim));
    console.log(`  Total Flips: ${s.total}`);
    console.log(`  Heads: ${s.heads} (${s.headsPercent.toFixed(1)}%)`);
    console.log(`  Tails: ${s.tails} (${s.tailsPercent.toFixed(1)}%)`);
    console.log(`  Longest Run (H): ${s.longestRunHeads}`);
    console.log(`  Longest Run (T): ${s.longestRunTails}`);
    console.log(`  Number of Runs: ${s.runs}`);
    console.log('\n📊 Distribution:');
    console.log(drawAsciiChart(s));
  }

  async setProbability() {
    const p = await this._askFloat('Probability of Heads (0.0–1.0): ', 0.5);
    if (p >= 0 && p <= 1) {
      this.prob = p;
      this.toss = new CoinToss(this.prob);
      console.log(c(`✅ Probability set to ${p.toFixed(2)}`, colors.green));
    } else {
      console.log(c('Invalid probability.', colors.red));
    }
  }

  async exportResults() {
    if (!this.lastStats || !this.lastStats.total) {
      console.log(c('No data to export.', colors.yellow));
      return;
    }
    const filename = await this._ask('Filename (leave empty for auto): ');
    exportResults(this.lastSeries, this.lastStats, filename.trim() || null);
  }

  async continuousFlip() {
    console.log(c('\n🔁 Continuous Flip Mode (press Ctrl+C to stop)', colors.cyan));
    let count = 0, heads = 0, tails = 0;
    const series = [];
    const handler = () => {
      console.log('\n');
      this.lastSeries = series;
      this.lastStats = analyzeSeries(series);
      this.showStats();
      process.exit(0);
    };
    process.on('SIGINT', handler);
    while (true) {
      const res = this.toss.flip();
      series.push(res);
      if (res === 'H') heads++; else tails++;
      count++;
      process.stdout.write(`\r ${count} flips: H=${heads} (${(heads/count*100).toFixed(1)}%) T=${tails} (${(tails/count*100).toFixed(1)}%)`);
    }
    // The loop never exits; SIGINT triggers handler
  }

  async run() {
    console.clear();
    console.log(c('\n🪙 Coin Toss – Advanced Flip Analyzer', colors.bright + colors.cyan));
    console.log(c('Flip, analyze, and master probability!', colors.dim));

    while (true) {
      await this.showMenu();
      const choice = await this._ask('Your choice: ');
      switch (choice.trim()) {
        case '1': await this.singleFlip(); break;
        case '2': await this.seriesFlips(); break;
        case '3': this.showStats(); break;
        case '4': await this.setProbability(); break;
        case '5': await this.exportResults(); break;
        case '6': await this.continuousFlip(); break;
        case '0':
          console.log(c('👋 Goodbye!', colors.cyan));
          this.rl.close();
          return;
        default:
          console.log(c('❌ Invalid choice.', colors.red));
      }
      if (choice !== '0' && choice !== '6') {
        console.log('\nPress Enter to continue...');
        await this._ask('');
      }
    }
  }
}

// ─── Main ────────────────────────────────────────────────────────────────────

const main = async () => {
  try {
    const app = new CoinApp();
    await app.run();
  } catch (e) {
    console.error(c(`❌ Unexpected error: ${e.message}`, colors.red));
    process.exit(1);
  }
};

main();

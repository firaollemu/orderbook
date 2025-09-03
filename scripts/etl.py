#!/usr/bin/env python3
# Minimal yfinance ETL → data/raw/<TICKER>_<INTERVAL>.csv
import argparse
from pathlib import Path
import pandas as pd
import yfinance as yf

ALLOWED = {"1m","2m","5m","15m","30m","60m","90m","1h","1d","5d","1wk","1mo","3mo"}

def is_intraday(iv: str) -> bool:
    return iv.endswith("m") or iv in {"60m","90m","1h"}

def fetch(ticker: str, interval: str, start: str | None, end: str | None, days: int, auto_adjust: bool) -> pd.DataFrame:
    kwargs = dict(tickers=ticker, interval=interval, auto_adjust=auto_adjust,
                  progress=False, threads=False, group_by="column")
    if is_intraday(interval):
        # Intraday: Yahoo only gives recent history (e.g., 1m ≈ 7d). Use period.
        kwargs["period"] = f"{max(1, days)}d"
        kwargs.pop("auto_adjust", None)  # keep as-is; arg still ok though
    else:
        if start: kwargs["start"] = start
        if end:   kwargs["end"] = end
    return yf.download(**kwargs)

def normalize(df: pd.DataFrame) -> pd.DataFrame:
    if df is None or df.empty:
        return pd.DataFrame(columns=["ts","open","high","low","close","volume"])
    # Ensure single-level columns and canonical names
    if isinstance(df.columns, pd.MultiIndex):
        df = df.droplevel(-1, axis=1)  # yfinance sometimes returns ('Open',) etc.
    cols = {str(c).strip().lower(): c for c in df.columns}
    need = ["open","high","low","close","volume"]
    missing = [n for n in need if n not in cols]
    if missing:
        raise ValueError(f"Missing columns from yfinance: {missing}. Got: {list(df.columns)}")
    core = df[[cols["open"], cols["high"], cols["low"], cols["close"], cols["volume"]]].copy()
    core.columns = ["open","high","low","close","volume"]
    # Index → UTC epoch seconds
    ts = (pd.to_datetime(df.index, utc=True).astype("int64") // 10**9).astype("int64")
    out = pd.DataFrame({"ts": ts.values})
    for c in ["open","high","low","close"]: out[c] = core[c].astype("float64").values
    out["volume"] = pd.to_numeric(core["volume"], errors="coerce").fillna(0).astype("int64").values
    out.dropna(inplace=True)
    out.drop_duplicates("ts", keep="last", inplace=True)
    out.sort_values("ts", inplace=True)
    return out.reset_index(drop=True)

def main():
    ap = argparse.ArgumentParser(description="Minimal yfinance → single CSV")
    ap.add_argument("--ticker", required=True, help="e.g., SPY")
    ap.add_argument("--interval", required=True, choices=sorted(ALLOWED), help="e.g., 1m or 1d")
    ap.add_argument("--start", default=None, help="YYYY-MM-DD (daily+ only)")
    ap.add_argument("--end", default=None, help="YYYY-MM-DD (daily+ only)")
    ap.add_argument("--days", type=int, default=7, help="Intraday lookback days (default 7)")
    ap.add_argument("--auto-adjust", action="store_true", help="Adjusted OHLC (splits/dividends)")
    args = ap.parse_args()

    df = fetch(args.ticker, args.interval, args.start, args.end, args.days, args.auto_adjust)
    out = normalize(df)
    outdir = Path("data/raw"); outdir.mkdir(parents=True, exist_ok=True)
    path = outdir / f"{args.ticker.upper()}_{args.interval}.csv"
    out.to_csv(path, index=False)
    print(f"[OK] {len(out):,} rows → {path}")

if __name__ == "__main__":
    main()

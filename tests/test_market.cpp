#!/usr/bin/env python3
# Minimal yfinance ETL (Py3.9) → data/raw/<TICKER>_<INTERVAL>.csv
import argparse
from pathlib import Path
import pandas as pd
import yfinance as yf

ALLOWED = {"1m","2m","5m","15m","30m","60m","90m","1h","1d","5d","1wk","1mo","3mo"}

def is_intraday(iv):
    return iv.endswith("m") or iv in {"60m","90m","1h"}

def fetch(ticker, interval, start, end, days, auto_adjust):
    # Use period for intraday (Yahoo limit), start/end for daily+.
    kwargs = dict(tickers=ticker, interval=interval, auto_adjust=auto_adjust,
                  progress=False, threads=False, group_by="column")
    if is_intraday(interval):
        kwargs["period"] = f"{max(1, int(days))}d"  # ignore start/end for intraday
    else:
        if start: kwargs["start"] = start
        if end:   kwargs["end"] = end
    return yf.download(**kwargs)

def normalize(df):
    if df is None or df.empty:
        return pd.DataFrame(columns=["ts","open","high","low","close","volume"])

    out = df.copy()

    # Flatten to single-level, lowercase column names
    if isinstance(out.columns, pd.MultiIndex):
        out.columns = [str(t[-1]).strip().lower() for t in out.columns]
    else:
        out.columns = [str(c).strip().lower() for c in out.columns]

    need = ["open","high","low","close","volume"]
    for n in need:
        if n not in out.columns:
            raise ValueError("Missing columns from yfinance. Got: %r" % list(out.columns))

    core = out[need].copy()
    # index -> UTC epoch seconds
    ts = (pd.to_datetime(out.index, utc=True).astype("int64") // 10**9).astype("int64")

    res = pd.DataFrame({"ts": ts.values})
    for c in ["open","high","low","close"]:
        res[c] = core[c].astype("float64").values
    res["volume"] = pd.to_numeric(core["volume"], errors="coerce").fillna(0).astype("int64").values

    res.dropna(inplace=True)
    res.drop_duplicates("ts", keep="last", inplace=True)
    res.sort_values("ts", inplace=True)
    return res.reset_index(drop=True)

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

    Path("data/raw").mkdir(parents=True, exist_ok=True)
    path = Path("data/raw") / f"{args.ticker.upper()}_{args.interval}.csv"
    out.to_csv(path, index=False)
    print(f"[OK] {len(out):,} rows → {path}")

if __name__ == "__main__":
    main()

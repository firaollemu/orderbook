import argparse
from pathlib import Path
import pandas as pd
import numpy as np


def main():
    ap = argparse.ArgumentParser(description="Build mid/vol/mom/inventory -> data/processed/features.csv")
    ap.add_argument("--bars", required=True, help="e.g., data/raw/SPY_1m.csv")
    ap.add_argument("--fills", default=None, help="optional fills CSV for inventory")
    ap.add_argument("--vol-win", type=int, default=60, help="rolling vol window (bars)")
    ap.add_argument("--mom-win", type=int, default=5, help="momentum window (bars)")  # <-- add this
    ap.add_argument("--out", default="data/processed/features.csv")
    args = ap.parse_args()


    # bars contain [open, highest, low, close, volume]
    bars = pd.read_csv(args.bars)
    bars = pd.read_csv(args.bars)
    bars.columns = [c.lower() for c in bars.columns]
    need = {"ts","open","high","low","close","volume"}
    if not need.issubset(set(bars.columns)):
        raise SystemExit(f"[ERR] bars missing: {sorted(need - set(bars.columns))}")
    bars = bars.sort_values("ts").drop_duplicates("ts").reset_index(drop=True)


    mid = (bars["high"] + bars["low"]) / 2.0
    logret = np.log(mid).diff().fillna(0.0)
    vol = logret.rolling(args.vol_win).std().fillna(0.0)
    mom = logret.rolling(args.mom_win).sum().fillna(0.0)


    features = pd.DataFrame({
        "ts": bars["ts"].astype("int64"),
        "mid": mid.astype("float64"),
        "ret_1": logret.astype("float64"),
        "vol": vol.astype("float64"),
        "mom": mom.astype("float64"),
        "volume": pd.to_numeric(bars["volume"], errors="coerce").fillna(0).astype("int64"),
    })

    # optional inventory from fills
    inventory = None
    if args.fills and Path(args.fills).exists():
        f = pd.read_csv(args.fills)
        f.columns = [c.lower() for c in f.columns]
        if "ts" not in f.columns or "qty" not in f.columns:
            raise SystemExit(f"[ERR] fills must have ts and qty columns: {list(f.columns)}")
        qty = pd.to_numeric(f["qty"], errors="coerce").fillna(0.0)
        if "side" in f.columns:
            s = f["side"].astype(str).str.lower()
            sign = np.where(s.isin(["buy","+1","b","1"]), 1, np.where(s.isin(["sell","-1","s"]), -1, 0))
            signed = sign * qty.values
        else:
            signed = qty.values  # assume already signed
        inv = pd.DataFrame({"ts": pd.to_numeric(f["ts"], errors="coerce"), "signed": signed}).dropna()
        inv = inv.groupby("ts", as_index=True)["signed"].sum().sort_index().cumsum()
        inventory = inv.reindex(feats["ts"]).ffill().fillna(0.0).values

    features["inventory"] = inventory if inventory is not None else 0.0

    out_path = Path(args.out)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    features.to_csv(out_path, index=False)
    print(f"[OK] wrote {len(features):,} rows → {out_path}")

if __name__ == "__main__":
    main()

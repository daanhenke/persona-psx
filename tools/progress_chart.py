"""Draw the matched-bytes history as a stacked area chart, one band per target.

Written as a plain SVG so the repo keeps no plotting dependency and the output
is byte-stable between runs. tools/progress.py --report calls build().

History rows recorded before per-target totals existed carry only a grand total.
Those are reconstructed from the manifest's order - it is append-ordered, so the
first N entries are what was matched when N functions were - and then scaled so
each stack still sums to the total that was actually measured at the time.
"""
import datetime
import os
import re

FUNCSIZE = re.compile(r"type:func size:0x([0-9A-Fa-f]+)")

# Load order of the game: the two boot screens, the movie player, then the
# three big overlays, then the minigames. Keeping the legend in this order
# makes the chart read like the disc rather than like a leaderboard.
TARGETS = ["main", "atlus", "open", "movie", "end",
           "dng", "btlp", "s2d", "adv", "casino", "name"]

COLOURS = {
    "main":   "#4e79a7",
    "atlus":  "#f28e2b",
    "open":   "#e15759",
    "movie":  "#76b7b2",
    "end":    "#59a14f",
    "dng":    "#edc948",
    "btlp":   "#b07aa1",
    "s2d":    "#ff9da7",
    "adv":    "#9c755f",
    "casino": "#8cd17d",
    "name":   "#86bcb6",
}

W, H = 1080, 520
L, R, T, B = 70, 290, 58, 52          # margins: left, right, top, bottom
PW, PH = W - L - R, H - T - B


def func_sizes(root, game):
    """(target, symbol) -> byte size, from the generated symbol files."""
    sizes = {}
    for t in TARGETS:
        path = os.path.join(root, "config", game, "%s.symbols.txt" % t)
        if not os.path.exists(path):
            continue
        for line in open(path):
            m = FUNCSIZE.search(line)
            if m:
                sizes[(t, line.split("=")[0].strip())] = int(m.group(1), 16)
    return sizes


def manifest_order(root, game):
    entries = []
    path = os.path.join(root, "config", game, "decomp.txt")
    for line in open(path):
        line = line.split("//")[0].strip()
        if not line:
            continue
        parts = line.split()
        if len(parts) >= 3:
            entries.append((parts[0], parts[1]))
    return entries


def fill_targets(hist, root, game):
    """Give every row a per-target split, reconstructing the old ones.

    Returns the number of rows that had to be reconstructed.
    """
    sizes = func_sizes(root, game)
    order = manifest_order(root, game)
    n_recon = 0
    for h in hist:
        if h.get("targets"):
            continue
        n_recon += 1
        got = {}
        for target, symbol in order[:h["matched_funcs"]]:
            got[target] = got.get(target, 0) + sizes.get((target, symbol), 0)
        total = sum(got.values())
        want = h["matched_bytes"]
        if total and total != want:
            # The manifest was reordered at some point (a source moved), so the
            # prefix is close but not exact. Keep the measured grand total.
            for k in got:
                got[k] = int(round(got[k] * want / float(total)))
        h["targets"] = got
    return n_recon


def _x(i, n):
    return L + (PW * i / float(n - 1) if n > 1 else PW / 2.0)


def _esc(s):
    return (str(s).replace("&", "&amp;").replace("<", "&lt;")
            .replace(">", "&gt;"))


def _nice_step(top, want=6):
    """A round gridline step giving roughly `want` lines below `top`."""
    raw = top / float(want)
    mag = 10 ** int(max(0, len("%d" % int(raw)) - 1))
    for mult in (1, 2, 2.5, 5, 10):
        if mag * mult >= raw:
            return mag * mult
    return mag * 10


def build(hist, root, game):
    """Return the chart as an SVG string."""
    n_recon = fill_targets(hist, root, game)
    n = len(hist)
    if n == 0:
        raise SystemExit("no history recorded yet - run with --record first")

    # Only band the targets that have ever been touched.
    live = [t for t in TARGETS
            if any(h["targets"].get(t) for h in hist)]
    top = max(h["matched_bytes"] for h in hist)
    step = _nice_step(top)
    ymax = step * (int(top / step) + 1)
    total_bytes = hist[-1]["total_bytes"]

    def y(v):
        return T + PH - PH * v / float(ymax)

    out = []
    add = out.append
    add('<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" '
        'viewBox="0 0 %d %d" font-family="Segoe UI, Helvetica, Arial, sans-serif">'
        % (W, H, W, H))
    add('<rect width="%d" height="%d" fill="#fbfaf7"/>' % (W, H))

    last = hist[-1]
    add('<text x="%d" y="28" font-size="17" font-weight="600" fill="#2b2b2b">'
        'Persona 1 (JP) - matching decompilation</text>' % L)
    add('<text x="%d" y="46" font-size="12" fill="#6b6b6b">'
        '%s of %s bytes matched (%.3f%%) in %d of %d functions</text>'
        % (L, "{:,}".format(last["matched_bytes"]),
           "{:,}".format(total_bytes), last["percent"],
           last["matched_funcs"], last["total_funcs"]))

    # Gridlines, left axis in bytes, right axis in percent of the whole game.
    v = 0
    while v <= ymax + 1:
        yy = y(v)
        add('<line x1="%d" y1="%.1f" x2="%d" y2="%.1f" stroke="#e3e0d8" '
            'stroke-width="1"/>' % (L, yy, L + PW, yy))
        add('<text x="%d" y="%.1f" font-size="10" fill="#8a857c" '
            'text-anchor="end">%s</text>'
            % (L - 8, yy + 3, "{:,}".format(int(v))))
        add('<text x="%d" y="%.1f" font-size="10" fill="#8a857c">%.1f%%</text>'
            % (L + PW + 8, yy + 3, 100.0 * v / total_bytes))
        v += step

    # Stacked areas, bottom band first.
    base = [0.0] * n
    for t in live:
        upper = [base[i] + hist[i]["targets"].get(t, 0) for i in range(n)]
        pts = ["%.1f,%.1f" % (_x(i, n), y(upper[i])) for i in range(n)]
        pts += ["%.1f,%.1f" % (_x(i, n), y(base[i]))
                for i in range(n - 1, -1, -1)]
        add('<polygon points="%s" fill="%s" fill-opacity="0.92" '
            'stroke="%s" stroke-width="0.6"/>'
            % (" ".join(pts), COLOURS[t], COLOURS[t]))
        base = upper

    # Axes.
    add('<line x1="%d" y1="%d" x2="%d" y2="%d" stroke="#b9b3a7"/>'
        % (L, T + PH, L + PW, T + PH))
    add('<line x1="%d" y1="%d" x2="%d" y2="%d" stroke="#b9b3a7"/>'
        % (L, T, L, T + PH))

    # Date ticks: enough to read, never so many they collide.
    stamps = [datetime.datetime.strptime(h["date"], "%Y-%m-%dT%H:%M:%S")
              for h in hist]
    same_day = stamps[0].date() == stamps[-1].date()
    every = max(1, int(round(n / 8.0)))
    for i in range(0, n, every):
        xx = _x(i, n)
        add('<line x1="%.1f" y1="%d" x2="%.1f" y2="%d" stroke="#b9b3a7"/>'
            % (xx, T + PH, xx, T + PH + 4))
        add('<text x="%.1f" y="%d" font-size="10" fill="#8a857c" '
            'text-anchor="middle">%s</text>'
            % (xx, T + PH + 17,
               stamps[i].strftime("%H:%M" if same_day else "%m-%d")))
    add('<text x="%.1f" y="%d" font-size="10" fill="#8a857c" '
        'text-anchor="middle">%s</text>'
        % (L + PW / 2.0, T + PH + 36,
           _esc(stamps[0].strftime("%Y-%m-%d")) if same_day else "date"))

    # Legend, in load order, with each target's own share of its own code.
    totals, counts = target_totals(root, game)
    matched_counts = manifest_counts(root, game)
    ly = T + 6
    add('<text x="%d" y="%d" font-size="10" fill="#8a857c" '
        'letter-spacing="0.5">TARGET   MATCHED  OF IT  FUNCTIONS</text>'
        % (L + PW + 46, ly))
    ly += 16
    for t in live:
        got = last["targets"].get(t, 0)
        own = totals.get(t, 0)
        add('<rect x="%d" y="%d" width="11" height="11" fill="%s" rx="2"/>'
            % (L + PW + 46, ly - 9, COLOURS[t]))
        add('<text x="%d" y="%d" font-size="11" fill="#3d3a35">%s</text>'
            % (L + PW + 63, ly, _esc(t)))
        add('<text x="%d" y="%d" font-size="11" fill="#3d3a35" '
            'text-anchor="end">%s</text>'
            % (L + PW + 137, ly, "{:,}".format(got)))
        add('<text x="%d" y="%d" font-size="11" fill="#8a857c" '
            'text-anchor="end">%.2f%%</text>'
            % (L + PW + 180, ly, 100.0 * got / own if own else 0.0))
        add('<text x="%d" y="%d" font-size="11" fill="#8a857c" '
            'text-anchor="end">%d/%d</text>'
            % (L + PW + 250, ly, matched_counts.get(t, 0), counts.get(t, 0)))
        ly += 17

    if n_recon:
        add('<text x="%d" y="%d" font-size="9.5" fill="#a49e93">'
            'The first %d samples predate per-target recording and are '
            'reconstructed from the order functions were added; their grand '
            'totals are as measured.</text>' % (L, H - 8, n_recon))
    add('</svg>')
    return "\n".join(out)


def target_totals(root, game):
    """(bytes of code, number of functions) in each target."""
    size, count = {}, {}
    for t in TARGETS:
        path = os.path.join(root, "config", game, "%s.symbols.txt" % t)
        if not os.path.exists(path):
            continue
        found = [m for m in (FUNCSIZE.search(l) for l in open(path)) if m]
        size[t] = sum(int(m.group(1), 16) for m in found)
        count[t] = len(found)
    return size, count


def manifest_counts(root, game):
    """How many functions each target has matched, from the manifest.

    Every row in decomp.txt is a function tools/progress.py has re-verified at
    100%, so counting rows per target says the same thing the run that drew the
    chart printed - no extra bookkeeping in the history for it.
    """
    out = {}
    for target, _ in manifest_order(root, game):
        out[target] = out.get(target, 0) + 1
    return out

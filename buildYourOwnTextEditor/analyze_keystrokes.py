#!/usr/bin/env python3
"""Baca file KILO_KEYLOG (format: delta_ms<TAB>keycode<TAB>is_backspace per
baris) terus kasih summary ritme ngetik.
Pakenya: python3 analyze_keystrokes.py <logfile>
"""
import sys
import statistics


def main():
    if len(sys.argv) != 2:
        print(f"usage: {sys.argv[0]} <keylog file>", file=sys.stderr)
        return 1

    deltas = []
    backspaces = 0
    with open(sys.argv[1]) as f:
        for line in f:
            parts = line.strip().split("\t")
            if len(parts) != 3:
                continue
            delta_ms, _keycode, is_backspace = parts
            deltas.append(float(delta_ms))
            backspaces += int(is_backspace)

    if not deltas:
        print("no keystrokes logged")
        return 0

    # delta pertama dibuang - itu jarak dari pas editor dibuka, bukan
    # jarak antar keystroke yang beneran
    rhythm = deltas[1:] if len(deltas) > 1 else deltas

    print(f"keystrokes: {len(deltas)}")
    print(f"backspaces: {backspaces} ({100 * backspaces / len(deltas):.1f}%)")
    if rhythm:
        print(f"inter-key delay: mean={statistics.mean(rhythm):.1f}ms "
              f"median={statistics.median(rhythm):.1f}ms "
              f"max={max(rhythm):.1f}ms")

        buckets = [0, 0, 0, 0, 0]
        edges = [100, 300, 800, 2000]
        labels = ["<100ms", "100-300ms", "300-800ms", "800-2000ms", ">2000ms"]
        for d in rhythm:
            placed = False
            for i, edge in enumerate(edges):
                if d < edge:
                    buckets[i] += 1
                    placed = True
                    break
            if not placed:
                buckets[-1] += 1

        print("\ndelay histogram (fast typing vs. thinking pauses):")
        for label, count in zip(labels, buckets):
            bar = "#" * int(40 * count / len(rhythm)) if rhythm else ""
            print(f"  {label:>12}: {bar} ({count})")

    return 0


if __name__ == "__main__":
    sys.exit(main())

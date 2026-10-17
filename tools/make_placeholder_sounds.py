#!/usr/bin/env python3
"""Generates the placeholder sound effects in assets/sounds/.

The sounds are simple synthesised tones and noise bursts, standing in until the
final audio is chosen. Run from the repository root:

    python3 tools/make_placeholder_sounds.py
"""

import math
import random
import struct
import wave
from pathlib import Path

RATE = 44100
OUT_DIR = Path(__file__).resolve().parent.parent / "assets" / "sounds"


def tone(start_hz, end_hz, seconds, shape="square", volume=0.5, decay=4.0):
    """A pitch sweep from start_hz to end_hz that fades out."""
    samples = []
    phase = 0.0
    count = int(RATE * seconds)
    for i in range(count):
        t = i / count
        freq = start_hz + (end_hz - start_hz) * t
        phase += freq / RATE
        cycle = phase % 1.0
        if shape == "square":
            value = 1.0 if cycle < 0.5 else -1.0
        elif shape == "saw":
            value = cycle * 2.0 - 1.0
        else:
            value = math.sin(phase * 2.0 * math.pi)
        samples.append(value * volume * math.exp(-decay * t))
    return samples


def noise(seconds, volume=0.5, decay=5.0, seed=1):
    """A burst of white noise that fades out."""
    rng = random.Random(seed)
    count = int(RATE * seconds)
    return [rng.uniform(-1.0, 1.0) * volume * math.exp(-decay * i / count) for i in range(count)]


def mix(*parts):
    length = max(len(part) for part in parts)
    return [sum(part[i] for part in parts if i < len(part)) for i in range(length)]


def join(*parts):
    return [sample for part in parts for sample in part]


def write(name, samples):
    OUT_DIR.mkdir(parents=True, exist_ok=True)
    path = OUT_DIR / f"{name}.wav"
    with wave.open(str(path), "wb") as file:
        file.setnchannels(1)
        file.setsampwidth(2)
        file.setframerate(RATE)
        clipped = (max(-1.0, min(1.0, sample)) for sample in samples)
        file.writeframes(b"".join(struct.pack("<h", int(sample * 32767)) for sample in clipped))
    print(path)


SOUNDS = {
    "shot_main_gun": lambda: tone(1400, 700, 0.05, "square", 0.25, 6.0),
    "shot_homing": lambda: mix(tone(300, 900, 0.18, "saw", 0.2, 3.0), noise(0.18, 0.1, 4.0, 2)),
    "shot_enemy": lambda: tone(500, 250, 0.08, "square", 0.2, 5.0),
    "hit": lambda: mix(noise(0.04, 0.35, 8.0, 3), tone(900, 600, 0.04, "sine", 0.2, 8.0)),
    "absorb": lambda: tone(600, 1400, 0.1, "sine", 0.35, 2.0),
    "enemy_death": lambda: mix(noise(0.35, 0.6, 5.0, 4), tone(200, 50, 0.35, "square", 0.25, 4.0)),
    "pickup": lambda: join(tone(660, 660, 0.06, "square", 0.25, 1.0),
                           tone(880, 880, 0.06, "square", 0.25, 1.0),
                           tone(1320, 1320, 0.1, "square", 0.25, 3.0)),
}


def main():
    for name, make in SOUNDS.items():
        write(name, make())


if __name__ == "__main__":
    main()

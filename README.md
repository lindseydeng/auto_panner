# Autopan

Autopan is a simple audio processing tool that applies **low-frequency oscillator (LFO)–driven panning** to a mono input file, producing a stereo output file with dynamic spatial movement.

---

## Features

- Supports input files with the following extensions:
  - `.wav`
  - `.aif`
  - `.aiff`
- Input file **must be mono**.
- Output is a **stereo audio file** with processed panning.
- User can control the following parameters:
  - **Width** – the stereo spread of the panning effect.
  - **Rate** – the speed of the LFO.
  - **Phase** – the phase offset of the LFO.
  - **Type** – the LFO waveform type (`sine`, etc.).

---

## Compilation (macOS M1)

To compile, use:

\```bash
gcc autopan.c breakpoints.c -o autopan -Iinclude -Llib -lsndfile
\```

---

## Usage

\```bash
./autopan <input_file> <output_file> <width> <rate> <phase> <type>
\```

### Example

\```bash
./autopan input.wav output.wav 1.0 0.5 0 sine
\```

---

## Acknowledgments

- Adapted from `sfpan.c` by Minglun Lee  
- Built with [libsndfile](https://github.com/libsndfile/libsndfile)  

---

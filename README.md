# FunkyFilter
An LFO-modulated band-pass filter effect plugin (VST3, AU, Standalone) built with JUCE 8.

Created as a Bachelor's thesis project at FER (Faculty of Electrical Engineering and Computing, University of Zagreb).

## Features
- **LFO-modulated band-pass filter** — cosine wavetable LFO sweeps frequency between user-defined minimum and maximum
- **Two modulation modes:**
  - *Free LFO* — modulation rate adjustable from 0.1–20 Hz
  - *Note-Synced* — rate calculated from BPM and note divisions (1, 1/2, 1/4, 1/8, 1/16)
- **Filter Q** — adjustable resonance from 0.1 to 10.0
- **Logarithmic frequency mapping** — natural-feeling sweep across the frequency spectrum
- **Real-time response curve** — 60 FPS visualization of the filter magnitude response with min/max frequency markers
- **Stereo processing** — independent left/right filter channels
- **State persistence** — parameter values saved/restored via `AudioProcessorValueTreeState`

## Building from Source

### Requirements
- **JUCE 8** — set the JUCE path in `FunkyFilter.jucer` (or adjust module paths in the build projects)
- **Visual Studio 2022** (Windows) or **Xcode** (macOS)
- **Windows SDK 10.0** (for VST3 builds on Windows)

### Steps
1. Open `FunkyFilter.jucer` in Projucer to regenerate build projects and `JuceLibraryCode/`.
2. Open `Builds/VisualStudio2022/FunkyFilter.sln` in VS 2022.
3. Select x64 configuration (Debug or Release) and build.

### Output
| Configuration | Artifact |
|---|---|
| VST3 | `FunkyFilter.vst3\Contents\x86_64-win\FunkyFilter.vst3` |
| Standalone | `FunkyFilter.exe` |
| AU | Builds on macOS only |

## Parameters
- **Mod Frequency** — `AudioParameterFloat` (0.1–20.0 Hz, step 0.1, default 1.0)
- **Filter Q** — `AudioParameterFloat` (0.1–10.0, step 0.05, default 1.0)
- **Minimum Frequency** — `AudioParameterFloat` (30–16000 Hz, step 1, default 200)
- **Maximum Frequency** — `AudioParameterFloat` (30–16000 Hz, step 1, default 5000)
- **Use Note Duration** — `AudioParameterBool` (default false)
- **BPM** — `AudioParameterFloat` (20–300, step 1, default 120)
- **Note Duration** — `AudioParameterChoice` (1, 1/2, 1/4, 1/8, 1/16)

## Credits
- **Framework:** JUCE
- **Thesis:** Bachelor's thesis, FER (University of Zagreb)

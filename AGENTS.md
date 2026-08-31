# AI Agent Instructions

This file helps AI coding agents understand the PCI-1716 data acquisition project and be immediately productive.

## Project Overview

C++ application suite for high-speed data acquisition (250 kHz) using Advantech PCI-1716 analog input card.

> **Note:** `README.md` does not currently exist in the workspace. If you need full documentation, consider creating it from the AGENTS.md content.

## Build Commands

**Preferred:** `build.bat` — interactive, prompts for configuration, architecture, and toolset.

**Manual:**
```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

**Outputs:**
- `build/application/data-logger/Release/data-logger.exe`
- `build/application/data-converter/Release/data-converter.exe`

## Running

**Interactive:** `run.bat` — menu-driven launcher with prompts and validation.

**Direct (data-logger):**
```bash
data-logger.exe --device PCI-1716,BID#0 --start-channel 0 --end-channel 7 --rate 100000 --input-mode unipolar --input-range 10V
```

See README for all command-line arguments.

## Key Architecture

- **Namespace:** `app`
- **Data Logger:** Three threads — main (control), callback (hardware interrupt → queue), writer (queue → binary file)
- **Callback:** `BDAQCALL` convention, high-priority Advantech thread; must not block
- **Queue:** `std::queue` + `std::mutex` + `std::condition_variable`
- **Data Format:** Binary file stores a `DataHeader` struct followed by raw `double` samples per channel (frame-based). Header fields: magic, version, samplingRate, startChannel, endChannel, channelCount, startTimeSeconds, endTimeSeconds
- **Converter:** Reads binary, computes timestamps from sampling rate, outputs CSV with Russian locale (comma decimal separator)

### Key Interfaces

| Interface | Purpose |
|-----------|---------|
| `IDataAcquisitionDevice` | `initialize()`, `configure()`, `start()`, `stop()`, `dispose()`, `setDataReadyCallback()` |
| `ILogger` | `error()`, `warning()`, `info()`, `debug()` |
| `DataReadyCallback` | `std::function<void(const std::vector<double>&)>` |
| `IDataReader` / `IDataWriter` | Binary/CSV converter interfaces |
| `ITimeFormatter` | Timestamp generation |

## Build Configuration

- **C++17** standard
- **MSVC static linking:** `/MT` (Release) or `/MTd` (Debug)
- **Target:** `library::DAQNavi` wraps Advantech SDK

## Windows-Only

Targets Windows due to Advantech DAQNavi SDK. No cross-platform builds.

## Common Pitfalls

1. **Callback must not block**: The `BDAQCALL` callback runs on a high-priority Advantech thread. Never perform I/O, heap allocations, or long operations inside it — only push data to the queue.

2. **CSV locale**: The converter outputs CSV with Russian locale (comma decimal separator). Ensure your parsing tools handle this correctly.

3. **Binary file format**: The binary file includes a `DataHeader` struct at the beginning. Don't treat the file as raw `double` samples only — read the header first.

4. **Static linking**: MSVC uses `/MT` or `/MTd` flags. If you encounter runtime DLL errors, verify the correct static libraries are linked.

5. **Device description**: The device string format is `PCI-1716,BID#0` (bus ID 0). Adjust the BID number if you have multiple cards.

## VS Code Build Tools

When working in VS Code with the CMake Tools extension:
- Use `Build_CMakeTools` for building instead of manual terminal commands
- Use `GetDiagnostics_CMakeTools` to check for CMake-related errors
- Use `ListBuildTargets_CMakeTools` to see available targets
- Use `RunCtest_CMakeTools` for running tests (if tests are configured)

## Tests

Tests are not currently configured in this project. To add tests, consider using Google Test or Catch2 with CMake's `add_test()` and `enable_testing()`.

## Next Customizations

Consider creating the following customizations to improve agent productivity:

- **`/create-skill build`** — Automate build tasks with preset configurations
- **`/create-skill run-acquisition`** — Common data-logger run commands with channel/rate presets
- **`/create-agent debug`** — Specialized agent for debugging hardware/device issues

Run `/chronicle improve` after sessions to iteratively refine these instructions based on real friction patterns.
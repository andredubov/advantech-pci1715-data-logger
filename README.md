# PCI-1715 Data Acquisition System

C++ application suite for high-speed data acquisition (up to 500 kHz) using the **Advantech PCI-1715** analog input card with galvanic isolation.

## Overview

The project consists of two main applications:

- **`data-logger`** — Real-time data acquisition with hardware-triggered sampling, writes binary files
- **`data-converter`** — Converts binary acquisition files to CSV format with timestamps

Both applications are built with C++17 and use the Advantech DAQNavi SDK for hardware control.

## Features

- **High-speed acquisition** — Up to 500 kHz sampling rate
- **Multi-channel support** — Up to 32 single-ended or 16 differential channels
- **Galvanic isolation** — 2500 VDC isolation for industrial safety
- **Real-time processing** — Callback-based data streaming with queue buffering
- **Binary storage** — Compact, fast binary format with header metadata
- **CSV export** — Converter generates CSV with Russian locale (comma decimal separator)
- **Interactive launcher** — `run.bat` provides a menu-driven interface

## Hardware Requirements

- **Device:** Advantech PCI-1715 / PCI-1715U
- **Interface:** PCI (5V)
- **SDK:** Advantech DAQNavi (included in `library/`)
- **Resolution:** 12-bit ADC
- **FIFO Buffer:** 1024 samples

## Build Instructions

### Interactive Build

Run the interactive build script:

```batch
build.bat
```

You'll be prompted to select:
- **Configuration:** Release or Debug
- **Architecture:** Win32 or x64
- **Toolset:** VS version (v140_xp, v141, v142, v143, ClangCL)

### Manual Build

```bash
mkdir build && cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Outputs

- `build/application/data-logger/Release/data-logger.exe`
- `build/application/data-converter/Release/data-converter.exe`

## Running the Applications

### Interactive Launcher (Recommended)

```batch
run.bat
```

Provides a menu-driven interface with prompts and validation for all parameters.

### Data Logger (Direct)

```bash
data-logger.exe --device PCI-1715,BID#0 --start-channel 0 --end-channel 31 --rate 100000 --input-mode unipolar --input-range 10V --output data.bin
```

#### Command-Line Arguments

| Argument | Description | Default |
|----------|-------------|---------|
| `--device` | Device description (e.g., `PCI-1715,BID#0`) | `DemoDevice,BID#0` |
| `--start-channel` | First channel (0-31) | `0` |
| `--end-channel` | Last channel (0-31) | `31` |
| `--rate` | Sampling rate in Hz (max 500000) | `500000` |
| `--samples-per-channel` | Buffer size per channel | `10240` |
| `--output` | Output binary file name | `daq_data.bin` |
| `--input-mode` | `bipolar` or `unipolar` | `unipolar` |
| `--input-range` | `10V`, `5V`, `2.5V` | `10V` |

### Data Converter

```bash
data-converter.exe --input data.bin --output data.csv
```

## Architecture

### UML Diagrams

#### Data Logger Class Diagram

```mermaid
classDiagram
    class AcquisitionManager {
        -shared_ptr~IDataAcquisitionDevice~ m_device
        -shared_ptr~DataProcessingEngine~ m_engine
        -shared_ptr~IDataWriter~ m_writer
        -shared_ptr~ILogger~ m_logger
        -shared_ptr~CommandLineOptions~ m_options
        -bool m_initialized
        -bool m_acquisitionStarted
        +initialize() bool
        +startAcquisition() bool
        +stopAcquisition()
        +waitForStop()
        +shutdown()
        -setupDeviceCallback()
        -printConfiguration()
    }

    class IDataAcquisitionDevice {
        <<interface>>
        +initialize(string) bool
        +configure(AcquisitionConfig) bool
        +start() bool
        +stop()
        +dispose()
        +setDataReadyCallback(DataReadyCallback)
    }

    class AdvantechDevice {
        -string m_deviceDescription
        -AcquisitionConfig m_config
        -double m_samplingRate
        -DataReadyCallback m_callback
        -void* m_deviceHandle
        +initialize(string) bool
        +configure(AcquisitionConfig) bool
        +start() bool
        +stop()
        +dispose()
        +setDataReadyCallback(DataReadyCallback)
        -setupHardware() bool
        -startAcquisitionThread() bool
    }

    class DataProcessingEngine {
        -queue~DataFrame~ m_queue
        -mutex m_mutex
        -condition_variable m_cv
        -thread m_writerThread
        -bool m_running
        -shared_ptr~IDataWriter~ m_writer
        -int m_channelCount
        +start(double, int, int)
        +stop()
        +pushDataFrame(DataFrame)
        -writerLoop()
    }

    class IDataWriter {
        <<interface>>
        +open(string) bool
        +writeHeader(DataHeader) bool
        +writeFrame(DataFrame) bool
        +close()
    }

    class BinaryFileWriter {
        -ofstream m_file
        -string m_filePath
        -bool m_isOpen
        -DataHeader m_header
        +open(string) bool
        +writeHeader(DataHeader) bool
        +writeFrame(DataFrame) bool
        +close()
        -writeSamples(vector~double~) bool
    }

    class ILogger {
        <<interface>>
        +error(string)
        +warning(string)
        +info(string)
        +debug(string)
    }

    class CommandLineOptions {
        -string m_deviceDescription
        -string m_outputFilePath
        -int m_startChannel
        -int m_endChannel
        -double m_samplingRate
        -int m_samplesPerChannel
        -string m_inputMode
        -string m_inputRange
        +parse(int, char**) state
        +get_help() string
        +get_device_description() string
        +get_output_file_path() string
        +get_start_channel() int
        +get_end_channel() int
        +get_channel_count() int
        +get_sampling_rate() double
        +get_samples_per_channel() int
        +get_input_mode() string
        +get_input_range() string
    }

    AcquisitionManager --> IDataAcquisitionDevice : uses
    AcquisitionManager --> DataProcessingEngine : uses
    AcquisitionManager --> IDataWriter : uses
    AcquisitionManager --> ILogger : uses
    AcquisitionManager --> CommandLineOptions : uses
    AdvantechDevice ..|> IDataAcquisitionDevice : implements
    DataProcessingEngine --> IDataWriter : uses
    BinaryFileWriter ..|> IDataWriter : implements
```

#### Data Converter Class Diagram

```mermaid
classDiagram
    class DataConverter {
        -shared_ptr~IDataReader~ m_reader
        -shared_ptr~IDataWriter~ m_writer
        -shared_ptr~ITimeFormatter~ m_timeFormatter
        -shared_ptr~IProgressReporter~ m_progress
        -shared_ptr~ILogger~ m_logger
        +convert(string, string) bool
        +setProgressCallback(function)
        -processFrames(size_t)
    }

    class IDataReader {
        <<interface>>
        +open(string) bool
        +readHeader(DataHeader) bool
        +readFrames(vector~DataFrame~, size_t) bool
        +getTotalFrames() size_t
        +isOpen() bool
        +close()
    }

    class BinaryReader {
        -ifstream m_file
        -string m_filePath
        -DataHeader m_header
        -size_t m_totalFrames
        -size_t m_currentFrame
        +open(string) bool
        +readHeader(DataHeader) bool
        +readFrames(vector~DataFrame~, size_t) bool
        +getTotalFrames() size_t
        +isOpen() bool
        +close()
        -readFrame(DataFrame) bool
    }

    class IDataWriter {
        <<interface>>
        +open(string) bool
        +writeHeader(DataHeader) bool
        +writeFrame(DataFrame) bool
        +close()
    }

    class CsvWriter {
        -ofstream m_file
        -string m_filePath
        -bool m_isOpen
        -locale m_locale
        -bool m_headerWritten
        +open(string) bool
        +writeHeader(DataHeader) bool
        +writeFrame(DataFrame) bool
        +close()
        -writeCsvLine(DataFrame) string
        -escapeString(string) string
    }

    class ITimeFormatter {
        <<interface>>
        +format(double) string
    }

    class TimeFormatter {
        -string m_format
        +format(double) string
    }

    class IProgressReporter {
        <<interface>>
        +report(size_t, size_t)
        +reportMessage(string)
    }

    class ILogger {
        <<interface>>
        +error(string)
        +warning(string)
        +info(string)
        +debug(string)
    }

    DataConverter --> IDataReader : uses
    DataConverter --> IDataWriter : uses
    DataConverter --> ITimeFormatter : uses
    DataConverter --> IProgressReporter : uses
    DataConverter --> ILogger : uses
    BinaryReader ..|> IDataReader : implements
    CsvWriter ..|> IDataWriter : implements
    TimeFormatter ..|> ITimeFormatter : implements
```

#### Data Logger Sequence Diagram (Acquisition Flow)

```mermaid
sequenceDiagram
    participant Main
    participant AcquisitionManager
    participant Device as AdvantechDevice
    participant Engine as DataProcessingEngine
    participant Writer as BinaryFileWriter
    participant Callback as Hardware Callback

    Main->>AcquisitionManager: initialize()
    AcquisitionManager->>Device: initialize(description)
    AcquisitionManager->>Device: configure(config)
    AcquisitionManager->>Writer: open(filePath)
    
    Main->>AcquisitionManager: startAcquisition()
    AcquisitionManager->>Device: setDataReadyCallback(callback)
    AcquisitionManager->>Engine: start(samplingRate, startCh, endCh)
    AcquisitionManager->>Device: start()
    
    loop Every hardware interrupt
        Callback->>Device: BDAQCALL(data)
        Device->>Engine: pushDataFrame(frame)
        Engine->>Engine: queue.push(frame)
    end
    
    loop Writer thread
        Engine->>Engine: queue.pop(frame)
        Engine->>Writer: writeFrame(frame)
    end
    
    Main->>AcquisitionManager: waitForStop()
    Note over Main: Press ENTER to stop
    
    Main->>AcquisitionManager: stopAcquisition()
    AcquisitionManager->>Device: stop()
    AcquisitionManager->>Engine: stop()
    Engine->>Writer: close()
    AcquisitionManager->>Device: dispose()
```

#### Data Converter Sequence Diagram (Conversion Flow)

```mermaid
sequenceDiagram
    participant Main
    participant DataConverter
    participant Reader as BinaryReader
    participant Writer as CsvWriter
    participant Progress as ProgressReporter
    participant Logger

    Main->>DataConverter: convert(inputPath, outputPath)
    DataConverter->>Reader: open(inputPath)
    Reader-->>DataConverter: success
    
    DataConverter->>Reader: readHeader(header)
    Reader-->>DataConverter: DataHeader
    
    DataConverter->>Writer: open(outputPath)
    Writer-->>DataConverter: success
    
    DataConverter->>Writer: writeHeader(header)
    
    DataConverter->>Progress: report(totalFrames, 0)
    
    loop Read and convert frames
        DataConverter->>Reader: readFrames(frames, maxFrames)
        Reader-->>DataConverter: vector<DataFrame>
        
        loop Each frame
            DataConverter->>Writer: writeFrame(frame)
            DataConverter->>Progress: report(processed, total)
        end
    end
    
    DataConverter->>Reader: close()
    DataConverter->>Writer: close()
    
    DataConverter->>Progress: reportMessage("Conversion complete")
    DataConverter-->>Main: success
```

### Data Logger

The data logger uses a three-thread architecture:

1. **Main Thread** — Controls the acquisition lifecycle (start/stop)
2. **Callback Thread** — Hardware interrupt handler (`BDAQCALL`), pushes data to queue
3. **Writer Thread** — Reads from queue, writes to binary file

### Key Components

| Component | Purpose |
|-----------|---------|
| `IDataAcquisitionDevice` | Hardware abstraction interface |
| `AdvantechDevice` | PCI-1715 implementation using DAQNavi SDK |
| `DataProcessingEngine` | Queue management and frame assembly |
| `BinaryFileWriter` | Binary file output with header |
| `AcquisitionManager` | Orchestrates device, engine, and writer |

### Data Format

Binary files contain a `DataHeader` struct followed by raw `double` samples per channel:

```cpp
struct DataHeader {
    uint32_t magic;           // File signature
    uint32_t version;         // Format version
    double samplingRate;      // Samples per second
    uint32_t startChannel;    // First channel
    uint32_t endChannel;      // Last channel
    uint32_t channelCount;    // Total channels
    double startTimeSeconds;  // Acquisition start time
    double endTimeSeconds;    // Acquisition end time
};
```

## Development

### Prerequisites

- **Windows** (only, due to DAQNavi SDK)
- **Visual Studio 2022** (or compatible)
- **CMake** 3.20 or higher
- **Advantech DAQNavi SDK** (included in `library/`)

### Project Structure

```
pci-1715/
├── AGENTS.md                 # AI agent instructions
├── build.bat                 # Interactive build script
├── CMakeLists.txt            # Root CMake configuration
├── run.bat                   # Interactive launcher
├── application/
│   ├── data-logger/          # Acquisition application
│   │   ├── core/             # Core components
│   │   ├── devices/          # Device implementations
│   │   ├── main/             # Entry point
│   │   └── storage/          # File I/O
│   └── data-converter/       # Conversion utility
│       ├── core/             # Reader/writer interfaces
│       ├── factory/          # Factory patterns
│       └── main/             # Entry point
└── library/
    └── DAQNavi/              # Advantech SDK
        ├── inc/              # Headers
        └── src/              # Source
```

### Build Configuration

- **C++ Standard:** C++17
- **MSVC Runtime:** Static linking (`/MT` for Release, `/MTd` for Debug)
- **Target Platform:** Windows only

## Common Pitfalls

1. **Callback must not block** — The `BDAQCALL` callback runs on a high-priority thread. Never perform I/O, heap allocations, or long operations inside it — only push data to the queue.

2. **CSV locale** — The converter outputs CSV with Russian locale (comma decimal separator). Ensure your parsing tools handle this correctly.

3. **Binary file format** — The binary file includes a `DataHeader` struct at the beginning. Don't treat the file as raw `double` samples only — read the header first.

4. **Device description** — The device string format is `PCI-1715,BID#0` (bus ID 0). Adjust the BID number if you have multiple cards.

5. **FIFO buffer** — The PCI-1715 has a 1024-sample FIFO. The default buffer size of 10240 samples per channel provides a 10x safety margin.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

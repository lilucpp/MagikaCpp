# Building the Magika C++ Wrapper

## Prerequisites

1. Go 1.16 or higher
2. ONNX Runtime C/C++ library (provided in onnxruntime-static-1.17.3/win32)
3. C++ compiler with C++11 support
4. CMake 3.12 or higher

## Building with CMake (Recommended)

### Method 1: Using CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

This will automatically:
1. Build the Go shared library from the Go source code
2. Compile the C++ wrapper
3. Link everything together

### Method 2: Manual compilation

#### Step 1: Build the Go shared library

First, you need to build the Go code as a shared library that can be linked with C++:

```bash
cd go
go build -buildmode=c-shared -o libmagika.so magika/magika_c_api.go
```

This will generate two files:
- `libmagika.so` (or `libmagika.dll` on Windows) - the shared library
- `libmagika.h` - the C header file

#### Step 2: Compile the C++ wrapper

To compile the C++ wrapper, you need to link against the Go shared library:

```bash
g++ -std=c++11 -I./go -L./go -o magika_cpp_example magika_cpp_wrapper.cpp -lmagika
```

### Step 3: Run the example

Make sure the shared library is in your library path:

```bash
# Linux
export LD_LIBRARY_PATH=./go:$LD_LIBRARY_PATH
./magika_cpp_example /path/to/some/file

# Windows (cmd)
set PATH=%PATH%;.\go
magika_cpp_example.exe \path\to\some\file

# Windows (PowerShell)
$env:PATH += ";.\go"
.\magika_cpp_example.exe \path\to\some\file
```

## Using the C++ Wrapper in Your Own Project

To use the Magika C++ wrapper in your own project:

1. Include the header file:
   ```cpp
   #include "magika_cpp_wrapper.h"
   ```

2. Link against the Go shared library when compiling:
   ```bash
   g++ -std=c++11 -I/path/to/magika/go -L/path/to/magika/go your_code.cpp -lmagika
   ```

3. In your C++ code:
   ```cpp
   #include "magika_cpp_wrapper.h"
   
   try {
       std::string filetype = MagikaScanner::scanFile("/path/to/file.txt");
       std::cout << "Detected file type: " << filetype << std::endl;
       
       // Or with confidence score:
       auto result = MagikaScanner::scanFileWithScore("/path/to/file.txt");
       std::cout << "File type: " << result.first << ", Confidence: " << result.second << std::endl;
   } catch (const MagikaException& e) {
       std::cerr << "Error: " << e.what() << std::endl;
   }
   ```

## Environment Variables

The wrapper recognizes two environment variables:

- `MAGIKA_ASSETS_DIR`: Directory containing the Magika model files (defaults to `./standard_v3_3`)
- `MAGIKA_MODEL`: Model name to use (defaults to `.`)

Example:
```bash
export MAGIKA_ASSETS_DIR=/path/to/models
export MAGIKA_MODEL=v3
```
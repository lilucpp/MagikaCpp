#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <memory>
#include <cstring>
#include "magikacpp.h"
#include "filefeatures.h"
#include "config.h"
#include "seekable.h"

#ifdef __cplusplus
extern "C" {
#endif

// Define the C struct that matches the Go definition
struct ContentTypeResult {
    char* label;
    float score;
};

// Declare the external C functions implemented in Go with Windows DLL import support
#ifdef _WIN32
__declspec(dllimport)
#endif
extern struct ContentTypeResult* magika_scan_file(const char* filepath);

#ifdef _WIN32
__declspec(dllimport)
#endif
extern void free_content_type_result(struct ContentTypeResult* result);

#ifdef __cplusplus
}
#endif
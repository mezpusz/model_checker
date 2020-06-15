#pragma once

#include <string>

#include "circuit.h"

bool parse_aiger_file(const std::string& input_file, circuit& out);
#include "aiger_parser.h"

#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

std::vector<std::string> get_and_split_line(std::ifstream& file) {
    std::vector<std::string> res;
    std::string line;
    if (!std::getline(file, line)) {
        return res;
    }

    std::istringstream istr(line);
    std::string temp;

    while (std::getline(istr, temp, ' ')) {
        res.push_back(temp);
    }
    // ignore symbol lines
    // TODO: do smth more elegant here
    if (!res.empty() && (res[0][0] == 'i' || res[0][0] == 'l' || res[0][0] == 'o')) {
        return get_and_split_line(file);
    }
    return res;
}

bool parse_aiger_file(const std::string& input_file, circuit& out) {
    std::ifstream file(input_file);
    if (!file.is_open()) {
        return false;
    }

    auto header = get_and_split_line(file);
    if (header.size() != 6 || header[0] != "aag") {
        return false;
    }

    int M = atoi(header[1].c_str());
    int I = atoi(header[2].c_str());
    int L = atoi(header[3].c_str());
    int O = atoi(header[4].c_str());
    int A = atoi(header[5].c_str());

    out.M = M;

    for (int i = 0; i < I; i++) {
        auto input = get_and_split_line(file);
        if (input.size() != 1) {
            std::cerr << "input line must contain 1 integer" << std::endl;
            return false;
        }
        out.inputs.insert(atoi(input[0].c_str()));
    }

    for (int i = 0; i < L; i++) {
        auto latch = get_and_split_line(file);
        if (latch.size() != 2) {
            std::cerr << "latch line must contain 2 integers" << std::endl;
            return false;
        }
        out.latches.insert(std::make_pair(atoi(latch[0].c_str()), atoi(latch[1].c_str())));
    }

    for (int i = 0; i < O; i++) {
        auto output = get_and_split_line(file);
        if (output.size() != 1) {
            std::cerr << "output line must contain 1 integer" << std::endl;
            return false;
        }
        out.outputs.insert(atoi(output[0].c_str()));
    }

    for (int i = 0; i < A; i++) {
        auto and_ = get_and_split_line(file);
        if (and_.size() != 3) {
            std::cerr << "and line must contain 3 integers" << std::endl;
            return false;
        }
        out.ands.insert(std::make_pair(
            std::make_pair(atoi(and_[0].c_str()), atoi(and_[1].c_str())),
            atoi(and_[2].c_str())));
    }

    return true;
}
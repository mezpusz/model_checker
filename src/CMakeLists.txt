cmake_minimum_required (VERSION 3.6)

add_executable(model_checker aiger_parser.cc
                             aiger_parser.h
                             circuit.cc
                             circuit.h
                             formula.cc
                             formula.h
                             main.cc)

include_directories("${PROJECT_SOURCE_DIR}")

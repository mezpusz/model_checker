cmake_minimum_required (VERSION 3.6)

add_executable(model_checker aiger_parser.cc
                             aiger_parser.h
                             bmc.cc
                             bmc.h
                             circuit.cc
                             circuit.h
                             dimacs.cc
                             dimacs.h
                             formula.cc
                             formula.h
                             main.cc
                             minisat/File.C
                             minisat/File.h
                             minisat/Proof.C
                             minisat/Proof.h
                             minisat/Solver.C
                             minisat/Solver.h)

include_directories("${PROJECT_SOURCE_DIR}")

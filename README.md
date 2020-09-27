# model_checker

## Introduction

model_checker is a program for verifying AIGER-based ASCII encoded circuits. Specifically, it checks that the output bit is not 1 with the following two algorithms:
* bounded model checking
* interpolation

## Building the project

After checking out the project repository and navigating a terminal instance to the root folder of the project, hit the following commands:
```bash
cmake .
make
```

## Running model_checker

The bounded model checking part can be run by the following commands:
```bash
run_part1 <filename> <k>
```
or
```bash
out/model_checker <filename> <k>
```
This will check the if the circuit can output 1 in exactly `k` iterations with all latches set to 0.

The interpolation part can be run by the following commands:
```bash
run_part2 <filename>
```
or
```bash
out/model_checker <filename>
```
This tries to prove that the circuit will never output a 1 bit.

Both algorithms return OK if the corresponding circuit property is satisfied (no output of 1), otherwise returns FAIL.

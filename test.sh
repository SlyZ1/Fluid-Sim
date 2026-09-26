#!/bin/bash
cmake -B build -DBUILD_TEST='ON' && cmake --build build && ctest --test-dir build --output-on-failure
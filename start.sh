#!/bin/bash
cmake -B build -DBUILD_TEST='OFF' && cmake --build build && prime-run ./myprogram
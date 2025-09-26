#!/bin/bash

g++ -std=c++23 -Wall -Wextra -Wpedantic -Wno-unused-parameter main.cpp ShaderLoading.cpp -lGL -lglfw -o triangle


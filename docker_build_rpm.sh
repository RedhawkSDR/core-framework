#!/bin/bash
mkdir -p output
docker run --rm -v $(pwd):/src:ro -v $(pwd)/output:/rpms rh-4-0-4-deps-el9:latest /src/build-rpms.sh

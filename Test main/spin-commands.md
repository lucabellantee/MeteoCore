# Spin Commands for Promela Model Analysis

This document describes the main commands used to analyze Promela models with the Spin tool via a Docker container.

## Main Commands

### Build the container

```bash
docker build -t spin .
```
This command creates a Docker image named "spin" based on the Dockerfile in the current directory. The image contains the environment needed to run Spin, including all required packages and dependencies.

### Start and enter the container

```bash
docker run -it --rm -v "%cd%:/work" spin
```

This command runs a container from the "spin" image, mounts the current host directory into the container’s "/work" directory, and provides an interactive terminal. The `--rm` option ensures the container is automatically removed when closed.

### Generate the verifier

```bash
spin -a test_main.pml
```
This command analyzes the Promela model (test_main.pml) and generates several C source files (pan.c, pan.h, etc.) that implement a verifier specific to the model. This verifier is tailored to the model’s features.

### Compile and run the verifier

```bash
gcc -o pan pan.c
./pan
```
The first command compiles the pan.c file, creating an executable called pan. The second runs the verifier, which automatically checks for deadlocks, assertion violations, and other issues in the model. Without additional parameters, it performs a basic verification.

### Run a deadlock-specific verification

```bash
./pan -d
```

This runs the verifier focusing specifically on detecting deadlocks (invalid end states). The `-d` option enables deadlock detection and shows details about all possible states and transitions.

## Additional Useful Commands

### Starvation check (non-progress cycles)

```bash
gcc -DNP -o pan pan.c
./pan -l -f
```

Compiles the verifier with the `-DNP` flag and runs it to detect non-progress cycles, which may indicate starvation situations (a process never acquiring needed resources).

### Verify specific LTL properties

```bash
spin -run -ltl deadlock_free test_main.pml
```

Checks a specific LTL property defined in the model (in this case, "deadlock_free").


### Full analysis with results saved

```bash
spin -a test_main.pml
gcc -DSAFETY -o pan pan.c
./pan -d -v > analysis_results.txt 2>&1
```

Performs a complete analysis and saves the output to a text file for later review.

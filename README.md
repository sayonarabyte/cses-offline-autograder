# CSES Autograder
A hybrid Python/C++ local autograder explicitly designed for macOS to test C++ solutions for [CSES Problem Set](https://cses.fi/problemset/) problems.

## Features
- **Forces Sandbox Constraints:** Enforces 1.0s CPU time, 512MB RAM, and 5MB output bounds via POSIX constraints.
## Prerequisites
- **macOS** (This tool relies on specific Darwin-based memory structures)
- Python 3.6+
- `g++` (Standard macOS Clang works)
## Installation
1. Clone or navigate to the autograder directory.
2. Install the necessary Python packages:
   ```bash
   pip install -r requirements.txt
   ```
   *(Optional)* If you plan on using the `fetch_samples.py` script to scrape public tests, install the scraping dependencies:
   ```bash
   pip install requests beautifulsoup4
   ```
## Getting the Test Cases
The autograder expects test cases to be placed in a `tests/<normalized_problem_name>/` directory. 
The problem name is derived from your source file, with all spaces and dashes stripped, lowercased (e.g. `Weird_Algorithm.cpp` looks for `tests/weirdalgorithm/`).


## To automatically fetch all public sample test cases from CSES:**
```bash
./fetch_samples.py
```
This script will scrape the publicly available test cases on the CSES website and correctly structure them in the `./tests/` folder.
*(Note: CSES does not release their hidden grading test cases to the public. If you acquire these hidden `.zip` files from a community dataset, simply unzip them into the corresponding problem directory.)*
## Usage
Create your C++ solution file anywhere in the directory (e.g., `Weird_Algorithm.cpp`). 
Run the autograder against your file:
```bash
./autograder.py run Weird_Algorithm.cpp
```
### Example Output
```text
Compiling Weird_Algorithm.cpp...

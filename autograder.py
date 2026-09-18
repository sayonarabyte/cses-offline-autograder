#!/usr/bin/env python3
import argparse
import subprocess
import os
import json
import sys
import glob
import re
from rich.progress import Progress
from rich.table import Table
from rich.console import Console

TIME_LIMIT = 1.0
MEM_LIMIT = 512.0

console = Console()

def natural_sort_key(s):
    return [int(text) if text.isdigit() else text.lower() for text in re.split('([0-9]+)', s)]

def compile_grader():
    base_dir = os.path.dirname(os.path.abspath(__file__))
    grader_cpp = os.path.join(base_dir, "grader.cpp")
    grader_bin = os.path.join(base_dir, "grader.bin")

    if not os.path.exists(grader_bin) or os.path.getmtime(grader_cpp) > os.path.getmtime(grader_bin):
        console.print("[yellow]Compiling grader...[/yellow]")
        res = subprocess.run(["g++", "-O3", "-std=c++17", "-Wall", grader_cpp, "-o", grader_bin])
        if res.returncode != 0:
            console.print("[red]Failed to compile grader.cpp[/red]")
            sys.exit(1)
    return grader_bin

def main():
    parser = argparse.ArgumentParser(description="CSES Autograder")
    parser.add_argument("run", help="run command")
    parser.add_argument("target", help="Target source file")
    args = parser.parse_args()

    if args.run != "run":
        console.print("[red]Invalid command. Use: autograder run <file>[/red]")
        sys.exit(1)

    grader_bin = compile_grader()

    target_file = os.path.abspath(args.target)
    if not os.path.exists(target_file):
        console.print(f"[red]File {target_file} not found.[/red]")
        sys.exit(1)

    basename = os.path.basename(target_file)
    problem_name = re.sub(r'[ \-_]', '', basename).split('.')[0].lower()
    
    test_dir = os.path.join(os.path.dirname(target_file), "tests", problem_name)
    if not os.path.exists(test_dir):
        console.print(f"[red]Test directory {test_dir} not found.[/red]")
        sys.exit(1)

    in_files = sorted(glob.glob(os.path.join(test_dir, "*.in")), key=natural_sort_key)
    if not in_files:
        console.print(f"[red]No .in files found in {test_dir}[/red]")
        sys.exit(1)

    test_cases = []
    for in_file in in_files:
        out_file = in_file[:-3] + ".out"
        if not os.path.exists(out_file):
            console.print(f"[red]Missing {out_file} for {in_file}[/red]")
            sys.exit(1)
        test_cases.append((in_file, out_file))

    target_bin = os.path.join(os.path.dirname(target_file), "solution.bin")
    
    console.print(f"Compiling [cyan]{basename}[/cyan]...")
    compile_cmd = ["g++", "-O2", "-std=c++17", "-Wall", target_file, "-o", target_bin]
    try:
        res = subprocess.run(compile_cmd, capture_output=True, text=True, timeout=15.0)
        if res.returncode != 0:
            console.print("[red]Compilation Error (CE)[/red]")
            console.print(res.stderr)
            sys.exit(1)
    except subprocess.TimeoutExpired:
        console.print("[red]Compilation Error (CE): Compiler timed out[/red]")
        sys.exit(1)

    results = []
    try:
        with Progress() as progress:
            task = progress.add_task("[cyan]Grading...", total=len(test_cases))
            
            for in_file, out_file in test_cases:
                test_name = os.path.basename(in_file)
                
                cmd = [
                    grader_bin,
                    target_bin,
                    in_file,
                    out_file,
                    str(TIME_LIMIT),
                    str(MEM_LIMIT)
                ]
                
                process = subprocess.Popen(
                    cmd,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.PIPE,
                    start_new_session=True
                )
                
                status = "RE"
                time_sec = 0.0
                mem_kb = 0
                error_msg = None
                
                try:
                    out, err = process.communicate(timeout=TIME_LIMIT + 2.0)
                    try:
                        payload = json.loads(out.decode('utf-8'))
                        status = payload.get("status", "RE")
                        time_sec = payload.get("time_seconds", 0.0)
                        mem_kb = payload.get("max_rss_kb", 0)
                        error_msg = payload.get("error_message")
                    except json.JSONDecodeError:
                        status = "RE"
                        error_msg = "Invalid JSON from grader"
                except subprocess.TimeoutExpired:
                    status = "TLE"
                    os.killpg(os.getpgid(process.pid), 9)
                    process.communicate() # reap
                
                results.append({
                    "test": test_name,
                    "status": status,
                    "time": time_sec,
                    "mem": mem_kb,
                    "err": error_msg
                })
                
                progress.update(task, advance=1)
                
    finally:
        if os.path.exists(target_bin):
            os.remove(target_bin)

    table = Table(title=f"Results: {basename}")
    table.add_column("Test", justify="right", style="cyan", no_wrap=True)
    table.add_column("Verdict", style="bold")
    table.add_column("Time (s)", justify="right")
    table.add_column("Mem (MB)", justify="right")
    table.add_column("Details", style="dim")

    for r in results:
        v = r["status"]
        color = "green" if v == "AC" else "red"
        
        mem_mb = r["mem"] / 1024.0
        details = r["err"] if r["err"] and r["err"] != "null" else ""
        
        table.add_row(
            r["test"],
            f"[{color}]{v}[/{color}]",
            f"{r['time']:.3f}",
            f"{mem_mb:.2f}",
            details
        )

    console.print(table)

if __name__ == "__main__":
    main()

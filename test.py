#!/usr/bin/env python3

import os
import subprocess
import sys

north = "./src/north"
north_com = "./src/north -c -v"

def record(test_file):
    outfile_path = os.path.splitext(test_file)[0] + ".txt"
    sim_result = None
    com_result = None
    with open(outfile_path, "w") as f:
        sim_result = subprocess.run([north, "-s", test_file], stdout=subprocess.PIPE)
        os.system(north_com + " " + test_file)
        com_result = subprocess.run([os.path.splitext(test_file)[0]], stdout=subprocess.PIPE)
        if sim_result.stdout == com_result.stdout:
            print("[INFO]: Recording results for %s" % test_file)
            f.write(sim_result.stdout.decode("utf8"))
        else:
            print("[ERROR]: Compilation and simulation results do not agree for test %s" % test_file)
            print("Simulation:\n%s" % sim_result.stdout.decode("utf8"))
            print("Compilation:\n%s" % com_result.stdout.decode("utf8"))

def test(test_file, results_file):
    expected = None
    with open(results_file, "r") as f:
        expected = f.read()
    sim = subprocess.run([north, "-s", test_file], stdout=subprocess.PIPE)
    os.system(north_com + " " + test_file)
    com = subprocess.run([os.path.splitext(test_file)[0]], stdout=subprocess.PIPE)
    sim_result = sim.stdout.decode("utf8")
    com_result = com.stdout.decode("utf8")
    if sim_result != expected:
        print("[INFO]: Simulation failed for %s. Expected %s but got %s" % (test_file, expected, sim_result))
        return False
    elif com_result != expected:
        print("[INFO]: Compilation failed for %s. Expected %s but got %s" % (test_file, expected, com_result))
        return False
    return True
              
test_dir = "./tests/"
test_dir_files = next(os.walk(test_dir), (None, None, []))[2]
test_files = sorted([test_dir+f for f in test_dir_files if os.path.splitext(f)[1] == ".nth"])
results_files = sorted([test_dir+f for f in test_dir_files if os.path.splitext(f)[1] == ".txt"])
success = 0

for arg in sys.argv[1:]:
    if arg == "-r":
        for f in test_files:
            record(f)
    elif arg == "-t":
        if len(results_files) != len(test_files):
            print("Please record test results first.")
            exit(1)
        for testf, resf in zip(test_files, results_files):
            if test(testf, resf) == True:
                success += 1
        print("[INFO] Tests completed: %d Success: %d" % (len(test_files), success))
    else:
        print("Unrecognised option %s" % arg)
        exit(1)

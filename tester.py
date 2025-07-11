import subprocess
import os

def read_file(file_path):
    with open(file_path, 'r') as file:
        return file.read()

def preprocess_output(output):
    return ' '.join(output.split())

def compare_outputs(output1, output2):
    lines_output1 = output1.split('\n')
    lines_output2 = output2.split('\n')
    
    while lines_output1 and not lines_output1[-1]:
        lines_output1.pop()
    while lines_output2 and not lines_output2[-1]:
        lines_output2.pop()

    num_lines_output1 = len(lines_output1)
    num_lines_output2 = len(lines_output2)
    
    if num_lines_output1 != num_lines_output2:
        return False, num_lines_output1, num_lines_output2, None, None
    
    for line1, line2 in zip(lines_output1, lines_output2):
        if preprocess_output(line1) != preprocess_output(line2):
            return False, num_lines_output1, num_lines_output2, line1, line2
    
    return True, num_lines_output1, num_lines_output2, None, None

def run_tests(exe_path, test_folder, print_output=False):
    test_files = [f.path for f in os.scandir(test_folder) if f.is_file() and f.name.startswith('stdin_')]
    
    if not test_files:
        print("No input files found in the specified test folder.")
        return
    
    test_files.sort()
    
    for stdin_path in test_files:
        stdout_path = stdin_path.replace("stdin_", "stdout_")
        parameters_path = stdin_path.replace("stdin_", "parameters_")
        
        if not (os.path.exists(stdout_path) and os.path.exists(parameters_path)):
            print(f"Missing files for test case {stdin_path}. Skipping.")
            continue
        
        input_data = read_file(stdin_path)
        expected_output = read_file(stdout_path).strip()
        parameters = read_file(parameters_path).strip()
        
        process = subprocess.Popen([exe_path] + parameters.split(), 
                                   stdin=subprocess.PIPE, 
                                   stdout=subprocess.PIPE, 
                                   stderr=subprocess.PIPE, 
                                   text=True)
        output, errors = process.communicate(input=input_data)
        
        if process.returncode != 0:
            print(f"Test case {stdin_path}: Execution failed with error:\n{errors}")
            continue
        
        result, num_lines_output, num_lines_expected, diff_output, diff_expected = compare_outputs(output, expected_output)
        if result:
            print(f"\n\nTEST - {stdin_path} - PASSED")
        else:
            print(f"\n\nTEST - {stdin_path} - FAILED")
            print(f"\n-- YOUR OUTPUT --\n{diff_output}")
            print(f"\n-- EXPECTED OUTPUT --\n{diff_expected}")
            print(f"\nNumber of lines in your output: {num_lines_output}")
            print(f"Number of lines in expected output: {num_lines_expected}")
        
        if print_output:
            print("\nOutput:")
            print(output)

def main():
    # Get the directory where this script is located
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Look for the executable in multiple locations
    possible_exe_paths = [
        os.path.join(script_dir, 'cmake-build-debug', 'z4'),
        os.path.join(script_dir, 'z4'),
        os.path.join(script_dir, 'build', 'z4')
    ]
    
    exe_path = None
    for path in possible_exe_paths:
        if os.path.exists(path):
            exe_path = path
            break
    
    # Check if executable exists
    if exe_path is None:
        print("Executable 'z4' not found in any of these locations:")
        for path in possible_exe_paths:
            print(f"  {path}")
        print("\nPlease build the project first using one of these methods:")
        print("\nMethod 1 - CMake (if you have cmake installed):")
        print("  mkdir -p cmake-build-debug")
        print("  cd cmake-build-debug")
        print("  cmake ..")
        print("  make")
        print("\nMethod 2 - Direct compilation (if you have gcc):")
        print("  gcc -Wall -Wextra -std=c11 -Iinclude src/z4.c src/monopoly.c -o z4")
        return
    
    # Look for tests directory
    tests_dir = os.path.join(script_dir, 'tests')
    if not os.path.exists(tests_dir):
        print(f"Tests directory not found at {tests_dir}")
        return
    
    # Find all test scenario directories (s1, s2...)
    test_scenarios = []
    for item in os.listdir(tests_dir):
        item_path = os.path.join(tests_dir, item)
        if os.path.isdir(item_path) and item.startswith('s') and item[1:].isdigit():
            test_scenarios.append(item)
    
    test_scenarios.sort()
    
    if not test_scenarios:
        print("No test scenarios found in tests directory")
        return
    
    print(f"Found executable: {exe_path}")
    print(f"Running tests from: {tests_dir}")
    print(f"Test scenarios: {', '.join(test_scenarios)}")
    print("-" * 50)
    
    # Run all test scenarios
    for scenario in test_scenarios:
        scenario_path = os.path.join(tests_dir, scenario)
        print(f"\nRunning tests for scenario {scenario}:")
        run_tests(exe_path, scenario_path, print_output=False)

if __name__ == "__main__":
    main()


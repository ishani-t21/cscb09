> # **SYSTEM MONITORING TOOL**
___

<br>

> # **General Overview**
The System Monitoring Tool is a C program designed to monitor system metrics like memory usage, CPU utilization, number of cores and maximum cpu frequency information on Linux-based systems, providing real-time graphical outputs. This involves user-defined parameters where they can customize the sampling frequency, number of samples, and which metrics to track.

<br>

> # **Problem-Solving Approach**

To solve this problem, I adopted a modular and systematic approach, breaking down the overall task into smaller, manageable components. The core of the solution is focused on gathering real-time system data related to CPU, memory, and core information, and then displaying it dynamically.

**Data Collection:** I implemented several helper functions to fetch system data:
- `get_cpu_cores()` retrieves the number of CPU cores.
- `get_max_cpu_freq()` determines the maximum CPU frequency.
- `get_mem_total()` gets the total available memory, and `compute_memory_usage()` calculates the current memory usage.
- `get_cpu_usage_diff()` and `get_cpu_util()` calculate the CPU utilization based on total and idle CPU times.

**Real-time Data Updates:**
- To ensure that the information remains up-to-date, I utilized ANSI escape sequences to clear and refresh the terminal display periodically. This allows the user to see continuous, real-time updates of memory and CPU usage without needing to manually refresh or re-run the program.

**Dynamic Display Functions:**
- I implemented `display_samp_microsec()` to show sample count and delay information. The `display_memory_cpu()` function updates memory and CPU graphs dynamically, computing memory usage and CPU utilization over multiple samples for real-time display.

**Argument Parsing:**
To manage user input, I structured an efficient argument parser to handle both positional (samples and tdelay) and flag-based (`--memory`, `--cpu`, `--cores`) arguments. The functions `parse_positional_args()` and `parse_flag_args()` process these inputs, validate them, and update the relevant flags.

<br>

> ## **Important Formulas**
**1. CPU Utilization Calculation:**
- Formula used in get_cpu_usage_diff():

`CPU Utilization = (1 - (Idle Time Difference / Total Time Difference)) * 100`

**When:** This formula is used to calculate the CPU utilization as a percentage by comparing two samples of total CPU time and idle CPU time.

**2. Memory Usage Calculation:**
- Formula used in compute_memory_usage():

`Memory Used = Total Memory - Free Memory`

**When:** This formula calculates the memory usage based on the total memory and free memory values, which are retrieved using system calls. The memory used is then displayed.


<br>

> ## **Assumptions**

- The program execution assumes a standard terminal size as the ones on the IA Lab Machines to ensure proper positioning of the escape codes used in printing Memory & CPU information
- The plotting logic for Memory and CPU graphs relies on ANSI escape codes for cursor movement
- 0 sample size not allowed to be specified for positional argument or value for --samples flag
    - Positional argument 0 will print "Invalid samples positional argument" and program ends execution
    - Value 0 specified for --samples flag will print "Invalid samples flag argument" and program ends execution
- 0 tdelay not allowed to be specified for positional argument or value for --tdelay flag
- Values for specifying samples and delay cannot be negative (due to logical reasons)
- Cannot have a tdelay positional argument without also including samples positional argument because first number put before including flags will be considered as the specified sample size

<br>

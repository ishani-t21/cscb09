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

> # **Overview of the Functions**

## Tables of Contents
1. [Flag and Arguments Handling](#section-1)
    - [whole_number](#whole_number)
    - [no_flags_given](#no_flags_given)
    - [parse_positional_args](#parse_positional_args)
    - [parse_flag_args](#parse_flag_args)
    - [parse_flags](#parse_flags)
2. [Memory Utilization](#section-2)
    - [convert_kb_to_gb](#convert_kb_to_gb)
    - [get_mem_total](#get_mem_total)
    - [get_free_memory](#get_free_memory)
    - [compute_memory_usage](#compute_memory_usage)
    - [make_horiz_axis](#make_horiz_axis)
    - [draw_mem_graph_struc](#draw_mem_graph_struc)
    - [place_mem_samp_pt](#place_mem_samp_pt)
3. [CPU Utilization](#section-3)
    - [parse_cpu_times](#parse_cpu_times)
    - [get_cpu_util](#get_cpu_util)
    - [get_cpu_usage_diff](#get_cpu_usage_diff)
    - [draw_cpu_graph_struc](#draw_cpu_graph_struc)
    - [place_cpu_samp_pt](#place_cpu_samp_pt)
4. [Cores and Max Frequency Information](#section-4)
    - [get_cpu_cores](#get_cpu_cores)
    - [convert_khz_to_ghz](#convert_khz_to_ghz)
    - [get_max_cpu_freq](#get_max_cpu_freq)
    - [print_rem_cores](#print_rem_cores)
    - [print_cores](#print_cores)
5. [Additional Display Functions](#section-5)
    - [display_samp_microsec](#display_samp_microsec)
    - [display_memory_cpu](#display_memory_cpu)
---

<br><br>

> ### **Flag and Arguments Handling**

**`Function: whole_number`**

**Description:**

This function checks if a given string represents a valid whole number (integer). It verifies that the string does not contain any non-numeric characters, except for an optional leading negative sign.

**Parameters:**  
- `input_str (char*)` : The string that needs to be checked to determine if it's a valid whole number.

**Returns:**

A boolean value as int (1 for true, 0 for false). The function returns true if the input string is a valid whole number, and false otherwise.

**Example:**
```c
char* str = "12345";
if (whole_number(str)) {
    printf("The string represents a valid whole number.\n");
} else {
    printf("The string is not a valid whole number.\n");
}
```


**`Function: no_flags_given`**

**Description:**

Checks if no flags for memory, CPU, or cores are provided. If none are set, it defaults all flags (`memory_flag`, `cpu_flag`, `cores_flag`) to 1, ensuring the program operates on all resources.

**Parameters:**  
- `flags (Flags*)` : A pointer to a `Flags` structure holding the flag values (`memory_flag`, `cpu_flag`, `cores_flag`).

**Returns:**

No return value. Updates the `Flags` structure.

**Example:**
```c
Flags flags = {0, 0, 0};  
no_flags_given(&flags);
printf("%d %d %d\n", flags.memory_flag, flags.cpu_flag, flags.cores_flag);  
// Output: 1 1 1
```


**`Function: parse_positional_args`**

**Description:**

This function parses the positional arguments from the command-line input (samples and tdelay). It checks if both arguments are provided and validates them. If valid, it updates the `flags` structure with the parsed values.

**Parameters:**  
- `index (int*)` : A pointer to the current argument index in the `argv` array. It is updated as the function processes the arguments.
- `argc (int)` : The total number of arguments passed to the program.
- `argv (char**)` : The array of argument strings.
- `flags (Flags*)` : A pointer to the `Flags` structure, which stores the parsed values for `samples_value` and `tdelay_value`.
- `pos_present (int*)` : A pointer to an integer that tracks how many positional arguments are present (1 for samples, 2 for tdelay).

**Returns:**

No return value. Updates the `Flags` structure and modifies `index` and `pos_present`.

**Example:**
```c
int index = 1, pos_present = 0;
parse_positional_args(&index, argc, argv, &flags, &pos_present);
```


**`Function: parse_flag_args`**

**Description:**

This function parses flag-based arguments from the command-line input (`--memory`, `--cpu`, `--cores`, `--samples`, `--tdelay`). It checks for valid flags, ensures no duplicates, and updates the flags structure with the parsed values. If any invalid or conflicting flags are encountered, the function exits with an error.

**Parameters:**  
- `index (int*)` : A pointer to the current argument index in the `argv` array. It is updated as the function processes the arguments.
- `argc (int)` : The total number of arguments passed to the program.
- `argv (char**)` : The array of argument strings.
- `flags (Flags*)` : A pointer to the `Flags` structure, which stores the parsed values for various flags.
- `pos_present (int)` : The number of positional arguments already parsed (1 for samples, 2 for tdelay).

**Returns:**

No return value. Updates the `Flags` structure and modifies `index`.

**Example:**
```c
int index = 1;
parse_flag_args(&index, argc, argv, &flags, pos_present);
```


**`Function: parse_flags`**

**Description:**

This function serves as the main entry point for parsing both positional and flag-based arguments. It initializes default values for flags (`samples_value`, `tdelay_value`, `memory_flag`, `cpu_flag`, `cores_flag`), then calls `parse_positional_args` to parse positional arguments followed by `parse_flag_args` to process flag arguments.

**Parameters:**  
- `argc (int)` : The total number of arguments passed to the program.
- `argv (char**)` : The array of argument strings.
- `flags (Flags*)` : A pointer to the `Flags` structure, which stores the parsed values.

**Returns:**

No return value. Updates the `Flags` structure with the parsed values.

**Example:**
```c
Flags flags;
parse_flags(argc, argv, &flags);
```


<br>

> ### **Memory Utilization**

**`Function: convert_kb_to_gb`**

**Description:**

Converts memory values from kilobytes (KB) to gigabytes (GB) for easier readability.

**Parameters:**  
- `kb (long)` : The memory value in kilobytes that needs to be converted to gigabytes.

**Returns:**

The memory value in gigabytes (float).

**Example:**
```c
long memory_kb = 1048576;  // 1 GB in KB
float memory_gb = convert_kb_to_gb(memory_kb);
printf("Memory in GB: %.2f\n", memory_gb);
```


**`Function: get_mem_total`**

**Description:**

Retrieves the total available memory from the system by reading the `/proc/meminfo` file.

**Parameters:**  
- `total_memory (long*)` : A pointer to store the total system memory in kilobytes (KB).

**Returns:**

No return value. Updates `total_memory` via the pointer.

**Example:**
```c
long total_memory;
get_mem_total(&total_memory);
printf("Total memory: %ld KB\n", total_memory);
```


**`Function: get_free_memory`**

**Description:**

Retrieves the amount of free memory available on the system by reading the `/proc/meminfo` file.

**Parameters:**  
- `free_memory (long*)` : A pointer to store the free system memory in kilobytes (KB).

**Returns:**

No return value. Updates `free_memory` via the pointer.

**Example:**
```c
long free_memory;
get_free_memory(&free_memory);
printf("Free memory: %ld KB\n", free_memory);
```


**`Function: compute_memory_usage`**

**Description:**

Calculates the percentage of memory used by the system, based on total and free memory values.

**Parameters:**  
- `total_memory (long)` : The total memory value in kilobytes (KB).
- `free_memory (long)` : The free memory value in kilobytes (KB).
- `mem_usage (float*)` : A pointer that will store the calculated memory usage in percentage.

**Returns:**

No return value. Updates `mem_usage` via pointer with the calculated memory usage percentage.

**Example:**
```c
long total_memory = 16384000;  // 16 GB in KB
long free_memory = 8192000;    // 8 GB in KB
float mem_usage;
compute_memory_usage(total_memory, free_memory, &mem_usage);
printf("Memory usage: %.2f%%\n", mem_usage);
```


**`Function: make_horiz_axis`**

**Description:**

Generates the horizontal axis for the memory utilization graph, based on the number of samples.

**Parameters:**  
- `samples (int)` : The number of memory samples to be displayed along the x-axis.

**Returns:**

No return value. Prints the horizontal axis for the graph.

**Example:**
```c
make_horiz_axis(20);  // Generates a horizontal axis for 20 samples
```


**`Function: draw_mem_graph_struc`**

**Description:**

Draws the graph structure for visualizing memory usage, including the y-axis (percentage scale) and x-axis (time scale).

**Parameters:**  
- `samples (int)` : The number of samples that will be displayed, determining the length of the x-axis.

**Returns:**

No return value. Prints the graph structure to the terminal.

**Example:**
```c
draw_mem_graph_struc(20);  // Draws a memory usage graph structure for 20 samples
```


**`Function: place_mem_samp_pt`**

**Description:**

Places a sample point on the memory usage graph by plotting the memory usage at a specific x and y coordinate. Adjusts the position based on the calculated memory usage and sample index.

**Parameters:**  
- `percent_usage (float)` : The memory usage in percentage that will determine the height (y-position) on the graph.
- `column (int)` : The x-axis position (sample index) where the data point will be placed.
- `spaces_from_top (int)` : The offset value to adjust the y-position to ensure accurate graph placement.

**Returns:**

No return value. The function places a graphical "#" on the terminal to represent the sample point.

**Example:**
```c
place_mem_samp_pt(45.0, 5, 14);  // Places a memory usage point at 45% on the graph at index 5
```


<br>

> ### **CPU Utilization**

**`Function: parse_cpu_times`**

**Description:**

Parses a string line containing CPU usage data and extracts the total CPU time and idle CPU time.

**Parameters:**  
- `line_ptr (char*)` : A string containing CPU data (e.g., from `/proc/stat`).  
- `total_cpu_time (long*)` : A pointer that will store the total CPU time (sum of all time values).  
- `idle_time (long*)` : A pointer that will store the idle CPU time (value at the 4th index of the CPU data).

**Returns:**

No return value. Updates the `total_cpu_time` and `idle_time via` pointers.

**Example:**
```c
long total_cpu_time, idle_time;
parse_cpu_times("cpu 123 456 789 1011", &total_cpu_time, &idle_time);
printf("Total: %ld, Idle: %ld\n", total_cpu_time, idle_time);
```


**`Function: get_cpu_util`**

**Description:**

Reads the CPU statistics from `/proc/stat` and calculates the total and idle CPU times by calling `parse_cpu_times`.

**Parameters:**  
- `total_cpu_time (long*)` : A pointer to store the total CPU time.
- `idle_time (long*)` : A pointer to store the idle CPU time.

**Returns:**

No return value. Updates the `total_cpu_time` and `idle_time` via pointers.

**Example:**
```c
long total_cpu_time, idle_time;
get_cpu_util(&total_cpu_time, &idle_time);
printf("Total CPU time: %ld, Idle CPU time: %ld\n", total_cpu_time, idle_time);
```


**`Function: get_cpu_usage_diff`**

**Description:**

Calculates the CPU utilization by computing the difference between two samples of total CPU time and idle CPU time.

**Parameters:**  
- `cpu_utilization (float*)` : A pointer that will store the computed CPU usage in percentage.
- `prev_total (long)` : The total CPU time from the previous sample.
- `prev_idle (long)` : The idle CPU time from the previous sample.
- `new_total (long)` : The total CPU time from the current sample.
- `new_idle (long)` : The idle CPU time from the current sample.

**Implementation Details:**

- Computes the difference between the total CPU time (`total_diff`) and idle CPU time (`idle_diff`) for both the previous and current samples.
- The formula `cpu_utilization = ((1.0 - (float)idle_diff / total_diff)) * 100.0` is used to calculate the CPU utilization percentage.
- If `total_diff` is 0, the CPU utilization is set to 0.0.
- The pointer `cpu_utilization` is updated with the resulting value.

**Returns:**

No return value. Updates `cpu_utilization` with the calculated CPU usage.

**Example:**
```c
float cpu_utilization;
get_cpu_usage_diff(&cpu_utilization, prev_total, prev_idle, new_total, new_idle);
printf("CPU utilization: %.2f%%\n", cpu_utilization);
```


**`Function: draw_cpu_graph_struc`**

**Description:**

Draws the graph structure for visualizing CPU usage, including the y-axis (percentage scale) and x-axis (time scale).

**Parameters:**  
- `samples (int)` : The number of samples that will be displayed, determining the length of the x-axis. Calls the helper function `make_horiz_axis` to draw the horizontal axis.

**Returns:**

No return value. Only prints the graph structure to the terminal.

**Example:**
```c
draw_cpu_graph_struc(20);
```


**`Function: place_cpu_samp_pt`**

**Description:**

Places a sample point on the graph by plotting the CPU usage at a specific x and y coordinate. Adjusts the position based on the calculated CPU usage and sample index.

**Parameters:**  
- `percent_usage (float)` : The CPU usage in percentage that will determine the height (y-position) on the graph.
- `column (int)` : The x-axis position (sample index) where the data point will be placed.
- `spaces_from_top (int)` : The offset value to adjust the y-position to ensure accurate graph placement.

**Returns:**

No return value. The function places a graphical "?" on the terminal to represent the sample point.

**Example:**
```c
place_cpu_samp_pt(45.0, 5, 14);
```

| CPU Utilization (%) | Plotted Height (Bars) |
|------|------|
| 0 ≤ cpu_usage < 10 | 0 |
| 10 ≤ cpu_usage < 20 | 1 |
| 20 ≤ cpu_usage < 30 | 2 |
| 30 ≤ cpu_usage < 40 | 3 |
| ... | ... |
| 80 ≤ cpu_usage < 90 | 8 |
| 90 ≤ cpu_usage ≤ 100 | 9 |



<br>

> ### **Cores and Max Frequency Information**

**`Function: get_cpu_cores`**

**Description:**

This function retrieves the number of CPU cores available on the system by querying the system's configuration. It uses the `sysconf` function with the `_SC_NPROCESSORS_ONLN` argument to get the number of cores.

**Parameters:**  
- none

**Returns:**

Returns the number of CPU cores as an integer. If there is an error, the function prints an error message and terminates the program.

**Example:**
```c
int num_cores = get_cpu_cores();
```


**`Function: convert_khz_to_ghz`**

**Description:**

This function converts a CPU frequency value from kilohertz (kHz) to gigahertz (GHz).

**Parameters:**  
- `value (int)` : The CPU frequency in kilohertz.

**Returns:**

Returns the converted frequency value in gigahertz (float).

**Example:**
```c
float freq_in_ghz = convert_khz_to_ghz(2800000);
```


**`Function: get_max_cpu_freq`**

**Description:**

This function retrieves the maximum CPU frequency from the system by reading the `cpuinfo_max_freq` file located in `/sys/devices/system/cpu/cpu0/cpufreq/`. The function parses the value from the file and converts it from kilohertz (kHz) to gigahertz (GHz) using the `convert_khz_to_ghz` function.

**Parameters:**  
- none

**Returns:**

Returns the maximum CPU frequency in gigahertz (float). If there is an error reading the file or parsing the value, the function prints an error message and terminates the program.

**Example:**
```c
float max_cpu_freq = get_max_cpu_freq();
```


**`Function: print_rem_cores`**

**Description:**

This function is used to print the visual representation of the remaining CPU cores (less than 4 cores) in a grid format. It prints each core within a frame (a box), which is used to display partially filled rows of CPU cores.

**Parameters:**  
- `rem_cores (int)` : The number of remaining CPU cores to be displayed (less than 4).

**Returns:**

No return value. This function prints the remaining cores in a grid format.

**Example:**
```c
print_rem_cores(2);
```


**`Function: print_cores`**

**Description:**

This function prints a visual representation of all the CPU cores in a grid format. It divides the cores into full rows of 4 cores and prints them as boxes. If there are fewer than 4 remaining cores after dividing by 4, it calls `print_rem_cores` to handle the remaining cores.

**Parameters:**  
- `num_cpu_cores (int)` : The total number of CPU cores.

**Returns:**

No return value. This function prints the grid representation of all the CPU cores.

**Example:**
```c
print_cores(10); // Prints 10 cores in a grid format with full and partial rows
```


<br>

> ### **Additional Display Functions**

**`Function: display_samp_microsec`**

**Description:**

This function clears the screen and displays the number of samples and the time delay between samples in both microseconds and seconds.

**Parameters:**  
- `flags (Flags*)` : A pointer to a `Flags` struct that contains the values for `samples_value` and `tdelay_value`.

**Returns:**

No return value. This function prints the number of samples and the time delay between them.

**Example:**
```c
Flags flags = {20, 500000, 0, 0, 0};  
display_samp_microsec(&flags);
```


**`Function: display_memory_cpu`**

**Description:**

This function displays the memory and CPU utilization data based on the flags set in the `Flags` struct. It draws memory and CPU usage graphs, computes memory usage, and calculates CPU usage over a series of samples. The function updates the terminal periodically with the current memory and CPU usage.

**Parameters:**  
- `flags (Flags*)` : A pointer to a `Flags` struct containing flags for enabling memory and CPU usage display, as well as the number of samples and the time delay between each sample.

**Implementation Details:**

- Dynamically updates memory and CPU usage in real-time.
- Displays memory usage in gigabytes (GB).
- Plots real-time graphical representations for both CPU and memory utilization.
- Uses ANSI escape sequences for structured and clear terminal output.
- Includes a configurable delay between updates to control the refresh rate.
- Handles multiple samples for periodic monitoring and display.

**Returns:**

No return value. This function prints memory and CPU usage statistics and updates the terminal screen at regular intervals.

**Example:**
```c
Flags flags = {20, 500000, 1, 1, 0};  
display_memory_cpu(&flags);
```


<br>

### **The main function:**

- Initializes a `Flags` structure and parses command-line arguments.
- Displays the number of samples and delay using `display_samp_microsec`.
- If memory or CPU flags are set, it shows memory and CPU usage graphs via `display_memory_cpu`.
- If the `cores_flag` is set, it retrieves and displays the number of CPU cores and their maximum frequency, using `get_cpu_cores` and `get_max_cpu_freq`.
- Displays a graphical representation of the CPU cores with `print_cores`.
Ends with a return value of 0.

<br>

---

<br><br>


> # **How to Run the Program**
### **Operating System: Linux**

If compiled using the following command (or anything equivalent containing -std=c99 flag) on a linux system:
```
gcc -std=c99 -o ishani_cscb09_a1 ishani_cscb09_a1.c
```
- It will give a warning regarding usleep() function used in the display_memory_cpu() function:
```
warning: implicit declaration of function ‘usleep’; did you mean ‘sleep’? [-Wimplicit-function-declaration]
  462 |                 usleep(flags->tdelay_value);
      |                 ^~~~~~
      |                 sleep
```
- Ignore this warning and continue executing the program after compilation

If compiled without using -std=c99 flag, then no warning is given and normal execution can be conducted.

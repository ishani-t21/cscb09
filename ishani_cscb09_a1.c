#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>


typedef struct {
    int samples_value;
    int tdelay_value;
    int memory_flag;
    int cpu_flag;
    int cores_flag;
} Flags;


int whole_number(char *element_to_check) {
	char *element_end_ptr;
	long check = strtol(element_to_check, &element_end_ptr, 10);
	return *element_end_ptr == '\0';
}


void no_flags_given(Flags *flags) {
	// If no memory, cpu, or cores flags are provided, set them all to 1 by default
    if (!flags->memory_flag && !flags->cpu_flag && !flags->cores_flag) {
        flags->memory_flag = 1;
        flags->cpu_flag = 1;
        flags->cores_flag = 1;
    }
}


void parse_positional_args(int *index, int argc, char **argv, Flags *flags, int *pos_present) {
    // Check if both positional arguments (samples and tdelay) are provided
    if (*index < argc && whole_number(argv[*index])) {
        flags->samples_value = atoi(argv[*index]);

        if (flags->samples_value == 0) {
            fprintf(stderr, "Invalid samples positional argument\n");
            exit(1);
        }
        (*index)++;  // Move to next argument
        *pos_present = 1;

        // Check if the second argument is numeric (tdelay)
        if (*index < argc && whole_number(argv[*index])) {
            flags->tdelay_value = atoi(argv[*index]);

            if (flags->tdelay_value == 0) {
                fprintf(stderr, "Invalid tdelay positional argument\n");
                exit(1);
            }
            (*index)++;
            *pos_present = 2;
        }
    }
}


void parse_flag_args(int *index, int argc, char **argv, Flags *flags, int pos_present) {
    while (*index < argc) {
        if (strcmp(argv[*index], "--memory") == 0) {
            if (flags->memory_flag) {
                fprintf(stderr, "Error: Duplicate --memory flag\n");
                exit(1);
            }
            flags->memory_flag = 1;
        } 
        else if (strcmp(argv[*index], "--cpu") == 0) {
            if (flags->cpu_flag) {
                fprintf(stderr, "Error: Duplicate --cpu flag\n");
                exit(1);
            }
            flags->cpu_flag = 1;
        } 
        else if (strcmp(argv[*index], "--cores") == 0) {
            if (flags->cores_flag) {
                fprintf(stderr, "Error: Duplicate --cores flag\n");
                exit(1);
            }
            flags->cores_flag = 1;
        } 
        else if (strncmp(argv[*index], "--samples=", 10) == 0) {
            if (pos_present >= 1) {
                fprintf(stderr, "Error: Positional samples argument is specified. Don't use this flag.\n");
                exit(1);
            }
            flags->samples_value = atoi(argv[*index] + 10);
            if (flags->samples_value == 0 || !whole_number(argv[*index] + 10)) {
                fprintf(stderr, "Invalid samples flag argument\n");
                exit(1);
            }
        } 
        else if (strncmp(argv[*index], "--tdelay=", 9) == 0) {
            if (pos_present == 2) {
                fprintf(stderr, "Error: Positional tdelay argument is specified. Don't use this flag.\n");
                exit(1);
            }
            flags->tdelay_value = atoi(argv[*index] + 9);
            if (flags->tdelay_value == 0 || !whole_number(argv[*index] + 9)) {
                fprintf(stderr, "Invalid tdelay flag argument\n");
                exit(1);
            }
        } 
        else {
            fprintf(stderr, "Error: Unrecognized argument '%s'.\n", argv[*index]);
            exit(1);
        }
        (*index)++;  // Increment the pointer to move to the next argument
    }
}


void parse_flags(int argc, char **argv, Flags *flags) {
    flags->samples_value = 20; // default for samples
    flags->tdelay_value = 500000; // default for tdelay
    flags->memory_flag = 0;
    flags->cpu_flag = 0;
    flags->cores_flag = 0;

    int index = 1, pos_present = 0;

    // Parse positional arguments first
    parse_positional_args(&index, argc, argv, flags, &pos_present);
    // Parse flag-based arguments
    parse_flag_args(&index, argc, argv, flags, pos_present);

    no_flags_given(flags);
}


// Function to convert values from kb to gb
float convert_kb_to_gb(long kb) {
	return (float)kb / (1024 * 1024);
}


float get_mem_total() {
	long tot_memory = 0;
	
	FILE *mem_tot_file = fopen("/proc/meminfo", "r");
	
    if (mem_tot_file == NULL) {
        perror("Failed to open /proc/meminfo file");
        exit(1);
    }
	
	fscanf(mem_tot_file, "%*s %ld", &tot_memory);

	// Close the file
    if (fclose(mem_tot_file) != 0) {
        fprintf(stderr, "fclose failed\n");
        exit(1);  // Exit the program if the file can't be closed
    }
	
	return convert_kb_to_gb(tot_memory);
}


void get_free_memory(long *memory_free) {
	char f_line[256];
	
	FILE *mem_free_file = fopen("/proc/meminfo", "r");
	
	if (mem_free_file == NULL) {
		perror("Failed to open /proc/meminfo file");
		exit(1);
	}
	
	while (fgets(f_line, sizeof(f_line), mem_free_file)) {
		if (f_line == NULL) {
			perror("Error reading from /proc/meminfo file");
			fclose(mem_free_file);
			exit(1);  // Exit the program if the first line can't be read
        }
		if (strncmp(f_line, "MemFree:", 8) == 0) {
            sscanf(f_line, "%*s %ld", memory_free);
		}
	}
	
	// Close the file
    if (fclose(mem_free_file) != 0) {
        fprintf(stderr, "fclose failed\n");
        exit(1);  // Exit the program if the file can't be closed
    }
}


// Function to compute memory usage
void compute_memory_usage(float mem_tot_value, float *mem_used_address) {
    long memory_free = 0;

    // Read free memory value
    get_free_memory(&memory_free);

    // Calculate the memory used correctly
    *mem_used_address = mem_tot_value - convert_kb_to_gb(memory_free);
}


// Function to draw the horizontal axis of the graph
void make_horiz_axis(int samples) {
    for (int i = 0; i <= samples; i++) {
        printf("_"); // Horizontal axis
    }
    printf("\n\n\n");
}


void draw_mem_graph_struc(int samples, float *mem_total) {
    printf("\n");

    for (int i = 0; i < 11; i++) { // Draw vertical axis
        if (i == 0) {
            printf("%3.0f GB |\n", *mem_total);
        }
        printf("       |\n");
    }

    printf("  0 GB ");
    make_horiz_axis(samples); // Draw horizontal axis
}


void place_mem_samp_pt(float total_memory, float mem_usage, int column) {
	int row = 17 - (int)((mem_usage / total_memory) * 12); 
    printf("\x1b[%d;%df#", row, 9 + column);
}


// Function to extract CPU times from a given line
void parse_cpu_times(char* line_ptr, long *total_cpu_time, long *idle_time) {
    char* cpy_line_ptr = line_ptr;
    int num_values = 0;
    long time_to_add = 0;

    *total_cpu_time = 0; // Reset values before accumulation
    *idle_time = 0;

    while (*cpy_line_ptr != '\0' && num_values < 10) {
        if (isdigit(*cpy_line_ptr)) {
            time_to_add = strtol(cpy_line_ptr, &cpy_line_ptr, 10);
            *total_cpu_time += time_to_add;

            // Assign the idle time on the 4th index
            if (num_values == 3) {
                *idle_time = time_to_add;
            }

            num_values++;
        }
        cpy_line_ptr++;
    }
}


// Function to get CPU utilization
void get_cpu_util(long *total_cpu_time, long *idle_time) {
    char cpu_line_space[256];

    // Open the file
    FILE* cpu_usage_file = fopen("/proc/stat", "r");
    if (cpu_usage_file == NULL) {
        perror("Failed to open /proc/stat file");
        exit(1);
    }

    // Read the first line from the file
    if (fgets(cpu_line_space, sizeof(cpu_line_space), cpu_usage_file) == NULL) {
        perror("Error reading from /proc/stat file");
        fclose(cpu_usage_file);
        exit(1);
    }

    fclose(cpu_usage_file);

    // Parse the CPU usage statistics
    parse_cpu_times(cpu_line_space, total_cpu_time, idle_time);
}


// Compute CPU utilization over time
void get_cpu_usage_diff(float *cpu_utilization, long prev_total, long prev_idle, long new_total, long new_idle) {
	
	long total_diff = new_total - prev_total;
	long idle_diff = new_idle - prev_idle;
	
	if (total_diff == 0) {
		*cpu_utilization = 0.0;
	}
	else {
		*cpu_utilization = (1.0 - ((float)idle_diff / total_diff)) * 100.0;
	}
}


// Function to draw the CPU usage graph structure
void draw_cpu_graph_struc(int samples) {
	printf("\n");
	printf(" 100%%  |\n");
	
	for (int i = 0; i < 9; i++) { // Draw vertical axis
        printf("       |\n");
    }
	
	printf("   0%%  ");
	
	make_horiz_axis(samples); // Draw horizontal axis
}


void place_cpu_samp_pt(float percent_usage, int column, int spaces_from_top) {
    int row = spaces_from_top - (int)(percent_usage / 10); 
    printf("\x1b[%d;%df$", row, 9 + column);
	//put offset parameter; 28 for memory and cpu considered
}


int get_cpu_cores() {
    long n_cores = sysconf(_SC_NPROCESSORS_ONLN);
    
    if (n_cores == -1) {  // Check if sysconf failed
        perror("Error getting number of cores");
        exit(1);  // Return error code if sysconf fails
    }
    
    return (int)n_cores;  // Return number of cores as integer
}


float convert_khz_to_ghz(int value) {
    return (float)value / 1000000.0;
}


float get_max_cpu_freq() {
    FILE *max_freq_file = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
    long int max_freq = 0;

    if (max_freq_file == NULL) {
        perror("Error opening frequency file");
        exit(1);
    }

    if (fscanf(max_freq_file, "%ld", &max_freq) != 1) {
        perror("Error reading max frequency");
        fclose(max_freq_file);
        exit(1);
    }

    fclose(max_freq_file);

    return convert_khz_to_ghz(max_freq);
}


// Print the remaining cores (less than 4)
void print_rem_cores(int rem_cores) {
	
	for (int i = 1; i < rem_cores; i++) {
		printf("+---+  ");
	}
	printf("+---+\n");

	for (int i = 1; i < rem_cores; i++) {
		printf("|   |  ");
	}
	printf("|   |\n");

	for (int i = 1; i < rem_cores; i++) {
		printf("+---+  ");
	}
	printf("+---+\n");
}


// Print the number of cores with by putting 4 in each row
void print_cores(int num_cpu_cores) {
	int full_rows = 0;
	int rem_cores = 0;
	
	full_rows = (int) (num_cpu_cores / 4);
	rem_cores = num_cpu_cores % 4;
	
	// Calculate the number of full rows and remaining cores
	for (int i = 0; i < full_rows; i++) {
		printf("+---+  +---+  +---+  +---+\n");
		printf("|   |  |   |  |   |  |   |\n");
		printf("+---+  +---+  +---+  +---+\n");
	}
	if (rem_cores > 0) {
		print_rem_cores(rem_cores);
	}
}


void display_samp_microsec(Flags* flags) {
	int samples = flags->samples_value;
    int tdelay = flags->tdelay_value;
	
	// Clear the screen and show initial info
    printf("\033[H\033[J\n");
    printf("Nbr of samples: %d -- every %d microSecs (%.6f secs)\n\n", samples, tdelay, (float)tdelay / 1000000);
}


void display_memory_cpu(Flags* flags) {

	float mem_total_value = 0.0;
	float mem_used_value = 0.0;
	
	long prev_total = 0, prev_idle = 0;
    long new_total = 0, new_idle = 0;
	float cpu_utilization = 0;
	
	int spaces_from_top = 14;
	int row_to_print = 4;
	
	mem_total_value = get_mem_total();
	
	if (flags->memory_flag) {
		printf("\n");
		draw_mem_graph_struc(flags->samples_value, &mem_total_value);
	}
	
	if (flags->cpu_flag) {
		draw_cpu_graph_struc(flags->samples_value);
		// Initial CPU reading
		get_cpu_util(&prev_total, &prev_idle);
		printf("\n");
	}
	
	if (flags->memory_flag && flags->cpu_flag) {
		spaces_from_top = 31;
		row_to_print = 20;
    }
	
	for (int i = 0; i < flags->samples_value; i++) {
		if (flags->memory_flag) {
			compute_memory_usage(mem_total_value, &mem_used_value);
			
			printf("\033[%d;1H\033[K> Memory  %2.2f GB\n", 4, mem_used_value);
			fflush(stdout);
			
			place_mem_samp_pt(mem_total_value, mem_used_value, i);			
		}
		if (flags->cpu_flag) {
			get_cpu_util(&new_total, &new_idle);
			get_cpu_usage_diff(&cpu_utilization, prev_total, prev_idle, new_total, new_idle);
			
			printf("\033[%d;1H\033[K> CPU  %2.2f %%\n", row_to_print, cpu_utilization);
			fflush(stdout);
			
			place_cpu_samp_pt(cpu_utilization, i, spaces_from_top);
			prev_total = new_total;
			prev_idle = new_idle;
		}
		
		usleep(flags->tdelay_value);
	}
}	


int main(int argc, char **argv) {
    Flags flags;
	
	int cpu_cores = 0;
	float max_freq = 0.0;
	
	parse_flags(argc, argv, &flags);
	
	display_samp_microsec(&flags);
	
	if (flags.memory_flag || flags.cpu_flag) {
		display_memory_cpu(&flags);
		printf("\n\n\n\n\n\n");
	}
	
	if (flags.cores_flag) {
		cpu_cores = get_cpu_cores();
		max_freq = get_max_cpu_freq();
		printf("> Number of Cores: %d @ %.2f GHz\n\n", cpu_cores, max_freq);
		print_cores(cpu_cores);
	}
	printf("\n\n\n");
	
	return 0;
}


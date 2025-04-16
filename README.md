# Out of Order and Causally Correct: Ready-Event Discovery through Data-Dependence Analysis

[![License: AGPL v3 or later](https://img.shields.io/badge/License-AGPL_v3_or_later-blue.svg)](https://www.gnu.org/licenses/agpl-3.0)

## ACM SIGSIM-PADS 25 (2025)

This repository contains the reproducibility artifact for our paper "Out of Order and Causally Correct: Ready-Event Discovery through Data-Dependence Analysis" presented at ACM SIGSIM-PADS 25.

## Abstract

Data-dependence analysis can identify causally-unordered events in a pending event set.  The execution of these events is independent from all other scheduled events, making them ready for execution.  These events can be executed out of order or in parallel. This approach may find and utilize more parallelism than spatial-decomposition parallelization methods, which are limited by the number of subdomains and by synchronization methods.  This work provides formal definitions that use data-dependence analysis to find causally-unordered events and uses these definitions to measure parallelism in several discrete-event simulation models.
A variant of the event-graph formalism is proposed, which assists with identifying ready events, by more clearly visualizing data dependencies between event types. Data dependencies between two event types may be direct or indirect, where the latter case considers the scheduling of intermediate events. Data dependencies and scheduling dependencies in a discrete-event simulation model are used to define time-interval limits that support the identification of events that are ready for execution. Experimental results from serial simulation testing demonstrate the availability of numerous events that are ready for execution, depending on model characteristics.  The mean size of the ready-event set varies from about 1.5 to 110 for the tested models, depending on the model type, the size of the model, and delay distribution parameters.  These findings support future work to develop a parallel capability to dynamically identify and execute ready events in a multi-threaded environment.

## Setup with Docker

### Prerequisites
- Docker
- Docker Compose (optional, for easier management)

### Building and Running with Docker

1. **Build the Docker image and start the container**:
   ```bash
   docker build -t pads-artifact .
   docker-compose up -d
   ```

2. **Enter the container and run the scripts**:
   ```bash
   # Enter the container
   sudo docker-compose exec pads-artifact bash
   
   # Inside the container, make the script executable and run it
   chmod +x run.sh
   ./run.sh
   ```

   This will:
   - Compile the code
   - Run the simulation experiments
   - Generate plots based on the results
   - Generate tables from the simulation data

## Artifact Structure

- `OoO_Sim`: Main simulation binary (compiled from C++ sources)
- `PADS_resilient_auto_testing.py`: Script to run simulations with various parameters
- `PADS_plot_REs_64.py`, `PADS_plot_REs_729.py`, `PADS_plot_order_diffs_64.py`: Scripts to generate figures
- `PADS_generate_table_2.py`, `PADS_generate_table_5.py`, `PADS_generate_table_6.py`: Scripts to generate tables
- Supporting directories:
  - `exec_orders/`: Execution order data
  - `input_files/`: Simulation input configurations
  - `output_files/`: Simulation outputs
  - `params_files/`: Parameter files for simulations
  - `traces/`: Trace files from simulation runs
  - `ITL_tables/`: Tables for analysis

## Running Experiments

1. **Compile the code** (already done in the Docker build):
   ```bash
   make
   ```

2. **Run the automatic testing script**:
   ```bash
   python3 PADS_resilient_auto_testing.py
   ```
   
   > **Note:** In `PADS_resilient_auto_testing.py`, the `max_workers` parameter controls how many simulation runs are executed in parallel. The default is the maximum number of cores. Adjust this value based on your system's CPU and memory constraints. Using 12 cores on a Snapdragon X Elite machine, testing uses about 6 GB of RAM and takes about 5 hours.
   >
   > **Important:** The `DELETE_TRACES` variable may be set to `False` if you want to keep the simulation traces, but this requires over 200 GB of hard drive space. Set to `True` by default.

3. **Generate plots** after simulations complete:
   ```bash
   python3 PADS_plot_REs_64.py
   python3 PADS_plot_REs_729.py
   python3 PADS_plot_order_diffs_64.py
   ```

4. **Run the simple DIR simulation**:
   ```bash
   python3 PADS25_python_sim_nQ_DIR.py
   ```

5. **Generate tables** (must be done after generating figures):
   ```bash
   python3 PADS25_python_sim_nQ_DIR.py | python3 PADS_generate_table_2.py
   python3 PADS_generate_table_5.py
   python3 PADS_generate_table_6.py
   ```
   > **Note:** For tables 5 and 6, the plotting scripts (Step 3) must be run first before running the table-generation scripts.

## Artifact Description

This artifact contains the code used to run the simulations described in the paper. The main components are:

1. **OoO_Sim**: A C++ implementation of the simulation engine that supports both in-order and out-of-order execution.

2. **Network Models**:
   - 1D Ring Network (`Ring_1D/`)
   - 2D Grid Network with Von Neumann neighborhood (`Grid_VN2D/`)
   - 3D Grid Network with Von Neumann neighborhood (`Grid_VN3D/`)
   - 3D Torus Network (`Torus_3D/`)

3. **Analysis Scripts**:
   - `PADS_resilient_auto_testing.py`: Runs simulations with different network configurations and parameters
   - `PADS_plot_REs_64.py`: Generates plots for 64-node networks
   - `PADS_plot_REs_729.py`: Generates plots for 729-node networks
   - `PADS_plot_order_diffs_64.py`: Analyzes execution order differences
   - `PADS25_python_sim_nQ_DIR.py`: Simple Python simulation for the DIR algorithm
   - `PADS_generate_table_2.py`: Generates Table 2 from DIR simulation results
   - `PADS_generate_table_5.py`: Generates Table 5 from plot data
   - `PADS_generate_table_6.py`: Generates Table 6 from plot data

## Figure and Table Reproduction

The artifact reproduces the following figures and tables from the paper:

### Figure 6
Generated by `PADS_plot_REs_64.py` and `PADS_plot_REs_729.py`

### Figure 7
Generated by `PADS_plot_order_diffs_64.py`

### Table 2
Generated by running `PADS25_python_sim_nQ_DIR.py | python3 PADS_generate_table_2.py`

### Table 5
Generated by `PADS_generate_table_5.py` (after running `PADS_plot_REs_64.py` and `PADS_plot_REs_729.py`)

### Table 6
Generated by `PADS_generate_table_6.py` (after running `PADS_plot_order_diffs_64.py`)

## Extending the Artifact

To run with different parameters, modify the `NETWORK_CONFIGS`, `HOP_RADIUS_VALUES`, or other parameters in `PADS_resilient_auto_testing.py`.

## License

This project is licensed under the GNU Affero General Public License v3.0 or later - see the [LICENSE](LICENSE) file for details.

This license requires that modifications to this code must be made available under the same license, even when the code is only used to provide a service over a network without distributing the actual code.
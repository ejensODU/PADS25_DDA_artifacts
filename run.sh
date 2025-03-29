#!/bin/bash

# This script automates the execution of the PADS artifact
# It can be run inside the Docker container

set -e  # Exit on error

# Create required directories if they don't exist
mkdir -p exec_orders input_files output_files params_files ITL_tables traces

# Step 1: Compile the code (in case it wasn't already compiled)
echo "Compiling OoO_Sim..."
make

# Step 2: Run the simulations
echo "Running simulation set..."
python3 PADS_resilient_auto_testing.py

# Step 3: Generate plots
echo "Generating plots..."
python3 PADS_plot_REs_64.py
python3 PADS_plot_REs_729.py
python3 PADS_plot_order_diffs_64.py

# Step 4: Run the simple DIR simulation
echo "Running DIR simulation..."
python3 PADS25_python_sim_nQ_DIR.py

echo "All steps completed successfully!"
echo "Generated plots:"
ls -la *.pdf

echo
echo "To view the plots, they need to be copied from the container to your host machine."
echo "From your host, run:"
echo "  docker cp <container_id>:/app/PADS_mean_REs_64.pdf ."
echo "  docker cp <container_id>:/app/PADS_mean_REs_729.pdf ."
echo "  docker cp <container_id>:/app/PADS_exec_order_diffs_64.pdf ."
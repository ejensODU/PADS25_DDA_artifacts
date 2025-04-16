#!/bin/bash

# This script automates the execution of the PADS artifact
# It can be run inside the Docker container

set -e  # Exit on error

# Create required directories if they don't exist
mkdir -p exec_orders input_files output_files params_files ITL_tables traces

# Step 1: Compile the code (in case it wasn't already compiled)
# Check if executable exists before compiling
if [ ! -f "OoO_Sim" ]; then
  echo "Compiling OoO_Sim..."
  make
else
  echo "OoO_Sim already compiled, skipping compilation step"
fi

# Step 2: Run the simulations
echo "Running simulation set..."
python3 PADS_resilient_auto_testing.py

# Step 3: Generate plots
echo "Generating plots..."
python3 PADS_plot_REs_64.py
python3 PADS_plot_REs_729.py
python3 PADS_plot_order_diffs_64.py

# Step 4: Generate tables
echo "Generating tables..."
python3 PADS25_python_sim_nQ_DIR.py | python3 PADS_generate_table_2.py
python3 PADS_generate_table_5.py
python3 PADS_generate_table_6.py

echo "All steps completed successfully!"
echo "Generated plots and tables:"
ls -la *.pdf *.csv

echo
echo "To view the plots and tables, they need to be copied from the container to your host machine."
echo "From your host, run:"
echo "  docker cp <container_id>:/app/PADS_mean_REs_64.pdf ."
echo "  docker cp <container_id>:/app/PADS_mean_REs_729.pdf ."
echo "  docker cp <container_id>:/app/PADS_exec_order_diffs_64.pdf ."
echo "  docker cp <container_id>:/app/table_2.csv ."
echo "  docker cp <container_id>:/app/table_5.csv ."
echo "  docker cp <container_id>:/app/table_6.csv ."
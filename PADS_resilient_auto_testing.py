import subprocess
import re
import time
import pandas as pd
import os
import concurrent.futures
import queue
import threading
import filecmp
import csv
import sys
from datetime import datetime

# NETWORK_CONFIGS = [
   # {'model': '1D_ring_network', 'size': [16]},
   # {'model': 'VN2D_grid_network', 'size': [4, 4]},
   # {'model': 'VN3D_grid_network', 'size': [2, 2, 2]},
   # {'model': '3D_torus_network', 'size': [2, 2, 2]},
   # {'model': '1D_ring_network', 'size': [64]},
   # {'model': 'VN2D_grid_network', 'size': [8, 8]},
   # {'model': 'VN3D_grid_network', 'size': [4, 4, 4]},
   # {'model': '3D_torus_network', 'size': [4, 4, 4]}
# ]

NETWORK_CONFIGS = [
   {'model': '1D_ring_network', 'size': [64]},
   {'model': 'VN2D_grid_network', 'size': [8, 8]},
   {'model': 'VN3D_grid_network', 'size': [4, 4, 4]},
   {'model': '3D_torus_network', 'size': [4, 4, 4]},
   {'model': 'VN2D_grid_network', 'size': [27, 27]},
   {'model': 'VN3D_grid_network', 'size': [9, 9, 9]},
   {'model': '3D_torus_network', 'size': [9, 9, 9]}
]

HOP_RADIUS_VALUES = [1, 2, 4]
INTRA_ARRIVAL_DELAYS = [10, 12, 16]
SERVICE_DELAYS = [2, 3, 4]
TRANSIT_DELAY_RATIOS = [0.25, 0.5, 0.75, 1.0, 1.25, 1.5, 1.75, 2.0, 2.25, 2.5]
RESULTS_FILE = 'PADS_network_simulation_results.csv'
RESULTS_LOCK = threading.Lock()

def get_experiment_key(config, dist_seed, params_config, hop_radius=None):
    """Generate a unique key for each experiment configuration"""
    return (
        config['model'],
        '_'.join(map(str, config['size'])),
        str(hop_radius) if hop_radius is not None else 'N/A',
        dist_seed,
        params_config
    )

def load_completed_experiments():
    """Load already completed experiments from the results file"""
    completed_experiments = set()
    if os.path.exists(RESULTS_FILE):
        try:
            df = pd.read_csv(RESULTS_FILE)
            for _, row in df.iterrows():
                key = (
                    row['model'],
                    row['size'],
                    str(row['hop_radius']),
                    row['dist_seed'],
                    row['params_config']
                )
                completed_experiments.add(key)
        except Exception as e:
            print(f"Warning: Error reading results file: {e}")
            print("Proceeding with empty completed experiments set")
    return completed_experiments

def get_base_filename(config, dist_seed, params_config, hop_radius=None):
    """Generate consistent base filename pattern"""
    size_str = '_'.join(map(str, config['size']))
    hop_str = f"_hop_{hop_radius}" if hop_radius is not None else ""
    return f"{config['model']}_size_{size_str}{hop_str}_seed_{dist_seed}_params_{params_config}"

def setup_results_file():
    """Create results file with headers if it doesn't exist"""
    headers = [
        'model', 'size', 'hop_radius', 'dist_seed', 'params_config', 'transit_delay_ratio',
        'io_events_executed', 'ooo_events_executed', 'mean_ready_events',
        'mean_event_set_size', 'traces_match', 'io_mean_packet_time',
        'ooo_mean_packet_time', 'timestamp'
    ]
    if not os.path.exists(RESULTS_FILE):
        with open(RESULTS_FILE, 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=headers)
            writer.writeheader()

def write_result(result):
    """Write a single result to the CSV file with lock protection"""
    with RESULTS_LOCK:
        with open(RESULTS_FILE, 'a', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=list(result.keys()))
            writer.writerow(result)

def CompareTraces(dir1, dir2):
    """Compare trace files between in-order and out-of-order runs"""
    if not os.path.exists(dir1) or not os.path.exists(dir2):
        print(f"Missing trace directory: {dir1} or {dir2}")
        return False

    files1 = set(f for f in os.listdir(dir1) if os.path.isfile(os.path.join(dir1, f)))
    files2 = set(f for f in os.listdir(dir2) if os.path.isfile(os.path.join(dir2, f)))

    if files1 != files2:
        print(f"Different files in directories: {dir1} and {dir2}")
        return False

    for filename in files1:
        if not filecmp.cmp(os.path.join(dir1, filename), os.path.join(dir2, filename), shallow=False):
            return False

    return True

def extract_metrics(output):
    """Extract all required metrics from simulation output"""
    metrics = {}

    # Extract events executed
    events_match = re.search(r"events executed: ([\d\.]+)", output)
    metrics['events_executed'] = float(events_match.group(1)) if events_match else None

    # Extract mean packet time
    time_match = re.search(r"Mean packet network time: ([\d\.]+)", output)
    metrics['mean_packet_time'] = float(time_match.group(1)) if time_match else None

    # For OOO runs, extract additional metrics
    re_match = re.search(r"mean size ready events: ([\d\.]+)", output)
    metrics['mean_ready_events'] = float(re_match.group(1)) if re_match else None

    es_match = re.search(r"mean E size: ([\d\.]+)", output)
    metrics['mean_event_set_size'] = float(es_match.group(1)) if es_match else None

    return metrics

def create_delay_params_file(config, params_config, dist_seed, hop_radius, exec_type):
    """Create distribution parameter file with given configuration"""
    mean_service = sum(SERVICE_DELAYS) / len(SERVICE_DELAYS)
    ratio = TRANSIT_DELAY_RATIOS[params_config % len(TRANSIT_DELAY_RATIOS)]
    mean_transit = mean_service * ratio

    transit_min = mean_transit * (SERVICE_DELAYS[0] / mean_service)
    transit_mode = mean_transit * (SERVICE_DELAYS[1] / mean_service)
    transit_max = mean_transit * (SERVICE_DELAYS[2] / mean_service)

    base_name = get_base_filename(config, dist_seed, params_config, hop_radius)
    filename = f"params_files/params_{base_name}_exec_{exec_type}.txt"

    with open(filename, 'w') as f:
        f.write(f"{INTRA_ARRIVAL_DELAYS[0]} {INTRA_ARRIVAL_DELAYS[1]} {INTRA_ARRIVAL_DELAYS[2]}\n")
        f.write(f"{SERVICE_DELAYS[0]} {SERVICE_DELAYS[1]} {SERVICE_DELAYS[2]}\n")
        f.write(f"{transit_min:.2f} {transit_mode:.2f} {transit_max:.2f}\n")

    return filename

def create_network_input_file(config, dist_seed, exec_type, params_file, params_config, hop_radius=None):
    """Create input file based on network configuration"""
    input_content = f"model_name : {config['model']}\n"

    if config['model'] == '1D_ring_network':
        input_content += f"ring_size : {config['size'][0]}\n"
    else:
        input_content += f"grid_size_x : {config['size'][0]}\n"
        input_content += f"grid_size_y : {config['size'][1]}\n"
        if len(config['size']) > 2:
            input_content += f"grid_size_z : {config['size'][2]}\n"
        input_content += f"hop_radius : {hop_radius}\n"

    input_content += f"""num_servers_per_network_node : 1
max_num_intra_arrive_events : 100
max_sim_time : 1000000
num_threads : 1
dist_seed : {dist_seed}
num_serial_OoO_execs : {exec_type}
dist_params_file : {params_file}
"""

    base_name = get_base_filename(config, dist_seed, params_config, hop_radius)
    input_file = f"input_files/input_{base_name}_exec_{exec_type}.txt"

    with open(input_file, 'w') as f:
        f.write(input_content)
    return input_file

def get_trace_folder_name(config, exec_type, dist_seed, params_config, hop_radius=None):
    """Generate trace folder name based on configuration"""
    base_name = get_base_filename(config, dist_seed, params_config, hop_radius)
    return f"traces/trace_{base_name}_exec_{exec_type}_threads_1_servers_1"

def run_single_simulation(config, dist_seed, exec_type, params_config, hop_radius=None):
    """Run a single simulation with given parameters"""
    # Create params file for this execution
    params_file = create_delay_params_file(config, params_config, dist_seed, hop_radius, exec_type)

    # Create input file
    input_file = create_network_input_file(config, dist_seed, exec_type, params_file, params_config, hop_radius)

    # Create unique output filename
    base_name = get_base_filename(config, dist_seed, params_config, hop_radius)
    output_file = f"output_files/output_{base_name}_exec_{exec_type}.txt"

    try:
        result = subprocess.run(f'./OoO_Sim {input_file} > {output_file}',
                              shell=True, capture_output=True, text=True)

        if result.returncode == 0:
            with open(output_file, 'r') as f:
                output = f.read()
            return extract_metrics(output)
        else:
            print(f"Error in simulation: {result.stderr}")
            return None
    except Exception as e:
        print(f"Exception in simulation: {e}")
        return None

def run_experiment(config, dist_seed, params_config, hop_radius=None, completed_experiments=None):
    """Run experiment if not already completed"""
    experiment_key = get_experiment_key(config, dist_seed, params_config, hop_radius)

    if completed_experiments and experiment_key in completed_experiments:
        print(f"Skipping completed experiment: Model={config['model']}, "
              f"Size={config['size']}, "
              f"Hop Radius={hop_radius if hop_radius is not None else 'N/A'}, "
              f"Seed={dist_seed}, Params={params_config}")
        return None

    print(f"Running new experiment: Model={config['model']}, "
          f"Size={config['size']}, "
          f"Hop Radius={hop_radius if hop_radius is not None else 'N/A'}, "
          f"Seed={dist_seed}, Params={params_config}")

    # Run in-order simulation (exec_type = 0)
    io_metrics = run_single_simulation(config, dist_seed, 0, params_config, hop_radius)
    if not io_metrics:
        return None

    # Run out-of-order simulation (exec_type = -10)
    ooo_metrics = run_single_simulation(config, dist_seed, -10, params_config, hop_radius)
    if not ooo_metrics:
        return None

    # Compare traces
    io_trace_folder = get_trace_folder_name(config, 0, dist_seed, params_config, hop_radius)
    ooo_trace_folder = get_trace_folder_name(config, -10, dist_seed, params_config, hop_radius)
    traces_match = CompareTraces(io_trace_folder, ooo_trace_folder)
    if not traces_match:
        print("Error: traces do not match!")
        sys.exit(0)

    result = {
        'model': config['model'],
        'size': '_'.join(map(str, config['size'])),
        'hop_radius': hop_radius if hop_radius is not None else 'N/A',
        'dist_seed': dist_seed,
        'params_config': params_config,
        'transit_delay_ratio': TRANSIT_DELAY_RATIOS[params_config % len(TRANSIT_DELAY_RATIOS)],
        'io_events_executed': io_metrics['events_executed'],
        'ooo_events_executed': ooo_metrics['events_executed'],
        'mean_ready_events': ooo_metrics['mean_ready_events'],
        'mean_event_set_size': ooo_metrics['mean_event_set_size'],
        'traces_match': traces_match,
        'io_mean_packet_time': io_metrics['mean_packet_time'],
        'ooo_mean_packet_time': ooo_metrics['mean_packet_time'],
        'timestamp': datetime.now().isoformat()
    }

    # Write result immediately
    write_result(result)
    return result

def main():
    setup_results_file()
    max_workers = 4  # Adjust based on your system

    # Load completed experiments at startup
    completed_experiments = load_completed_experiments()
    print(f"Found {len(completed_experiments)} completed experiments")

    with concurrent.futures.ThreadPoolExecutor(max_workers=max_workers) as executor:
        futures = []
        for config in NETWORK_CONFIGS:
            for dist_seed in range(10):
                for params_config in range(10):
                    if config['model'] == '1D_ring_network':
                        # Run 1D ring network without hop radius
                        futures.append(
                            executor.submit(
                                run_experiment,
                                config,
                                dist_seed,
                                params_config,
                                None,
                                completed_experiments
                            )
                        )
                    else:
                        # Run other models with different hop radius values
                        for hop_radius in HOP_RADIUS_VALUES:
                            futures.append(
                                executor.submit(
                                    run_experiment,
                                    config,
                                    dist_seed,
                                    params_config,
                                    hop_radius,
                                    completed_experiments
                                )
                            )

        # Wait for all futures to complete
        completed_count = 0
        skipped_count = 0
        for future in concurrent.futures.as_completed(futures):
            try:
                result = future.result()
                if result:
                    completed_count += 1
                    print(f"Completed experiment: {result['model']}, "
                          f"Hop Radius={result['hop_radius']}, "
                          f"Seed={result['dist_seed']}, "
                          f"Params={result['params_config']}")
                else:
                    skipped_count += 1
            except Exception as e:
                print(f"Experiment failed with error: {e}")

        print(f"\nExecution complete:")
        print(f"- Previously completed experiments: {len(completed_experiments)}")
        print(f"- Newly completed experiments: {completed_count}")
        print(f"- Skipped experiments: {skipped_count}")
        print(f"- Failed experiments: {len(futures) - completed_count - skipped_count}")

if __name__ == "__main__":
    main()

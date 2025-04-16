import pandas as pd
import numpy as np

# Required LaTeX packages for the table:
# \\usepackage{multirow}    % For cells spanning multiple rows
# \\usepackage{colortbl}    % For cell background colors
# \\usepackage{xcolor}      % For color definitions (lightgray)
# \\usepackage{array}       % For advanced table formatting

def read_csv_data(file_path):
    """
    Read the CSV file and return a processed DataFrame.
    """
    # Read the CSV file, properly handling the hop_radius column
    df = pd.read_csv(file_path)
    
    # Add a column for total node count (for easier filtering)
    def calculate_node_count(size_spec):
        if '_' in str(size_spec):
            dimensions = [int(dim) for dim in str(size_spec).split('_')]
            return np.prod(dimensions)
        return int(size_spec)
    
    df['node_count'] = df['size'].apply(calculate_node_count)
    
    # Map the network model names to match the table
    model_map = {
        '1D_ring_network': 'Ring (1D)',
        'VN2D_grid_network': 'Mesh (2D)',
        'VN3D_grid_network': 'Mesh (3D)',
        '3D_torus_network': 'Torus (3D)'
    }
    
    df['network'] = df['model'].map(model_map)
    
    # Convert hop_radius to string type first (avoid the warning)
    df['hop_radius'] = df['hop_radius'].astype(str)
    
    # Replace 'nan' with 'N/A' for all hop_radius values
    df['hop_radius'] = df['hop_radius'].replace('nan', 'N/A')
    
    return df

def calculate_statistics(df):
    """
    Calculate statistics for each configuration as described:
    1. Group by model, size, hop radius, and params_config
    2. Calculate average mean_ready_events across all dist_seed values for each group
    3. Then for each model, size, hop radius combo, find min, median, max across different params_configs
    """
    # First, group by model, size, hop radius, and params_config
    # Calculate average mean_ready_events across all dist_seed values
    grouped_df = df.groupby(['network', 'node_count', 'hop_radius', 'params_config'])['mean_ready_events'].mean().reset_index()
    
    # Print the first few rows of grouped data to verify it contains Ring network
    print("\nFirst few rows of grouped data:")
    pd.set_option('display.max_columns', None)  # Show all columns
    print(grouped_df.head(5))
    
    # Now, group by model, size, hop radius
    # Find min, median, max of these averages across different params_configs
    stats = []
    
    # Sort the grouped dataframe to make sure we process Ring data first
    networks_order = {'Ring (1D)': 1, 'Mesh (2D)': 2, 'Mesh (3D)': 3, 'Torus (3D)': 4}
    grouped_df['network_order'] = grouped_df['network'].map(networks_order)
    grouped_df = grouped_df.sort_values('network_order')
    
    # Process network by network to ensure we don't miss any
    for network in sorted(grouped_df['network'].unique(), key=lambda x: networks_order.get(x, 999)):
        print(f"\nProcessing network: {network}")
        network_df = grouped_df[grouped_df['network'] == network]
        
        for size in sorted(network_df['node_count'].unique()):
            size_df = network_df[network_df['node_count'] == size]
            
            for hop_radius in sorted(size_df['hop_radius'].unique()):
                hr_df = size_df[size_df['hop_radius'] == hop_radius]
                
                if not hr_df.empty:
                    ready_events = hr_df['mean_ready_events'].values
                    ready_events_sorted = sorted(ready_events)
                    
                    # Calculate min and max
                    min_val = np.min(ready_events)
                    max_val = np.max(ready_events)
                    
                    # Calculate median using different methods for comparison
                    n = len(ready_events_sorted)
                    mid = n // 2
                    
                    # Traditional median calculation
                    if n % 2 == 0:
                        median_traditional = (ready_events_sorted[mid-1] + ready_events_sorted[mid]) / 2
                    else:
                        median_traditional = ready_events_sorted[mid]
                    
                    # Alternative median calculation methods
                    median_lower = ready_events_sorted[mid-1] if n % 2 == 0 else ready_events_sorted[mid]
                    median_higher = ready_events_sorted[mid] if n % 2 == 0 else ready_events_sorted[mid]
                    
                    # NumPy median
                    median_numpy = np.median(ready_events)
                    
                    # Print all median values for this configuration
                    print(f"Network: {network}, Size: {size}, Hop Radius: {hop_radius}")
                    print(f"  Ready events (sorted): {[f'{x:.2f}' for x in ready_events_sorted]}")
                    print(f"  Number of values: {n}")
                    print(f"  Min: {min_val:.2f}")
                    print(f"  Median (traditional): {median_traditional:.2f}")
                    print(f"  Median (lower): {median_lower:.2f}")
                    print(f"  Median (higher): {median_higher:.2f}")
                    print(f"  Median (numpy): {median_numpy:.2f}")
                    print(f"  Max: {max_val:.2f}")
                    
                    # Use traditional median in the stats
                    stats.append({
                        'network': network,
                        'size': size,
                        'hop_radius': hop_radius,
                        'min': min_val,
                        'median': median_numpy,
                        'max': max_val
                    })
    
    # Debug: Check if we have the Ring model in our stats
    networks_in_stats = {stat['network'] for stat in stats}
    sizes_in_stats = {(stat['network'], stat['size']) for stat in stats}
    print(f"\nNetworks found in stats: {networks_in_stats}")
    print(f"Network-size combinations in stats: {sizes_in_stats}")
    
    return pd.DataFrame(stats)

def get_required_latex_packages():
    """
    Returns a string of LaTeX package requirements for the table.
    
    The table requires the following LaTeX packages:
    - multirow: For cells spanning multiple rows
    - colortbl: For cell background colors
    - xcolor: For color definitions (lightgray)
    - array: For advanced table formatting
    
    These packages should be included in the preamble of your LaTeX document:
    \\usepackage{multirow}
    \\usepackage{colortbl}
    \\usepackage{xcolor}
    \\usepackage{array}
    """
    packages = r"""
Required LaTeX packages for this table:
\usepackage{multirow}    % For cells spanning multiple rows
\usepackage{colortbl}    % For cell background colors
\usepackage{xcolor}      % For color definitions (lightgray)
\usepackage{array}       % For advanced table formatting
"""
    return packages


def generate_latex_table(stats_df):
    """
    Generate the LaTeX table code based on the statistics DataFrame.
    """
    latex_code = r'''\begin{table}[htbp]
\centering
\caption{Network Model Ready-Event Statistics}
\begin{tabular}{c|c|c|c|c|c}
\hline
\multirow{2}{*}{Network} & \multirow{2}{*}{Size} & Hop & \multicolumn{3}{c}{Mean Ready Events} \\
\cline{4-6}
& & Radius & Min & Median & Max \\
\hline
'''
    
    # Process Ring (1D) network
    ring_df = stats_df[stats_df['network'] == 'Ring (1D)']
    if not ring_df.empty:
        row = ring_df.iloc[0]
        latex_code += f"Ring (1D) & {int(row['size'])} & N/A & {row['min']:.2f} & {row['median']:.2f} & {row['max']:.2f} \\\\\n"
        latex_code += r'\hline' + '\n'
    
    # Process Mesh (2D) network
    mesh_2d_df = stats_df[stats_df['network'] == 'Mesh (2D)']
    if not mesh_2d_df.empty:
        latex_code += r'\multirow{6}{*}{Mesh (2D)}' + '\n'
        
        # Size 64
        mesh_2d_64 = mesh_2d_df[mesh_2d_df['size'] == 64]
        if not mesh_2d_64.empty:
            latex_code += r'  & \multirow{3}{*}{64}' + '\n'
            
            sorted_rows = mesh_2d_64.sort_values('hop_radius')
            for i, (_, row) in enumerate(sorted_rows.iterrows()):
                # Convert hop_radius to integer if possible
                hop_radius = row['hop_radius']
                if hop_radius.replace('.', '', 1).isdigit():  # Check if it's a number
                    hop_radius = int(float(hop_radius))
                
                if i == 0:
                    latex_code += f"      & {hop_radius} & {row['min']:.2f} & {row['median']:.2f} & {row['max']:.2f} \\\\\n"
                else:
                    latex_code += f"  &                   & {hop_radius} & {row['min']:.2f} & {row['median']:.2f}  & {row['max']:.2f}  \\\\\n"
            
            latex_code += r'\cline{2-6}' + '\n'
        
        # Size 729
        mesh_2d_729 = mesh_2d_df[mesh_2d_df['size'] == 729]
        if not mesh_2d_729.empty:
            hop_radii = sorted(mesh_2d_729['hop_radius'].unique())
            
            for i, hr in enumerate(hop_radii):
                row = mesh_2d_729[mesh_2d_729['hop_radius'] == hr].iloc[0]
                
                # Convert hop_radius to integer if possible
                hop_radius = hr
                if hop_radius.replace('.', '', 1).isdigit():  # Check if it's a number
                    hop_radius = int(float(hop_radius))
                
                if i == 0:
                    latex_code += r'  & \cellcolor{lightgray!50} & \cellcolor{lightgray!50}' + f"{hop_radius} & \\cellcolor{{lightgray!50}}{row['min']:.2f} & \\cellcolor{{lightgray!50}}{row['median']:.2f} & \\cellcolor{{lightgray!50}}{row['max']:.2f} \\\\\n"
                elif i == 1:
                    latex_code += r'  & \cellcolor{lightgray!50}729 & \cellcolor{lightgray!50}' + f"{hop_radius} & \\cellcolor{{lightgray!50}}{row['min']:.2f} & \\cellcolor{{lightgray!50}}{row['median']:.2f} & \\cellcolor{{lightgray!50}}{row['max']:.2f} \\\\\n"
                else:
                    latex_code += r'  & \cellcolor{lightgray!50} & \cellcolor{lightgray!50}' + f"{hop_radius} & \\cellcolor{{lightgray!50}}{row['min']:.2f} & \\cellcolor{{lightgray!50}}{row['median']:.2f} & \\cellcolor{{lightgray!50}}{row['max']:.2f} \\\\\n"
            
            latex_code += r'\hline' + '\n'
    
    # Process Mesh (3D) network
    mesh_3d_df = stats_df[stats_df['network'] == 'Mesh (3D)']
    if not mesh_3d_df.empty:
        latex_code += r'\multirow{6}{*}{Mesh (3D)}' + '\n'
        
        # Size 64
        mesh_3d_64 = mesh_3d_df[mesh_3d_df['size'] == 64]
        if not mesh_3d_64.empty:
            latex_code += r'  & \multirow{3}{*}{64}' + '\n'
            
            sorted_rows = mesh_3d_64.sort_values('hop_radius')
            for i, (_, row) in enumerate(sorted_rows.iterrows()):
                # Convert hop_radius to integer if possible
                hop_radius = row['hop_radius']
                if hop_radius.replace('.', '', 1).isdigit():  # Check if it's a number
                    hop_radius = int(float(hop_radius))
                
                if i == 0:
                    latex_code += f"      & {hop_radius} & {row['min']:.2f} & {row['median']:.2f} & {row['max']:.2f} \\\\\n"
                else:
                    latex_code += f"  &                   & {hop_radius} & {row['min']:.2f} & {row['median']:.2f} & {row['max']:.2f} \\\\\n"
            
            latex_code += r'\cline{2-6}' + '\n'
        
        # Size 729
        mesh_3d_729 = mesh_3d_df[mesh_3d_df['size'] == 729]
        if not mesh_3d_729.empty:
            hop_radii = sorted(mesh_3d_729['hop_radius'].unique())
            
            for i, hr in enumerate(hop_radii):
                row = mesh_3d_729[mesh_3d_729['hop_radius'] == hr].iloc[0]
                
                # Convert hop_radius to integer if possible
                hop_radius = hr
                if hop_radius.replace('.', '', 1).isdigit():  # Check if it's a number
                    hop_radius = int(float(hop_radius))
                
                if i == 0:
                    latex_code += r'  & \cellcolor{lightgray!50} & \cellcolor{lightgray!50}' + f"{hop_radius} & \\cellcolor{{lightgray!50}}{row['min']:.2f} & \\cellcolor{{lightgray!50}}{row['median']:.2f} & \\cellcolor{{lightgray!50}}{row['max']:.2f} \\\\\n"
                elif i == 1:
                    latex_code += r'  & \cellcolor{lightgray!50}729 & \cellcolor{lightgray!50}' + f"{hop_radius} & \\cellcolor{{lightgray!50}}{row['min']:.2f} & \\cellcolor{{lightgray!50}}{row['median']:.2f} & \\cellcolor{{lightgray!50}}{row['max']:.2f} \\\\\n"
                else:
                    latex_code += r'  & \cellcolor{lightgray!50} & \cellcolor{lightgray!50}' + f"{hop_radius} & \\cellcolor{{lightgray!50}}{row['min']:.2f} & \\cellcolor{{lightgray!50}}{row['median']:.2f} & \\cellcolor{{lightgray!50}}{row['max']:.2f} \\\\\n"
            
            latex_code += r'\hline' + '\n'
    
    # Process Torus (3D) network
    torus_df = stats_df[stats_df['network'] == 'Torus (3D)']
    if not torus_df.empty:
        latex_code += r'\multirow{6}{*}{Torus (3D)}' + '\n'
        
        # Size 64
        torus_64 = torus_df[torus_df['size'] == 64]
        if not torus_64.empty:
            latex_code += r'  & \multirow{3}{*}{64}' + '\n'
            
            sorted_rows = torus_64.sort_values('hop_radius')
            for i, (_, row) in enumerate(sorted_rows.iterrows()):
                # Convert hop_radius to integer if possible
                hop_radius = row['hop_radius']
                if hop_radius.replace('.', '', 1).isdigit():  # Check if it's a number
                    hop_radius = int(float(hop_radius))
                
                if i == 0:
                    latex_code += f"      & {hop_radius} & {row['min']:.2f} & {row['median']:.2f} & {row['max']:.2f} \\\\\n"
                else:
                    latex_code += f"  &                   & {hop_radius} & {row['min']:.2f} & {row['median']:.2f} & {row['max']:.2f} \\\\\n"
            
            latex_code += r'\cline{2-6}' + '\n'
        
        # Size 729
        torus_729 = torus_df[torus_df['size'] == 729]
        if not torus_729.empty:
            hop_radii = sorted(torus_729['hop_radius'].unique())
            
            for i, hr in enumerate(hop_radii):
                row = torus_729[torus_729['hop_radius'] == hr].iloc[0]
                
                # Convert hop_radius to integer if possible
                hop_radius = hr
                if hop_radius.replace('.', '', 1).isdigit():  # Check if it's a number
                    hop_radius = int(float(hop_radius))
                
                if i == 0:
                    latex_code += r'  & \cellcolor{lightgray!50} & \cellcolor{lightgray!50}' + f"{hop_radius} & \\cellcolor{{lightgray!50}}{row['min']:.2f} & \\cellcolor{{lightgray!50}}{row['median']:.2f} & \\cellcolor{{lightgray!50}}{row['max']:.2f} \\\\\n"
                elif i == 1:
                    latex_code += r'  & \cellcolor{lightgray!50}729 & \cellcolor{lightgray!50}' + f"{hop_radius} & \\cellcolor{{lightgray!50}}{row['min']:.2f} & \\cellcolor{{lightgray!50}}{row['median']:.2f} & \\cellcolor{{lightgray!50}}{row['max']:.2f} \\\\\n"
                else:
                    latex_code += r'  & \cellcolor{lightgray!50} & \cellcolor{lightgray!50}' + f"{hop_radius} & \\cellcolor{{lightgray!50}}{row['min']:.2f} & \\cellcolor{{lightgray!50}}{row['median']:.2f} & \\cellcolor{{lightgray!50}}{row['max']:.2f} \\\\\n"
            
            latex_code += r'\hline' + '\n'
    
    # Close the table
    latex_code += r'''\end{tabular}
\label{tab:network_stats}
\end{table}'''
    
    return latex_code
    
    # Close the table
    latex_code += r'''\end{tabular}
\label{tab:network_stats}
\end{table}'''
    
    return latex_code

def main():
    """
    Main function to process the CSV file and generate the LaTeX table.
    """
    input_file = 'PADS_network_simulation_results.csv'
    output_file = 'PADS_table_5.tex'
    
    # Read and process the CSV data
    df = read_csv_data(input_file)
    
    # Calculate statistics based on the described approach
    stats_df = calculate_statistics(df)
    
    # Generate the LaTeX table
    latex_table = generate_latex_table(stats_df)
    
    # Add LaTeX package requirements as a comment at the top of the file
    package_info = "% " + get_required_latex_packages().replace("\n", "\n% ").strip()
    full_output = package_info + "\n\n" + latex_table
    
    # Save the LaTeX table to a file
    with open(output_file, 'w') as f:
        f.write(full_output)
    
    print(f"LaTeX table has been generated and saved to {output_file}")
    print("\nGenerated LaTeX Table:")
    print(latex_table)
    
    print("\nNote: To use this table in your LaTeX document, include the following packages:")
    print(r"\usepackage{multirow}    % For cells spanning multiple rows")
    print(r"\usepackage{colortbl}    % For cell background colors")
    print(r"\usepackage{xcolor}      % For color definitions (lightgray)")
    print(r"\usepackage{array}       % For advanced table formatting")

if __name__ == "__main__":
    main()
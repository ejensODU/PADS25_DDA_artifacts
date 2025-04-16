import pandas as pd
import numpy as np

# Required LaTeX packages for the table:
# \\usepackage{multirow}    % For cells spanning multiple rows
# \\usepackage{array}       % For advanced table formatting

def get_required_latex_packages():
    """
    Returns a string of LaTeX package requirements for the table.
    
    The table requires the following LaTeX packages:
    - multirow: For cells spanning multiple rows
    - array: For advanced table formatting
    
    These packages should be included in the preamble of your LaTeX document:
    \\usepackage{multirow}
    \\usepackage{array}
    """
    packages = r"""
Required LaTeX packages for this table:
\usepackage{multirow}    % For cells spanning multiple rows
\usepackage{array}       % For advanced table formatting
"""
    return packages

def read_csv_data(file_path):
    """
    Read the CSV file and return a processed DataFrame.
    """
    # Read the CSV file
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
    
    # Convert hop_radius to string type first (avoid warnings)
    df['hop_radius'] = df['hop_radius'].astype(str)
    
    # Replace 'nan' with 'N/A' for all hop_radius values
    df['hop_radius'] = df['hop_radius'].replace('nan', 'N/A')
    
    return df

def calculate_statistics(df):
    """
    Calculate statistics for the mean_diffs values.
    Group by network, node_count, and hop_radius,
    then find min, median, max across different params_config values.
    """
    # Group data by network, size, and hop_radius
    stats = []
    
    # Process network by network to ensure we don't miss any
    networks_order = {'Ring (1D)': 1, 'Mesh (2D)': 2, 'Mesh (3D)': 3, 'Torus (3D)': 4}
    
    for network in sorted(df['network'].unique(), key=lambda x: networks_order.get(x, 999)):
        network_df = df[df['network'] == network]
        
        for size in sorted(network_df['node_count'].unique()):
            size_df = network_df[network_df['node_count'] == size]
            
            for hop_radius in sorted(size_df['hop_radius'].unique()):
                hr_df = size_df[size_df['hop_radius'] == hop_radius]
                
                if not hr_df.empty:
                    mean_diffs = hr_df['mean_diffs'].values
                    
                    # Calculate min, median, max
                    min_val = np.min(mean_diffs)
                    median_val = np.median(mean_diffs)
                    max_val = np.max(mean_diffs)
                    
                    # Print debug info
                    print(f"Network: {network}, Size: {size}, Hop Radius: {hop_radius}")
                    print(f"  Mean Diffs: {[f'{x:.2f}' for x in sorted(mean_diffs)]}")
                    print(f"  Min: {min_val:.2f}, Median: {median_val:.2f}, Max: {max_val:.2f}")
                    
                    stats.append({
                        'network': network,
                        'size': size,
                        'hop_radius': hop_radius,
                        'min': min_val,
                        'median': median_val,
                        'max': max_val
                    })
    
    return pd.DataFrame(stats)

def generate_latex_table(stats_df):
    """
    Generate the LaTeX table based on the statistics DataFrame.
    """
    latex_code = r'''\begin{table}[htbp]
\centering
\caption{Event-Execution Order Difference Statistics}
\begin{tabular}{c|c|c|c|c|c}
\hline
\multirow{2}{*}{Network} & \multirow{2}{*}{Size} & Hop & \multicolumn{3}{c}{Mean Order Difference} \\
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
        hop_radii = sorted(mesh_2d_df['hop_radius'].unique())
        
        for i, hr in enumerate(hop_radii):
            row = mesh_2d_df[mesh_2d_df['hop_radius'] == hr].iloc[0]
            
            # Convert hop_radius to integer if possible
            hop_radius = hr
            if hop_radius.replace('.', '', 1).isdigit():  # Check if it's a number
                hop_radius = int(float(hop_radius))
            
            if i == 0:
                latex_code += r'\multirow{3}{*}{Mesh (2D)} & \multirow{3}{*}{64}' + f" & {hop_radius} & {row['min']:.2f} & {row['median']:.2f} & {row['max']:.2f} \\\\\n"
            else:
                latex_code += f"& & {hop_radius} & {row['min']:.2f} & {row['median']:.2f} & {row['max']:.2f} \\\\\n"
        
        latex_code += r'\hline' + '\n'
    
    # Process Mesh (3D) network
    mesh_3d_df = stats_df[stats_df['network'] == 'Mesh (3D)']
    if not mesh_3d_df.empty:
        hop_radii = sorted(mesh_3d_df['hop_radius'].unique())
        
        for i, hr in enumerate(hop_radii):
            row = mesh_3d_df[mesh_3d_df['hop_radius'] == hr].iloc[0]
            
            # Convert hop_radius to integer if possible
            hop_radius = hr
            if hop_radius.replace('.', '', 1).isdigit():  # Check if it's a number
                hop_radius = int(float(hop_radius))
            
            if i == 0:
                latex_code += r'\multirow{3}{*}{Mesh (3D)} & \multirow{3}{*}{64}' + f" & {hop_radius} & {row['min']:.2f} & {row['median']:.2f} & {row['max']:.2f} \\\\\n"
            else:
                latex_code += f"& & {hop_radius} & {row['min']:.2f} & {row['median']:.2f} & {row['max']:.2f} \\\\\n"
        
        latex_code += r'\hline' + '\n'
    
    # Process Torus (3D) network
    torus_df = stats_df[stats_df['network'] == 'Torus (3D)']
    if not torus_df.empty:
        hop_radii = sorted(torus_df['hop_radius'].unique())
        
        for i, hr in enumerate(hop_radii):
            row = torus_df[torus_df['hop_radius'] == hr].iloc[0]
            
            # Convert hop_radius to integer if possible
            hop_radius = hr
            if hop_radius.replace('.', '', 1).isdigit():  # Check if it's a number
                hop_radius = int(float(hop_radius))
            
            if i == 0:
                latex_code += r'\multirow{3}{*}{Torus (3D)} & \multirow{3}{*}{64}' + f" & {hop_radius} & {row['min']:.2f} & {row['median']:.2f} & {row['max']:.2f} \\\\\n"
            else:
                latex_code += f"& & {hop_radius} & {row['min']:.2f} & {row['median']:.2f} & {row['max']:.2f} \\\\\n"
        
        latex_code += r'\hline' + '\n'
    
    # Close the table
    latex_code += r'''\end{tabular}
\label{tab:exec_order_stats}
\end{table}'''
    
    return latex_code

def main():
    """
    Main function to process the CSV file and generate the LaTeX table.
    """
    input_file = 'PADS_exec_order_means_stds_64.csv'
    output_file = 'PADS_table_6.tex'
    
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
    print(r"\usepackage{array}       % For advanced table formatting")

if __name__ == "__main__":
    main()
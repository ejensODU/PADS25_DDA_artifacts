import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Set up LaTeX rendering
plt.rcParams.update({
    "text.usetex": True,
    "font.family": "sans-serif",
    "font.serif": ["Computer Modern Roman"],
    "text.latex.preamble": r"\usepackage{amsmath}"
})

# Model configurations
MODEL_CONFIG = {
    '1D_ring_network': '64',
    'VN2D_grid_network': '8_8',
    'VN3D_grid_network': '4_4_4',
    '3D_torus_network': '4_4_4'
}

MODEL_NAME_MAPPING = {
    '1D_ring_network': 'Rng64',
    'VN2D_grid_network': 'Msh8-2D',
    'VN3D_grid_network': 'Msh4-3D',
    '3D_torus_network': 'Trs4-3D'
}

MODEL_COLORS = {
    'Rng64': "#D2691E",      # Darker, more vivid orange-brown (more saturated chocolate)
    'Msh8-2D': "#4682B4",    # Richer blue (more saturated steelblue)
    'Msh4-3D': "#BC8F8F",    # Deeper pink-brown (more saturated rosybrown)
    'Trs4-3D': "#696969"     # Darker gray (more saturated gray)
}

def load_execution_data(base_path, models, seeds, params, exec_types, hop_radii=None):
    data = []
    for model in models:
        size = MODEL_CONFIG[model]
        for seed in seeds:
            for param in params:
                for exec_type in exec_types:
                    if model == '1D_ring_network':
                        file_path = os.path.join(
                            base_path,
                            f"order_{model}_size_{size}_seed_{seed}_params_{param}_exec_{exec_type}_threads_1_servers_1.csv"
                        )
                        if os.path.exists(file_path):
                            df = pd.read_csv(file_path)
                            df.columns = df.columns.str.strip()
                            data.append({
                                'model': model,
                                'display_name': MODEL_NAME_MAPPING[model],
                                'size': size,
                                'seed': seed,
                                'params_config': param,
                                'exec_type': exec_type,
                                'hop_radius': 'N/A',
                                'mean_diffs': df['mean_diffs'].mean(),
                                'std_diffs': df['std_diffs'].mean()
                            })
                    else:
                        for hop_radius in hop_radii:
                            file_path = os.path.join(
                                base_path,
                                f"order_{model}_size_{size}_hop_{hop_radius}_seed_{seed}_params_{param}_exec_{exec_type}_threads_1_servers_1.csv"
                            )
                            if os.path.exists(file_path):
                                df = pd.read_csv(file_path)
                                df.columns = df.columns.str.strip()
                                data.append({
                                    'model': model,
                                    'display_name': MODEL_NAME_MAPPING[model],
                                    'size': size,
                                    'seed': seed,
                                    'params_config': param,
                                    'exec_type': exec_type,
                                    'hop_radius': hop_radius,
                                    'mean_diffs': df['mean_diffs'].mean(),
                                    'std_diffs': df['std_diffs'].mean()
                                })

    df = pd.DataFrame(data)
    if not df.empty:
        # Calculate means across seeds for each configuration
        mean_df = df.groupby(['model', 'display_name', 'size', 'params_config', 'exec_type', 'hop_radius'])[['mean_diffs', 'std_diffs']].mean().reset_index()
        mean_df.to_csv('PADS_exec_order_means_stds_64.csv', index=False)

    return df


def plot_execution_differences(df):
    """
    Plot execution order differences with consistent styling from the reference implementation.
    """
    plt.figure(figsize=(12, 8))

    # Define style elements
    line_styles = ['-', '--', ':']
    markers = ['o', 's', '^', 'D']

    lines = []
    labels = []

    # Plot data for each model and execution type
    for model in sorted(df['model'].unique()):
        base_name = MODEL_NAME_MAPPING[model]
        model_data = df[df['model'] == model]
        color = MODEL_COLORS[base_name]

        if model == '1D_ring_network':
            for exec_type in sorted(model_data['exec_type'].unique()):
                exec_data = model_data[model_data['exec_type'] == exec_type]
                mean_data = exec_data.groupby('params_config')['mean_diffs'].mean()
                std_data = exec_data.groupby('params_config')['std_diffs'].mean()

                line = plt.errorbar(mean_data.index, mean_data.values,
                                  yerr=std_data,
                                  label=f'{base_name}',
                                  color=color,
                                  marker='o',
                                  markersize=8,
                                  linewidth=2,
                                  capsize=5)
                lines.append(line[0])
                labels.append(f'{base_name}-$C$')
        else:
            for i, hop_radius in enumerate([1, 2, 4]):
                hop_data = model_data[model_data['hop_radius'] == hop_radius]
                if not hop_data.empty:
                    for exec_type in sorted(hop_data['exec_type'].unique()):
                        exec_data = hop_data[hop_data['exec_type'] == exec_type]
                        mean_data = exec_data.groupby('params_config')['mean_diffs'].mean()
                        std_data = exec_data.groupby('params_config')['std_diffs'].mean()

                        line = plt.errorbar(mean_data.index, mean_data.values,
                                          yerr=std_data,
                                          label=f'{base_name}-{hop_radius}',
                                          color=color,
                                          linestyle=line_styles[i % len(line_styles)],
                                          marker=markers[i % len(markers)],
                                          markersize=8,
                                          linewidth=2,
                                          capsize=5)
                        lines.append(line[0])
                        labels.append(f'{base_name}-{hop_radius}H-$C$')

    # Customize plot
    plt.xlabel(r'Delay Distribution Configuration ($C$)', fontsize=28)
    plt.ylabel(r'Mean Event-Execution Order Difference', fontsize=28)
    plt.xticks(range(10), [r'$\mathrm{' + str(i) + '}$' for i in range(1, 11)])
    plt.tick_params(axis='both', labelsize=20)
    plt.grid(True, linestyle='--', alpha=1)

    # Sort and group labels
    labels_sorted = sorted(labels)
    rng_labels = [l for l in labels_sorted if l.startswith('Rng64')]
    msh2d_labels = [l for l in labels_sorted if l.startswith('Msh8-2D')]
    msh3d_labels = [l for l in labels_sorted if l.startswith('Msh4-3D')]
    trs3d_labels = [l for l in labels_sorted if l.startswith('Trs4-3D')]

    # Create corresponding line objects lists
    rng_lines = [lines[labels.index(l)] for l in rng_labels]
    msh2d_lines = [lines[labels.index(l)] for l in msh2d_labels]
    msh3d_lines = [lines[labels.index(l)] for l in msh3d_labels]
    trs3d_lines = [lines[labels.index(l)] for l in trs3d_labels]

    # Create legend entries with empty placeholders for alignment
    all_lines = rng_lines + [plt.plot([], [], alpha=0)[0], plt.plot([], [], alpha=0)[0]] + msh2d_lines + msh3d_lines + trs3d_lines
    all_labels = [r'\texttt{' + rng_labels[0] + '}'] + ['', ''] + \
            [r'\texttt{' + label + '}' for label in msh2d_labels + msh3d_labels + trs3d_labels]

    plt.legend(all_lines, all_labels,
            bbox_to_anchor=(0.5, 1.25),
            loc='upper center',
            ncol=4,
            fontsize=20,
            frameon=True,
            columnspacing=1.0)

    plt.tight_layout()

    # Save plots
    plt.savefig('PADS_exec_order_diffs_64.pdf', bbox_inches='tight')
    plt.close()

# Example usage
if __name__ == "__main__":
    base_path = "exec_orders"
    models = list(MODEL_CONFIG.keys())
    seeds = range(10)
    params = range(10)
    exec_types = [0, -10]
    hop_radii = [1, 2, 4]

    df = load_execution_data(base_path, models, seeds, params, exec_types, hop_radii)
    if df is not None:
        plot_execution_differences(df)
    else:
        print("No data found.")

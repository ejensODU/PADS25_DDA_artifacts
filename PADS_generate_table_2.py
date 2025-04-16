import re
import sys

def parse_events(input_text):
    events = []
    current_event = None
    
    for line in input_text.split('\n'):
        line = line.strip()
        if not line:
            continue
        if line.startswith('i:'):
            try:
                parts = [p.strip() for p in line.split(',')]
                i = int(parts[0].split(':')[1].strip())
                
                time_part = parts[1].split(':')
                time = float(time_part[1].strip())
                
                event_desc = parts[2].split(':')[1].strip()
                event_type = event_desc.split()[0][0]  # first letter of event type
                unit = int(event_desc.split()[1])
                
                current_event = {
                    'i': i,
                    't': time,
                    'event': event_desc,
                    'type': event_type,
                    'unit': unit,
                    'E': [],  # All events (sorted)
                    'D': [],  # Direct dependencies
                    'I': [],  # Indirect dependencies
                    'R': []   # Independent events
                }
                events.append(current_event)
            except (IndexError, ValueError) as e:
                continue
        elif current_event and 'Sorted Events:' in line:
            try:
                deps = re.findall(r"\('([\d.]+)', '([^']+)', (\d+)\)", line)
                current_event['E'] = [(event_type[0], float(time), int(unit)) for time, event_type, unit in deps]
            except:
                continue
        elif current_event and 'EC - Direct Dependencies (D):' in line:
            try:
                deps = re.findall(r"\('([\d.]+)', '([^']+)', (\d+)\)", line)
                current_event['D'] = [(event_type[0], float(time), int(unit)) for time, event_type, unit in deps]
            except:
                continue
        elif current_event and 'EC - Indirect Dependencies (I):' in line:
            try:
                deps = re.findall(r"\('([\d.]+)', '([^']+)', (\d+)\)", line)
                current_event['I'] = [(event_type[0], float(time), int(unit)) for time, event_type, unit in deps]
            except:
                continue
        elif current_event and 'Independent Events (R):' in line:
            try:
                deps = re.findall(r"\('([\d.]+)', '([^']+)', (\d+)\)", line)
                current_event['R'] = [(event_type[0], float(time), int(unit)) for time, event_type, unit in deps]
            except:
                continue
    
    return events

def is_in_set(event, event_set):
    """Check if an event (type, time, unit) is in a given set"""
    for e in event_set:
        if e[0] == event[0] and abs(e[1] - event[1]) < 0.001 and e[2] == event[2]:
            return True
    return False

def truncate_float(value, decimals=2):
    """Truncate (not round) a float to the specified number of decimal places"""
    factor = 10 ** decimals
    return int(value * factor) / factor

def format_event_set(events, event_sets):
    """
    Format event set with proper styling
    events: List of (type, time, unit) tuples to format
    event_sets: Dictionary containing 'D' and 'I' sets for checking membership
    """
    if not events:
        return "\\{\\}"
    
    # Sort by time
    sorted_events = sorted(events, key=lambda x: x[1])
    
    formatted_events = []
    for typ, time, unit in sorted_events:
        # Truncate time to 2 decimal places (no rounding)
        trunc_time = truncate_float(time, 2)
        # Format with exactly 2 decimal places
        time_str = f"{trunc_time:.2f}"
        
        # Format each event
        event_str = f"(\\mathtt{{{unit}\\text{{-}}{typ}}}, {time_str})"
        
        # Check if it should be bold (in set D)
        if is_in_set((typ, time, unit), event_sets['D']):
            event_str = f"\\boldsymbol{{{event_str}}}"
        
        # Check if it should be red and bold (in set I)
        if is_in_set((typ, time, unit), event_sets['I']):
            event_str = f"\\boldsymbol{{\\textcolor{{red!80!black}}{{{event_str}}}}}"
            
        formatted_events.append(event_str)
    
    return "\\{" + ", ".join(formatted_events) + "\\}"

def main():
    input_text = sys.stdin.read()
    events = parse_events(input_text)
    
    output_lines = []
    # Add comment indicating required packages
    output_lines.append("% Required LaTeX packages for this table:")
    output_lines.append("% \\usepackage{array}     % For column formatting with >{\\raggedright\\arraybackslash}")
    output_lines.append("% \\usepackage{xcolor}    % For colored text with \\textcolor")
    output_lines.append("% \\usepackage{amsmath}   % For \\text command inside math mode")
    output_lines.append("% \\usepackage{amssymb}   % For \\mathcal for the distributions")
    output_lines.append("")
    output_lines.append("\\begin{table*}")
    output_lines.append("    \\centering")
    output_lines.append("    \\begin{tabular}{>{\\raggedright\\arraybackslash}m{0.4cm} >{\\raggedright\\arraybackslash}m{0.8cm} >{\\raggedright\\arraybackslash}m{9cm} >{\\raggedright\\arraybackslash}m{5.6cm}}")
    output_lines.append("    \\hline")
    output_lines.append("    $i$ & $t_\\mathrm{sim}$ & $\\mathbf{E}$ & $\\mathbf{R}$ \\\\")
    output_lines.append("    \\hline")
    
    for evt in events:
        if evt['i'] < 30 or evt['i'] > 37:  # the ones included in the table
            continue
        
        event_sets = {'D': evt['D'], 'I': evt['I']}
        E_str = format_event_set(evt['E'], event_sets)
        R_str = format_event_set(evt['R'], event_sets)
        
        # Truncate simulation time to 2 decimal places (no rounding)
        trunc_time = truncate_float(evt['t'], 2)
        # Format with exactly 2 decimal places
        time_str = f"{trunc_time:.2f}"
        output_lines.append(f"    {evt['i']} & {time_str} & ${E_str}$ & ${R_str}$ \\\\")
    
    output_lines.append("    \\hline")
    output_lines.append("    \\end{tabular}")
    output_lines.append("    \\caption{Simulation of 4-queue tandem network \\texttt{Tdm4}, with $t_a{\\sim} \\mathcal{U}(1,2)$, $t_p{\\sim} \\mathcal{U}(2,10)$, $t_t{\\sim} \\mathcal{U}(0.1,0.2)$. Column $i$ indicates IO event-execution iteration. Events in $\\mathbf{E}$ with DDD-ECs are bold; events with IDD-ECs are bold and red.}")
    output_lines.append("    \\label{tab:4q_network_sim_output}")
    output_lines.append("\\end{table*}")
    
    # Print to stdout for backward compatibility
    for line in output_lines:
        print(line)
    
    # Write to file
    with open("PADS_table_2.tex", "w") as f:
        for line in output_lines:
            f.write(line + "\n")

if __name__ == "__main__":
    main()
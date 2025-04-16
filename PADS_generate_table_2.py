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
                    'E': [],
                    'R': []
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
        elif current_event and 'Independent Events (R):' in line:
            try:
                deps = re.findall(r"\('([\d.]+)', '([^']+)', (\d+)\)", line)
                current_event['R'] = [(event_type[0], float(time), int(unit)) for time, event_type, unit in deps]
            except:
                continue
    
    return events

def should_be_bold(typ, unit):
    # 1-D events should be bold
    if typ == 'D' and unit == 1:
        return True
    return False

def should_be_red(typ, unit, i):
    # 2-D and 4-D events should be bold and red after a certain point
    if typ == 'D' and (unit == 2 or unit == 4) and i >= 33:
        return True
    return False

def format_event_set(events, event_i):
    if not events:
        return "\\{\\}"
    
    # Sort by time
    sorted_events = sorted(events, key=lambda x: x[1])
    
    formatted_events = []
    for typ, time, unit in sorted_events:
        # Format each event
        event_str = f"(\\mathtt{{{unit}\\text{{-}}{typ}}}, {time:.2f})"
        
        # Check if it should be bold
        if should_be_bold(typ, unit):
            event_str = f"\\boldsymbol{{{event_str}}}"
        
        # Check if it should be red and bold
        if should_be_red(typ, unit, event_i):
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
        
        E_str = format_event_set(evt['E'], evt['i'])
        R_str = format_event_set(evt['R'], evt['i'])
        
        output_lines.append(f"    {evt['i']} & {evt['t']:.2f} & ${E_str}$ & ${R_str}$ \\\\")
    
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
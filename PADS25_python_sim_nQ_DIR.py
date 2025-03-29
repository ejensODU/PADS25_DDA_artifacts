import heapq
import random

# Seed the random number generator for reproducibility
random.seed(0)

# Define the initial state
num_stations = 4
time = 0
event_queue = [(0, 'Arrive', 1)]  # (time, event_type, station_number)
P = [1, 1, 1, 1]  # Processing units for Station 1 and 2
Q = [0, 0, 0, 0]  # Queue lengths for Station 1 and 2
min_IA_delay = 1
max_IA_delay = 2
min_process_delay = 2
max_process_delay = 10
min_transit_delay = 0.1
max_transit_delay = 0.2

def compute_min_distance(event_j, event_k):

    min_distance = 999999

    if event_k[2] > event_j[2]:
        event_j_min_time_next_st = 0
        if event_j[1] == 'Arrive': event_j_min_time_next_st = min_process_delay + min_transit_delay
        elif event_j[1] == 'Process': event_j_min_time_next_st = min_process_delay + min_transit_delay
        elif event_j[1] == 'Depart': event_j_min_time_next_st = min_transit_delay

        intermediate_min_distance = min_process_delay + min_transit_delay * (event_k[2] - event_j[2] - 1)

        event_k_min_time_start_st = 0
        if event_k[1] == 'Arrive': event_k_min_time_start_st = 0
        elif event_k[1] == 'Process': event_k_min_time_start_st = 0
        elif event_k[1] == 'Depart': event_k_min_time_start_st = min_process_delay

        min_distance = event_j_min_time_next_st + intermediate_min_distance + event_k_min_time_start_st

    return min_distance

def classify_events(sorted_events):
    D = set()  # Direct dependencies
    I = set()  # Indirect dependencies
    R = set()  # Independent events

    for k in range(len(sorted_events)):
        event_k = sorted_events[k]
        added = False
        for j in range(k):
            event_j = sorted_events[j]
            if event_j[2] == event_k[2] and (event_j[1] != 'Process' or event_k[1] != 'Process'):
                D.add(event_k)
                added = True
                break
            elif compute_min_distance(event_j, event_k) <= event_k[0] - event_j[0]:
                I.add(event_k)
                added = True
                break
        if not added:
            R.add(event_k)

    return D, I, R

def process_event(event):
    global time
    _, event_type, station = event
    station_index = station - 1  # Adjust for 0-based indexing

    if event_type == 'Arrive':
        # Schedule next arrival for station 1 only
        if station == 1:
            heapq.heappush(event_queue, (time + random.uniform(min_IA_delay, max_IA_delay), 'Arrive', 1))
        # Add to the queue if processing unit not available
        if P[station_index] == 0:
            Q[station_index] += 1
        # Schedule a process event if processing unit available
        elif P[station_index] > 0:
            P[station_index] -= 1
            heapq.heappush(event_queue, (time, 'Process', station))

    elif event_type == 'Process':
        # Process the event (considering it immediately starts processing)
        heapq.heappush(event_queue, (time + random.uniform(min_process_delay, max_process_delay), 'Depart', station))

    elif event_type == 'Depart':
        # Schedule arrival at next station
        if station_index + 1 < num_stations:
            heapq.heappush(event_queue, (time + random.uniform(min_transit_delay, max_transit_delay), 'Arrive', station + 1))
        # Free up a processing unit and check if there's more to process
        if Q[station_index] == 0:
            P[station_index] += 1
        elif Q[station_index] > 0:
            heapq.heappush(event_queue, (time, 'Process', station))

count_Is = 0
# Main simulation loop
for i in range(40):
    if not event_queue:
        break

    time, event_type, station = heapq.heappop(event_queue)
    print(f"i: {i}, Time: {time:.3f}, Event: {event_type} {station}, Queue: {Q}, Processing Units: {P}")
    process_event((time, event_type, station))

    # Print the event queue in a sorted order and format timestamps to three decimal places
    sorted_events = sorted(event_queue)
    print("Sorted Events:", [(f"{t:.3f}", etype, st) for t, etype, st in sorted_events])

    # Classify events
    D, I, R = classify_events(sorted_events)
    print("EC - Direct Dependencies (D):", [(f"{t:.3f}", etype, st) for t, etype, st in D])
    print("EC - Indirect Dependencies (I):", [(f"{t:.3f}", etype, st) for t, etype, st in I])
    print("Independent Events (R):", [(f"{t:.3f}", etype, st) for t, etype, st in R])

    count_Is += len(I)
    print('\n')

print(count_Is)

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import math

# Read the CSV, skipping the first row which is a description
df = pd.read_csv('maxcut_results.csv', skiprows=1)

# Clean up column names
df.columns = df.columns.str.strip()

# Rename columns
df.rename(columns={
    'Simple Randomized or Randomized-1': 'Randomized',
    'Simple Greedy or Greedy-1': 'Greedy',
    'Semi-greedy-1': 'Semi-Greedy',
    'Average value': 'Local Search',
    'Best value': 'GRASP'
}, inplace=True)

# Loop through in chunks of 10
chunk_size = 10
num_rows = len(df)
num_chunks = math.ceil(num_rows / chunk_size)

for i in range(num_chunks):
    start = i * chunk_size
    end = min((i + 1) * chunk_size, num_rows)
    chunk = df.iloc[start:end]

    # Extract data
    names = chunk['Name']
    randomized = chunk['Randomized']
    greedy = chunk['Greedy']
    semi_greedy = chunk['Semi-Greedy']
    local_search = chunk['Local Search']
    grasp = chunk['GRASP']

    # Bar chart setup
    x = np.arange(len(names))
    width = 0.15

    fig, ax = plt.subplots(figsize=(12, 6))
    ax.bar(x - 2*width, randomized, width, label='Randomized', color='skyblue')
    ax.bar(x - width, greedy, width, label='Greedy', color='red')
    ax.bar(x, semi_greedy, width, label='Semi-Greedy', color='lightgrey')
    ax.bar(x + width, grasp, width, label='GRASP', color='green')
    ax.bar(x + 2*width, local_search, width, label='Local Search', color='dodgerblue')

    # Labels and title
    ax.set_ylabel('Max Cut Value')
    ax.set_title(f'Max Cut (Graph {start+1}–{end})')
    ax.set_xticks(x)
    ax.set_xticklabels(names)
    ax.legend()
    plt.tight_layout()
    plt.grid(axis='y', linestyle='--', alpha=0.7)

    # Save the figure
    plt.savefig(f'maxcut_graphs_{start+1}_to_{end}.png')
    plt.close()  # Close the figure to free memory

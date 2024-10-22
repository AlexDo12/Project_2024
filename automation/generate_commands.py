# generate_commands.py

import math

input_sizes = [2**16, 2**18, 2**20, 2**22, 2**24, 2**26, 2**28]
input_types = ['sorted', 'reverse', '1perturbed', 'random']
# num_procs = [2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]
num_procs = [1024]
algorithm = 'sample'

cores_per_node = 48

with open('commands.txt', 'w') as file:
    for input_size in input_sizes:
        for input_type in input_types:
            for proc in num_procs:
                # Calculate nodes and tasks per node
                if proc == 2:
                    nodes_needed = 1
                    ntasks_per_node = 2
                elif proc == 4:
                    nodes_needed = 1
                    ntasks_per_node = 4
                elif proc == 8:
                    nodes_needed = 1
                    ntasks_per_node = 8
                elif proc == 16:
                    nodes_needed = 1
                    ntasks_per_node = 16
                elif proc == 32:
                    nodes_needed = 1
                    ntasks_per_node = 32
                elif proc == 64:
                    nodes_needed = 2
                    ntasks_per_node = 32
                elif proc == 128:
                    nodes_needed = 4
                    ntasks_per_node = 32
                elif proc == 256:
                    nodes_needed = 8
                    ntasks_per_node = 32
                elif proc == 512:
                    nodes_needed = 16
                    ntasks_per_node = 32
                elif proc == 1024:
                    nodes_needed = 32
                    ntasks_per_node = 32
                command = (
                    # The first two lines should make it so we don't have to manually modify the grace_job every time
                    # Side note, if you want to check, you could try:
                    # sbatch --output=youaresus.%j mpi.grace_job 65536 2 bitonic sorted
                    # Notice the output name changes
                    f"sbatch --nodes={nodes_needed} "
                    f"--ntasks-per-node={ntasks_per_node} "
                    f"mpi.grace_job {input_size} {proc} {algorithm} {input_type}"
                )
                file.write(command + '\n')


"""
Old base
# generate_commands.py

# Define your parameters
input_sizes = [2**16, 2**18, 2**20, 2**22, 2**24, 2**26, 2**28]
input_types = ['sorted', 'reverse', '1perturbed', 'random']
num_procs = [2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]

# Open a file to write the commands
with open('commands.txt', 'w') as file:
    for input_size in input_sizes:
        for input_type in input_types:
            for proc in num_procs:
                command = f"sbatch mpi.grace_job {input_size} {proc} bitonic {input_type}"
                file.write(command + '\n')
"""

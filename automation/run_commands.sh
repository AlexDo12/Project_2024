#!/bin/bash

# run_commands.sh

# Read each line from commands.txt and execute it
while IFS= read -r command; do
    echo "Executing: $command"
    $command
done < commands.txt

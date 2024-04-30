#!/bin/bash

# Check the number of arguments
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <input_file> <output_file>"
    exit 1
fi

input_file="$1"
output_file="$2"

# Copy the first two lines (start time and number of particles) of the input file to the output file
head -n2 "$input_file" > "$output_file"

# Add the csv header
echo "type,x,y,z,vx,vy,vz" >> "$output_file"

# Convert the rest of the input file to csv format (start from the third line)
tail -n +3 "$input_file" | while IFS=" " read -r type x y z vx vy vz; do
    # Skip the line if the type is empty or not a number
    if [[ -z "$type" || ! "$type" =~ ^[0-9]+$ ]]; then
        continue
    fi

    echo "$type,$x,$y,$z,$vx,$vy,$vz" >> "$output_file"
done

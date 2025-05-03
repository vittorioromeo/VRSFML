#!/bin/bash
# Create the target directory if it doesn't exist.
mkdir -p ./unachieved

# Loop over each PNG file in the achieved directory.
for file in ./achieved/*.png; do
    # Extract the filename from the full path.
    filename=$(basename "$file")

    # Convert the image to grayscale and save it in the unachieved folder.
    /ucrt64/bin/convert.exe "$file" -colorspace Gray "./unachieved/$filename"
done

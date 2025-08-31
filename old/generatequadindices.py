def generate_quad_indices(num_quads: int) -> list[int]:
    """
    Generates a list of vertex indices for a strip of quads.

    Each quad is composed of 4 vertices and represented by 2 triangles.
    For the n-th quad (0-indexed), the vertices used are 4n, 4n+1, 4n+2, 4n+3.
    The indices generated are for the two triangles: (4n, 4n+1, 4n+2) and (4n+1, 4n+2, 4n+3).

    Args:
        num_quads: The total number of quads to generate indices for.

    Returns:
        A list of integers representing the triangle indices.
    """
    if num_quads < 0:
        print("Warning: Number of quads cannot be negative. Returning empty list.")
        return []

    indices = []
    for i in range(num_quads):
        # Calculate the starting vertex index for the current quad
        base_index = i * 4

        # First triangle of the quad
        indices.append(base_index)
        indices.append(base_index + 1)
        indices.append(base_index + 2)

        # Second triangle of the quad
        indices.append(base_index + 1)
        indices.append(base_index + 2)
        indices.append(base_index + 3)

    return indices

# --- Main execution block ---
if __name__ == "__main__":
    # Specify the amount of quads you want to generate indices for.
    QUADS_TO_GENERATE = 65536

    print(f"Generating indices for {QUADS_TO_GENERATE} quads...")

    # Generate the list of indices
    quad_indices = generate_quad_indices(QUADS_TO_GENERATE)

    # --- Option 2: Print as a clean, comma-separated string ---
    # This is useful for copying and pasting into code (e.g., a C++ or JS array)
    indices_string = ",".join(map(str, quad_indices))
    print(indices_string)

    # --- Option 3: Write the indices to a text file ---
    output_filename = "quad_indices.txt"
    try:
        with open(output_filename, "w") as f:
            f.write(indices_string)
        print(f"\n3. Successfully wrote indices to '{output_filename}'")
    except IOError as e:
        print(f"\nError writing to file: {e}")

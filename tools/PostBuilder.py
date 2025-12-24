import os
import sys
import csv
import subprocess

def parse_partition_table(partition_csv):
    partitions = {}
    with open(partition_csv, "r") as f:
        reader = csv.reader(f)
        for row in reader:
            if len(row) < 4 or row[0].startswith("#"):
                continue
            name, _, offset, path = row[:4]
            partitions[name] = (int(offset, 16), path.strip())
    return partitions

def generate_combined_binary(output_bin, partitions):
    with open(output_bin, "wb") as combined:
        for name, (offset, path) in partitions.items():
            if not os.path.exists(path):
                print(f"Warning: {path} not found, skipping {name}")
                continue
            with open(path, "rb") as part_bin:
                data = part_bin.read()
            combined.seek(offset)
            combined.write(data)
    print(f"Combined binary created: {output_bin}")

def main():
    if len(sys.argv) < 4:
        print("Usage: python PostBuilder.py <partition.csv> <output.bin> <esptool.py path>")
        sys.exit(1)
    
    partition_csv, output_bin, esptool = sys.argv[1:4]
    partitions = parse_partition_table(partition_csv)
    generate_combined_binary(output_bin, partitions)
    
    flash_cmd = [
        sys.executable, esptool,
        "--chip", "esp32",
        "write_flash", "-z"
    ]
    
    for name, (offset, path) in partitions.items():
        if os.path.exists(path):
            flash_cmd.extend([hex(offset), path])
    
    print("Flashing command:", " ".join(flash_cmd))
    subprocess.run(flash_cmd)

if __name__ == "__main__":
    main()

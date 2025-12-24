import("env")
import os
import subprocess

def post_build(source, target, env):
    firmware_path = str(target[0])
    print(f"Firmware built: {firmware_path}")
    
    # Path al file partizioni
    partition_csv = os.path.join(env.subst("$PROJECT_DIR"), "partitions_custom.csv")
    
    # Verifica che esista
    if not os.path.exists(partition_csv):
        print(f"ERROR: Partition file not found: {partition_csv}")
        return
    
    print(f"Using partition table: {partition_csv}")
    
    # Genera il file binario delle partizioni
    partition_bin = os.path.join(env.subst("$BUILD_DIR"), "partitions.bin")
    
    gen_esp32part = env.subst("$PYTHONEXE") + " " + os.path.join(
        env.PioPlatform().get_package_dir("framework-arduinoespressif32"),
        "tools", "gen_esp32part.py"
    )
    
    cmd = f'{gen_esp32part} {partition_csv} {partition_bin}'
    print(f"Generating partition binary: {cmd}")
    os.system(cmd)
    
    print("=" * 60)
    print("Partition table generated successfully!")
    print("To flash manually with partitions:")
    print(f"esptool.py --chip esp32c3 --port /dev/ttyACM0 --baud 921600 \\")
    print(f"  write_flash -z \\")
    print(f"  0x0 {env.subst('$BUILD_DIR')}/bootloader.bin \\")
    print(f"  0x8000 {partition_bin} \\")
    print(f"  0x10000 {firmware_path}")
    print("=" * 60)

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", post_build)
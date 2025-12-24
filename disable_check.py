Import('env') # type: ignore

# Rimuovi il target che controlla la dimensione
env.Replace(
    SIZEPROGREGEXP=r"^$"  # Disabilita il check regex
)

# Oppure sostituisci il comando di check
def dummy_check(*args, **kwargs):
    print("Size check disabled - using custom partitions")
    return None

# Rimuovi tutti i check di dimensione
for target in ["checkprogsize", "checkprog"]:
    if target in env.get("__PIO_BUILD_TARGETS", []):
        env.get("__PIO_BUILD_TARGETS").remove(target)

print("=" * 60)
print("NOTICE: Program size check DISABLED")
print("Using custom partition table: partitions_custom.csv")
print("=" * 60)
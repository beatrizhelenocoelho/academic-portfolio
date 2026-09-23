import os
import zmq
import message_comms_pb2
import sys

# ============================================================
# Configuration file reader
# ============================================================
# Reads a simple key=value configuration file and extracts
# only the requested keys.
# ============================================================

def read_conf_keys(filename, keys):
    config = {}

    with open(filename, "r") as f:
        for line in f:
            line = line.strip()

            # Ignore empty lines and comments
            if not line or line.startswith("#"):
                continue

            if "=" not in line:
                continue

            key, value = line.split("=", 1)
            key = key.strip()
            value = value.replace(";", "").replace('"', "").strip()

            # Try to convert numeric values
            if key in keys:
                try:
                    config[key] = int(value)
                except ValueError:
                    config[key] = value

    # Ensure all required keys exist
    missing = [k for k in keys if k not in config]
    if missing:
        raise KeyError(f"Missing keys in config file: {missing}")

    return config


# ============================================================
# Read configuration
# ============================================================

cfg = read_conf_keys(
    "../config.conf",
    ["max_trash", "ip_server", "pub_port"]
)

ip = cfg["ip_server"]
port = cfg["pub_port"]

# Optional IP override via command line
if len(sys.argv) > 1:
    ip = sys.argv[1]

print(f"Connecting to {ip}:{port}...")


# ============================================================
# ZeroMQ subscriber setup
# ============================================================

context = zmq.Context()
socket = context.socket(zmq.SUB)

# Connect to server publisher
socket.connect(f"tcp://{ip}:{port}")

# Subscribe only to WORLD messages
socket.setsockopt(zmq.SUBSCRIBE, b"WORLD")

# Receive timeout (milliseconds)
socket.setsockopt(zmq.RCVTIMEO, 200)

print("Waiting for WorldState messages...")


# ============================================================
# Terminal formatting
# ============================================================

BRIGHT_GREEN = "\033[1;92m"
RESET = "\033[0m"


# ============================================================
# Receive and display loop
# ============================================================

missed = 0
MAX_MISSES = 5

try:
    while True:
        try:
            # Receive topic + protobuf payload
            topic, data = socket.recv_multipart()
            missed = 0   # reset timeout counter on success

        except zmq.Again:
            missed += 1
            print(f"[WARN] No data from server ({missed}/{MAX_MISSES})")

            # Exit if server is considered down
            if missed >= MAX_MISSES:
                print("[ERROR] Server timeout, exiting.")
                break

            continue

        # Decode protobuf message
        world_state = message_comms_pb2.WorldState()
        world_state.ParseFromString(data)

        # Clear terminal for updated view
        os.system("clear")

        # Server shutdown condition
        if world_state.seq == 0 or world_state.seq == -1:
            print("[INFO] Server closed world.")
            break

        # Display planets and recycled trash
        print(f"{BRIGHT_GREEN}{'-' * 30}{RESET}")
        print(f"{BRIGHT_GREEN}PLANETS   (Recycled Trash){RESET}")
        for planet in world_state.planets:
            print(f"{BRIGHT_GREEN}{planet.name} - {planet.trash:<3}{RESET}")

        # Display active ships and carried trash
        print(f"\n{BRIGHT_GREEN}TRASH-SHIPS   (Trash Cargo){RESET}")
        for ship in world_state.ships:
            if ship.use == 1:
                print(f"{BRIGHT_GREEN}{ship.name} - {ship.trash:<3}{RESET}")

        # Compute roaming trash statistics
        trash_roaming = sum(
            1 for trash in world_state.trash if trash.taken == 0
        )

        trash_roaming_percent = int(
            (trash_roaming / cfg["max_trash"]) * 100
        )

        # Display universe status
        print(f"\n{BRIGHT_GREEN}UNIVERSE{RESET}")
        print(f"{BRIGHT_GREEN}ROAMING TRASH: {trash_roaming:<3}{RESET}")
        print(f"{BRIGHT_GREEN}TRASH CAPACITY: {trash_roaming_percent}%{RESET}")
        print(f"{BRIGHT_GREEN}{'-' * 30}{RESET}")

except KeyboardInterrupt:
    print("\nExiting...")

finally:
    socket.close()
    context.term()

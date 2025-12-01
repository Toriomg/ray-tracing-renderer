import sys
import subprocess
import os
import argparse

# --- CONFIGURATION ---
EXTRACT_SCRIPT = "scripts/memory/extract_time.py"
# ---------------------

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--log-file", required=True)
    args = parser.parse_args()

    print(">>> Monitoring Validation Pipeline...")
    
    total_scenes = 0
    valid_count = 0
    invalid_count = 0
    
    # We use this to clear the line for the progress bar
    # \r goes to start of line, \033[K clears the line
    def print_status(msg):
        sys.stdout.write(f"\r\033[K>>> ⏳ {msg}")
        sys.stdout.flush()

    try:
        for line in sys.stdin:
            # --- CHANGE: We DO NOT print the raw line anymore ---
            # sys.stdout.write(line) 

            # Update progress bar when a new scene starts
            if "Checking Scene" in line:
                # Extract the ID (e.g. "Checking Scene 5:")
                parts = line.split()
                scene_id = parts[2].replace(':', '') if len(parts) > 2 else "?"
                print_status(f"Analizing Scene {scene_id}...")
                total_scenes += 1
            
            if "RESULTADO: VÁLIDO" in line:
                valid_count += 1
            
            if "RESULTADO: INVÁLIDO" in line or "RESULTADO: INVALIDO" in line:
                invalid_count += 1
                # Optional: If it fails, print the error line so you know why
                sys.stdout.write(f"\n❌ Failure detected in current scene.\n")

    except KeyboardInterrupt:
        sys.exit(0)

    # Clear progress line
    sys.stdout.write("\r\033[K")
    
    # 2. Print Summary
    print("=========================================")
    print(f"Pipeline Summary:")
    print(f"   - Scenes Checked: {total_scenes}")
    print(f"   - Valid:          {valid_count}")
    print(f"   - Invalid:        {invalid_count}")
    print("=========================================")

    # 3. Decision Logic
    if total_scenes > 0 and invalid_count == 0 and valid_count == total_scenes:
        print(">>> All images are VALID.")
        
        if not os.path.exists(EXTRACT_SCRIPT):
            print(f">>> ❌ Error: Script '{EXTRACT_SCRIPT}' not found.")
            sys.exit(1)

        # Assuming you want to save to memoria/graphs/ based on previous context
        output_dir = "memoria/graphs/"
        cmd = ["python3", EXTRACT_SCRIPT, args.log_file, output_dir]
        
        try:
            # Run extraction silently (or let it print its own success message)
            subprocess.run(cmd, check=True)
            print(">>> Pipeline Finished Successfully.")
        except subprocess.CalledProcessError:
            print(f">>> ❌ Error running extraction script.")
            sys.exit(1)
             
    else:
        print(">>> ⛔ Pipeline Stopped: Validation failed or incomplete.")
        sys.exit(1)

if __name__ == "__main__":
    main()
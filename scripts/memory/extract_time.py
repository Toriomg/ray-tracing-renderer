import re
import csv
import argparse
import sys
import subprocess
import os

def get_git_branch_name():
    """
    Retrieves the current git branch name.
    Returns 'output' if not in a git repository.
    """
    try:
        # Run 'git rev-parse --abbrev-ref HEAD' to get the current branch name
        result = subprocess.check_output(
            ["git", "rev-parse", "--abbrev-ref", "HEAD"], 
            stderr=subprocess.DEVNULL
        )
        branch = result.decode('utf-8').strip()
        
        # Sanitize filename: replace '/' with '_'
        clean_branch = re.sub(r'[\\/*?:"<>|]', '_', branch)
        return clean_branch
    except (subprocess.CalledProcessError, FileNotFoundError):
        return "output"

def main():
    # 1. Setup Argument Parser
    parser = argparse.ArgumentParser(description="Extract execution times to a CSV based on Git Branch.")
    
    # Input file (e.g., logs/txt/run.out)
    parser.add_argument("input_file", help="Path to the input text file (log).")
    
    # Output Directory (e.g., results/)
    parser.add_argument(
        "output_dir", 
        help="The directory where the CSV will be saved. The filename will be [GIT_BRANCH].csv"
    )
    
    args = parser.parse_args()

    # 2. Determine Output Path
    branch_name = get_git_branch_name()
    filename = f"{branch_name}.csv"
    
    # Combine the user-specified directory with the git-branch filename
    full_output_path = os.path.join(args.output_dir, filename)

    # 3. Create the destination directory if it doesn't exist
    try:
        os.makedirs(args.output_dir, exist_ok=True)
    except OSError as e:
        print(f"Error creating directory '{args.output_dir}': {e}")
        sys.exit(1)

    # 4. Define Regex Patterns
    scene_pattern = re.compile(r"res/scene_scripts/(scene\d+)\.txt")
    time_pattern = re.compile(r"^\s*([\d,\.]+)\s+seconds time elapsed")

    extracted_data = []
    current_scene = "Unknown"

    # 5. Process the file
    try:
        with open(args.input_file, 'r', encoding='utf-8') as f:
            for line in f:
                # Search for scene name
                scene_match = scene_pattern.search(line)
                if scene_match:
                    current_scene = scene_match.group(1)
                    continue

                # Search for time
                time_match = time_pattern.search(line)
                if time_match:
                    raw_time = time_match.group(1)
                    clean_time = raw_time.replace(',', '.')
                    
                    extracted_data.append({
                        'Scene': current_scene,
                        'Seconds': clean_time
                    })

    except FileNotFoundError:
        print(f"Error: The file '{args.input_file}' was not found.")
        sys.exit(1)
    except Exception as e:
        print(f"An error occurred: {e}")
        sys.exit(1)

    # 6. Write to CSV
    if extracted_data:
        try:
            with open(full_output_path, 'w', newline='', encoding='utf-8') as csvfile:
                fieldnames = ['Scene', 'Seconds']
                writer = csv.DictWriter(csvfile, fieldnames=fieldnames)

                writer.writeheader()
                for row in extracted_data:
                    writer.writerow(row)
            
            print(f"Success! Extracted {len(extracted_data)} records.")
            print(f"File saved to: {full_output_path}")
            
        except IOError as e:
            print(f"Error writing to file '{full_output_path}': {e}")
    else:
        print("No matching data found in the input file.")

if __name__ == "__main__":
    main()
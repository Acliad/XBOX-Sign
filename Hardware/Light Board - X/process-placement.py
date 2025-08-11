import argparse
from pathlib import Path
from dataclasses import dataclass

FILE_NOT_FOUND_ERROR = 1

@dataclass
class Replacements:
    old: str
    new: str

header_replacements = [
    Replacements("Ref", "Designator"),
    Replacements("PosX", "\"Mid X\""),
    Replacements("PosY", "\"Mid Y\""),
    Replacements("Rot", "Rotation"),
    Replacements("Side", "Layer")
]

def main(placement_file: Path):
    if not placement_file.exists():
        print(f"Error: The file {placement_file} does not exist.")
        exit(FILE_NOT_FOUND_ERROR)
    
    with open(placement_file, 'r') as file:
        lines = file.readlines()
        header = lines[0]
        for replacement in header_replacements:
            header = header.replace(replacement.old, replacement.new)
        lines[0] = header

    with open(placement_file, 'w') as file:
        file.writelines(lines)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Process a KiCAD placement file to conform to JLC PCB requirements.")
    parser.add_argument("file", type=Path, help="Path to the placement file to process.")
    
    args = parser.parse_args()
    main(args.file)

    
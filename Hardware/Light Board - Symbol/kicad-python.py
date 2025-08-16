#!/usr/bin/env python3
"""
KiCad Python Wrapper Script

This script finds and executes the Python interpreter bundled with KiCad,
allowing execution of scripts that require access to KiCad's Python libraries.

Usage:
    ./kicad-python.py <script.py> [arguments]

The script automatically detects the KiCad installation on the current platform
and executes the provided script using KiCad's Python interpreter, passing along
any additional arguments.

This is useful for running KiCad PCB automation scripts that depend on KiCad's
Python modules without having to manually locate and invoke the correct Python
interpreter with the proper environment.

Currently supports:
- macOS: Automatically locates KiCad's Python in standard installation locations
- Windows: Not yet tested

Author: Isaac Rex via Claude AI
Version: 1.0
"""

import os
import sys
import platform
import subprocess
from pathlib import Path

def find_kicad_python():
    """
    Find the KiCad Python executable path based on the current platform.
    
    Returns:
        str: Path to KiCad Python executable or None if not found
    """
    system = platform.system()
    
    if system == "Windows":
        # Common KiCad installation locations on Windows
        kicad_install_paths = [
            Path("C:/Program Files/KiCad"),
            Path("C:/Program Files (x86)/KiCad"),
            # Add custom installation location if provided in environment variable
            Path(os.environ.get("KICAD_PATH", "C:/KiCad"))
        ]
        
        # Find latest KiCad version (if multiple exist)
        for base_path in kicad_install_paths:
            if not base_path.exists():
                continue
                
            # Try to find the version-specific directories (like 7.0)
            versions = []
            try:
                # Check for version subdirectories
                for item in base_path.iterdir():
                    if item.is_dir() and item.name[0].isdigit():
                        versions.append(item)
            except (PermissionError, FileNotFoundError):
                continue
                
            # If version subdirectories found, sort numerically by version
            if versions:
                # Sort version directories numerically instead of lexicographically
                def version_key(path):
                    # Extract version components and convert to float for proper numeric comparison
                    try:
                        # Handle formats like "7.0" or "10.0"
                        return float(path.name.split()[0])
                    except (ValueError, IndexError):
                        # Fallback in case of non-standard naming
                        return 0
                
                versions.sort(key=version_key, reverse=True)
                
                for version_path in versions:
                    # Look for Python in the bin directory
                    python_path = version_path / "bin" / "python.exe"
                    if python_path.exists():
                        return str(python_path)
                        
                    # Check alternative path structure
                    python_path = version_path / "python" / "python.exe"  
                    if python_path.exists():
                        return str(python_path)
            
            # If no version directories or python not found, try default structure
            python_path = base_path / "bin" / "python.exe"
            if python_path.exists():
                return str(python_path)
                
            # Try another common structure
            python_path = base_path / "python" / "python.exe"
            if python_path.exists():
                return str(python_path)
                
        # If we get here, we didn't find KiCad's Python in common locations
        # Try checking Windows Registry
        try:
            import winreg
            with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, r"SOFTWARE\KiCad") as key:
                install_dir, _ = winreg.QueryValueEx(key, "InstallDir")
                if install_dir:
                    python_path = Path(install_dir) / "bin" / "python.exe"
                    if python_path.exists():
                        return str(python_path)
        except (ImportError, FileNotFoundError, PermissionError):
            pass
            
        print("KiCad Python not found on Windows")
        return None
    
    elif system == "Darwin":  # macOS
        # Common KiCad installation location on macOS
        kicad_app_paths = [
            Path("/Applications/KiCad/KiCad.app"),
            Path("/Applications/KiCad.app")
        ]
        
        # Find the KiCad.app location
        kicad_app_path = None
        for path in kicad_app_paths:
            if path.exists():
                kicad_app_path = path
                break
        
        if kicad_app_path:
            # First check the standard Python location
            python_path = kicad_app_path / "Contents/Frameworks/Python.framework/Versions/Current/bin/python3"
            
            # Resolve symlink if it exists
            if python_path.exists():
                python_path = python_path.resolve()
                return str(python_path)
                
            # If not found, try the direct Python executable
            python_path = kicad_app_path / "Contents/Frameworks/Python.framework/Versions/Current/Python"
            
            # Resolve symlink if it exists
            if python_path.exists():
                python_path = python_path.resolve()
                return str(python_path)
                
            # As a last resort, search in all of Frameworks
            frameworks_dir = kicad_app_path / "Contents/Frameworks"
            for root, dirs, files in os.walk(frameworks_dir):
                root_path = Path(root)
                for file in files:
                    if file == "python3" or file == "Python":
                        python_path = root_path / file
                        if python_path.is_file() and os.access(python_path, os.X_OK):  # Check if executable
                            # Resolve symlink if it exists
                            python_path = python_path.resolve()
                            return str(python_path)
        
        print("KiCad Python not found on macOS")
        return None
    
    else:
        print(f"Unsupported operating system: {system}")
        return None

def main():
    # Find KiCad Python executable
    kicad_python_path = find_kicad_python()
    
    if not kicad_python_path:
        print("Error: KiCad Python executable not found.")
        sys.exit(1)
    
    # Pass all arguments to the KiCad Python executable
    cmd = [kicad_python_path] + sys.argv[1:]
    
    try:
        # Execute the KiCad Python with all arguments
        result = subprocess.run(cmd, check=True)
        sys.exit(result.returncode)
    except subprocess.CalledProcessError as e:
        print(f"Error executing KiCad Python: {e}")
        sys.exit(e.returncode)
    except FileNotFoundError:
        print(f"Error: KiCad Python executable not found at {kicad_python_path}")
        sys.exit(1)

if __name__ == "__main__":
    main()

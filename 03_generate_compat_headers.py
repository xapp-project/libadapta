#!/usr/bin/env python3
"""
adw-compat-generator.py

This script generates a compatibility header in src/adw-compat.h that maps libadwaita symbols
to libadapta symbols, allowing code written for libadwaita to work with libadapta with
minimal changes.
"""

import os
import re
import sys

def find_header_files():
    """Find all .h files in the given directory."""
    header_files = []
    for root, _, files in os.walk("."):
        for file in files:
            if file.endswith('.h') and file != 'adw-compat.h':
                header_files.append(os.path.join(root, file))
    return header_files

def extract_symbols(header_files):
    """Extract types, macros, and functions from header files."""
    types = set()
    all_macros = set()
    functions = set()
    gobject_type_declarations = []  # Store G_DECLARE_*_TYPE declarations
    
    # Regular expressions to match different kinds of symbols
    type_pattern = re.compile(r'typedef\s+struct\s+\w+\s+(Adap\w+)')
    opaque_type_pattern = re.compile(r'typedef\s+struct\s+_(Adap\w+)')
    # This pattern captures the 4 components of G_DECLARE_*_TYPE
    g_declare_pattern = re.compile(r'G_DECLARE_\w+_TYPE\s*\(\s*(Adap\w+)\s*,\s*(adap_\w+)\s*,\s*(ADAP)\s*,\s*([A-Z_]+)')
    param_pattern = re.compile(r'\b(Adap\w+)\s+\*')
    macro_pattern = re.compile(r'#define\s+(ADAP_[A-Z_]+)')
    function_pattern = re.compile(r'\b(adap_\w+)\s*\(')

    for header_file in header_files:
        try:
            with open(header_file, 'r', encoding='utf-8') as f:
                content = f.read()
                
                # Extract types
                for match in type_pattern.finditer(content):
                    types.add(match.group(1))
                
                # Extract opaque types
                for match in opaque_type_pattern.finditer(content):
                    types.add(match.group(1))
                    
                # Extract and process GObject-style type declarations
                for match in g_declare_pattern.finditer(content):
                    type_name = match.group(1)
                    func_name = match.group(2)
                    prefix = match.group(3)
                    suffix = match.group(4)
                    
                    types.add(type_name)
                    # Save the full macro name for later
                    gobject_type_declarations.append(f"{prefix}_{suffix}")
                    
                # Extract types from function parameters
                for match in param_pattern.finditer(content):
                    types.add(match.group(1))
                
                # Extract macros
                for match in macro_pattern.finditer(content):
                    all_macros.add(match.group(1))
                
                # Extract functions
                for match in function_pattern.finditer(content):
                    functions.add(match.group(1))
        except UnicodeDecodeError:
            print(f"Warning: Could not read {header_file}, skipping...")
            continue
    
    # Ensure all macros from G_DECLARE_*_TYPE are included, even if they're not defined in a #define
    for macro in gobject_type_declarations:
        all_macros.add(macro)
    
    # Based on GTK/GObject conventions, classify macros
    casting_macros = set()
    constant_macros = set()
    
    for macro in all_macros:
        # All macros from G_DECLARE_*_TYPE are casting macros
        if macro in gobject_type_declarations:
            casting_macros.add(macro)
            continue
            
        # Common patterns for casting macros
        if (macro.startswith('ADAP_TYPE_') or   # Type registration macros
            macro.startswith('ADAP_IS_')):      # Type checking macros
            casting_macros.add(macro)
            continue
            
        # For other macros, check if they match a pattern similar to G_DECLARE_*_TYPE output
        is_casting_macro = False
        for type_name in types:
            # Remove 'Adap' prefix
            short_name = type_name[4:] if type_name.startswith('Adap') else type_name
            
            # Convert to underscore format (e.g., ToolbarView -> TOOLBAR_VIEW)
            parts = []
            current_part = ""
            for char in short_name:
                if char.isupper() and current_part:
                    parts.append(current_part)
                    current_part = char
                else:
                    current_part += char
            if current_part:
                parts.append(current_part)
            
            underscore_name = '_'.join(parts).upper()
            
            # Check if macro matches the pattern
            if macro == f"ADAP_{underscore_name}":
                is_casting_macro = True
                break
        
        if is_casting_macro:
            casting_macros.add(macro)
        else:
            constant_macros.add(macro)
    
    return types, casting_macros, constant_macros, functions

def generate_compat_header(types, casting_macros, constant_macros, functions):
    """Generate the compatibility header file."""
    with open("src/adw-compat.h", 'w', encoding='utf-8') as f:
        f.write("""
#ifndef _ADW_COMPAT_H
#define _ADW_COMPAT_H

/**
 * SECTION:adw-compat
 * @title: Adwaita Compatibility
 * @short_description: Compatibility layer for libadwaita code
 *
 * This header provides compatibility definitions to allow code written for
 * libadwaita to work with libadapta with minimal changes. Simply include
 * this header before including adapta.h in your code.
 *
 * Example:
 * ```c
 * #include <libadapta-1/adw-compat.h>
 * #include <libadapta-1/adapta.h>
 *
 * // Now use Adwaita class names and function names
 * AdwApplicationWindow *window = ADW_APPLICATION_WINDOW(adw_application_window_new(app));
 * ```
 */

/* General namespace mapping */
#define Adw Adap

/* Map libAdwaita (Adw) types to libAdapta (Adap) types */
""")

        # Add type mappings
        for type_name in sorted(types):
            adw_name = type_name.replace('Adap', 'Adw')
            f.write(f"#define {adw_name} {type_name}\n")

        f.write("\n/* Map libAdwaita (Adw) type casting macros to libAdapta (Adap) type casting macros */\n")

        # Add casting macro mappings - these take an argument
        for macro in sorted(casting_macros):
            adw_macro = macro.replace('ADAP_', 'ADW_')
            f.write(f"#define {adw_macro}(obj) {macro}(obj)\n")

        f.write("\n/* Map libAdwaita (Adw) constant macros to libAdapta (Adap) constant macros */\n")
            
        # Add constant macro mappings - these don't take arguments
        for macro in sorted(constant_macros):
            adw_macro = macro.replace('ADAP_', 'ADW_')
            f.write(f"#define {adw_macro} {macro}\n")

        f.write("\n/* Map libAdwaita (Adw) functions to libAdapta (Adap) functions */\n")

        # Add function mappings
        for func in sorted(functions):
            adw_func = func.replace('adap_', 'adw_')
            f.write(f"#define {adw_func} {func}\n")

        f.write("\n#endif /* _ADW_COMPAT_H */\n")

    print(f"Generated compatibility header: src/adw-compat.h")
    print(f"Mapped {len(types)} types, {len(casting_macros)} casting macros, {len(constant_macros)} constant macros, {len(functions)} functions.")

def main():
    header_files = find_header_files()
    
    if not header_files:
        print("No header files found")
        return 1

    types, casting_macros, constant_macros, functions = extract_symbols(header_files)
    generate_compat_header(types, casting_macros, constant_macros, functions)

    return 0

if __name__ == '__main__':
    sys.exit(main())
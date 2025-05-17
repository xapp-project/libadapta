#!/usr/bin/python3

import os

REPLACEMENTS = {
    "Adwaita": "Adapta",
    "Adw": "Adap"
}

def case_preserve_replace(text, old, new):
    if text.isupper():
        return new.upper()
    elif text.islower():
        return new.lower()
    elif text[0].isupper():
        return new.capitalize()
    else:
        return new

def replace_in_name(name):
    new_name = name
    for old, new in REPLACEMENTS.items():
        new_name = re_case_preserve_replace(new_name, old, new)
    return new_name

def re_case_preserve_replace(text, old, new):
    import re
    def repl(match):
        return case_preserve_replace(match.group(), old, new)
    return re.sub(re.escape(old), repl, text, flags=re.IGNORECASE)

def rename_items(root_dir="."):
    # Walk bottom-up to safely rename folders after contents
    for dirpath, dirnames, filenames in os.walk(root_dir, topdown=False):
        if ".git" in dirpath.split(os.sep):
            continue
        # Rename files
        for name in filenames:
            old_path = os.path.join(dirpath, name)
            new_name = replace_in_name(name)
            if new_name != name:
                new_path = os.path.join(dirpath, new_name)
                os.rename(old_path, new_path)
                print(f"Renamed file: {old_path} → {new_path}")

        # Rename directories
        for name in dirnames:
            old_path = os.path.join(dirpath, name)
            if ".git" in old_path.split(os.sep):
                continue
            new_name = replace_in_name(name)
            if new_name != name:
                new_path = os.path.join(dirpath, new_name)
                os.rename(old_path, new_path)
                print(f"Renamed dir: {old_path} → {new_path}")

if __name__ == "__main__":
    rename_items()

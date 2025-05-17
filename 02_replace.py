#!/usr/bin/python3

import os
import re

REPLACEMENTS = {
    "Adwaita": "Adapta",
    "Adw": "Adap",
    "READAPRITE": "READWRITE"
}

EXCLUDED_FILES = ["01_rename.py", "02_replace.py", "03_generate_compat_headers.py"]

def case_preserve_replace(text, old, new):
    def repl(match):
        word = match.group()
        if word.isupper():
            return new.upper()
        elif word.islower():
            return new.lower()
        elif word[0].isupper():
            return new.capitalize()
        else:
            return new
    return re.sub(re.escape(old), repl, text, flags=re.IGNORECASE)

def is_binary_file(filepath):
    try:
        with open(filepath, 'rb') as f:
            chunk = f.read(1024)
            return b'\0' in chunk
    except:
        return True

def is_excluded(filepath):
    abs_path = os.path.abspath(filepath)
    root_path = os.path.abspath(".")
    basename = os.path.basename(filepath)
    parent = os.path.dirname(abs_path)
    return (
        basename in EXCLUDED_FILES and parent == root_path or
        ".git" in abs_path.split(os.sep)
    )

def process_file(filepath):
    if is_binary_file(filepath) or is_excluded(filepath):
        return

    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
    except:
        return

    original = content
    for old, new in REPLACEMENTS.items():
        content = case_preserve_replace(content, old, new)

    if content != original:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Modified: {filepath}")

def walk_and_replace(root_dir="."):
    for dirpath, _, filenames in os.walk(root_dir):
        if ".git" in dirpath.split(os.sep):
            continue
        for filename in filenames:
            filepath = os.path.join(dirpath, filename)
            process_file(filepath)

if __name__ == "__main__":
    walk_and_replace()

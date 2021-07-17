#!/usr/bin/env python

import sys
import os
import re

"""
this is a small utility for find/replace in the scope of a project. pretty sure
like every modern IDE does this, but vim doesn't understand things outside the
current buffer or whatever so here it is
"""

def construct_file_list_r(base_path):
    file_list = []

    for file_name in os.listdir(base_path):
        file_path = os.path.join(base_path, file_name)

        if os.path.isdir(file_path):
            file_list += construct_file_list(file_path)
        else:
            file_list.append(file_path)

    return file_list

def construct_file_list(file_path):
    if os.path.isdir(file_path):
        return construct_file_list_r(file_path)
    elif os.path.isfile(file_path):
        return [file_path]
    else:
        print(f"\"{file_path}\" is not an existing directory or file.")
        exit(0)

def find_pattern(file_path, pattern):
    for file_name in construct_file_list(file_path):
        with open(file_name, "r") as f:
            for line_num, line in enumerate(f.readlines()):
                if re.search(pattern, line) != None:
                    print(f"{file_name}:{line_num + 1}: \t{line.strip()}")

def findname():
    if len(sys.argv) != 4:
        print("usage: findname [base path] [name]")
        exit(0)

    file_path, name = sys.argv[2:]

    print(f"finding all occurrences of \"{name}\":")

    find_pattern(file_path, r"\b" + re.escape(name) + r"\b")

def findmatch():
    if len(sys.argv) != 4:
        print("usage: findmatch [base path] [regex]")
        exit(0)

    file_path, regex = sys.argv[2:]

    print(f"finding all occurrences of /{regex}/:")

    find_pattern(file_path, regex)

def rename():
    if len(sys.argv) != 5:
        print("usage: rename [base path] [current name] [replacement name]")
        exit(0)

    file_path, cur_name, new_name = sys.argv[2:]

    print(f"replacing all occurrences of \"{cur_name}\" with \"{new_name}\"")

    pattern = r"\b" + re.escape(cur_name) + r"\b"

    for file_name in construct_file_list(file_path):
        text = ""

        with open(file_name, "r") as f:
            text = f.read()

        with open(file_name, "w") as f:
            f.write(re.sub(pattern, new_name, text))

def replace():
    if len(sys.argv) != 5:
        print("usage: replace [base path] [regex] [replacement]")
        exit(0)

    file_path, regex, replacement = sys.argv[2:]

    print(f"replacing all matches for /{regex}/ with \"{replacement}\"")

    for file_name in construct_file_list(file_path):
        text = ""

        with open(file_name, "r") as f:
            text = f.read()

        with open(file_name, "w") as f:
            for match in re.finditer(regex, text, re.MULTILINE):
                this_replace = replacement

                if len(match.groups()):
                    this_replace = this_replace.format(*match.groups())

                text = text.replace(match.group(0), this_replace)

            f.write(text)

def main():
    if len(sys.argv) < 2:
        print("use with an action as the first argument:")
        print("rename: rename a variable/function/thing")
        print("findname: list all occurrences of name")
        print("replace: replace regex matches")
        print("findmatch: list all regex matches")
        exit(0)

    action = sys.argv[1]

    # dispatch
    if action == "findname":
        findname()
    elif action == "findmatch":
        findmatch()
    elif action == "rename":
        rename()
    elif action == "replace":
        replace()
    else:
        print("unknown action, try again.")

if __name__ == "__main__":
    main()

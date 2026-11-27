#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."

for project in projects/*/; do
    pio run -s -d "$project" -t compiledb
done

python3 -I - <<'EOF'
import glob
import json
import os
import subprocess

toolchain = os.path.expanduser("~/.platformio/packages/toolchain-xtensa-esp-elf/bin/")


def system_include_dirs(compiler):
    output = subprocess.run([compiler, "-xc++", "-E", "-v", "/dev/null"],
                            capture_output=True, text=True).stderr
    lines = output.split("#include <...> search starts here:")[1].split("End of search list.")[0]
    return [os.path.normpath(line.strip()) for line in lines.splitlines() if line.strip()]


system_includes = " ".join(
    f"-isystem {path}" for path in system_include_dirs(toolchain + "xtensa-esp32-elf-g++"))
extra_flags = system_includes + " -D__XTENSA__=1"

entries = []
for path in sorted(glob.glob("projects/*/compile_commands.json")):
    for entry in json.load(open(path)):
        if entry["command"].startswith("xtensa-"):
            entry["command"] = toolchain + entry["command"]
        entry["command"] += " " + extra_flags
        entries.append(entry)
json.dump(entries, open("compile_commands.json", "w"))
print(f"compile_commands.json: {len(entries)} entries")
EOF

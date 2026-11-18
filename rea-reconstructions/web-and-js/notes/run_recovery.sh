#!/bin/bash
# Wakaru-backed module recovery for the four captured production bundles, then a
# second application-graph pass over the recovered module trees.
# Heap/limits: REA recovery = 1 worker, 1 GiB address space, 120 s deadline, 64 MiB input.
set -u
export REA_WAKARU_COMMAND=/root/.local/share/rea-tools/node_modules/@wakaru/cli-linux-x64/wakaru
REA=/usr/local/bin/rea
BASE=/root/wsl-demo/rea-reconstructions/web-and-js
B=$BASE/bundles
R=$BASE/recovered
E=$BASE/evidence
LOG=$BASE/notes/recovery_runs.log
mkdir -p "$R" "$E"
: > "$LOG"

rec () {
  name="$1"; shift
  main="$1"; shift
  addl=(); for p in "$@"; do addl+=(--additional-path "$p"); done
  echo "=== recover $name $(date -Is)" | tee -a "$LOG"
  "$REA" recover-javascript-sources "$main" "$R/$name" "${addl[@]}" \
      --extraction-mode structural --rewrite-level standard --format json \
      > "$E/${name}__recover.json" 2>>"$LOG"
  rc=$?
  echo "    rc=$rc modules=$(ls "$R/$name/modules" 2>/dev/null | wc -l) bytes=$(du -sk "$R/$name" 2>/dev/null | cut -f1)" | tee -a "$LOG"
  if [ -d "$R/$name/modules" ]; then
    "$REA" analyze-javascript-application "$R/$name/modules" --detail summary --max-heap-mb 768 \
        --evidence_output "$E/${name}_recovered__app.json" --token-limit 300 \
        > "$E/${name}_recovered__summary.json" 2>>"$LOG"
    echo "    graph rc=$? size=$(stat -c%s "$E/${name}_recovered__app.json" 2>/dev/null)" | tee -a "$LOG"
  fi
}

rec stateofleonida    "$B/stateofleonida/solmain2809_4.js"
rec tootr             "$B/tootr/index-CHfdltUy.js"
rec gta6-leonida-atlas "$B/gta6-leonida-atlas/assets/index-B34Su3Ax.js" \
    "$B/gta6-leonida-atlas/assets/leonida-coordinates-CXGuqbYF.js" \
    "$B/gta6-leonida-atlas/assets/leonida-evidence-BTSqoURv.js" \
    "$B/gta6-leonida-atlas/assets/gtadb-mx_UyCIg.js" \
    "$B/gta6-leonida-atlas/assets/public-path-DoqNwZJ4.js" \
    "$B/gta6-leonida-atlas/assets/jsx-runtime-DyZJNswf.js"
rec noclip-website    "$B/noclip.website/static/js/8.15df99b7.js" \
    "$B/noclip.website/static/js/863.6a594d03.js" \
    "$B/noclip.website/static/js/index.0843dfe9.js"
echo "RECOVERY DONE $(date -Is)" | tee -a "$LOG"

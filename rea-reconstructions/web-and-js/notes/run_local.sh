#!/bin/bash
# Sequential local REA application-graph reconstructions (heap-bounded: host has ~3 GiB free).
set -u
REA=/usr/local/bin/rea
OUT=/root/wsl-demo/rea-reconstructions/web-and-js/evidence
LOG=/root/wsl-demo/rea-reconstructions/web-and-js/notes/local_runs.log
mkdir -p "$OUT"
run () {
  name="$1"; path="$2"
  echo "=== $name  $path  $(date -Is)" | tee -a "$LOG"
  "$REA" analyze-javascript-application "$path" \
      --detail summary --max-heap-mb 768 \
      --evidence_output "$OUT/${name}__app.json" \
      --token-limit 400 > "$OUT/${name}__summary.yaml" 2>>"$LOG"
  echo "    rc=$? size=$(stat -c%s "$OUT/${name}__app.json" 2>/dev/null)" | tee -a "$LOG"
}
run noclip-website   /root/wsl-demo/oot-peripherals/noclip.website
run Ocarina-Atlas    /root/wsl-demo/oot-peripherals/Ocarina-Atlas
run gta6-leonida-atlas /root/wsl-demo/gta-projects/gta6-leonida-atlas
run npwd             /root/wsl-demo/gta-peripherals/tools-and-apps/npwd
run txAdmin          /root/orca-workspaces/gta-repos/txAdmin
echo "ALL DONE $(date -Is)" | tee -a "$LOG"

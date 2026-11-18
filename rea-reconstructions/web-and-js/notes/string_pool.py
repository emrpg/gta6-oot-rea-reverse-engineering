#!/usr/bin/env python3
"""String-pool inventory for the Leonida bundle, cross-checked against REA endpoint observations.

Regex over raw bytes is a report-side method, not REA output: it recovers literals that
live only in the obfuscator's string array and therefore never appear as an endpoint
observation at a call site.
"""
import csv
import json
import os
import re

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RAW = os.path.join(ROOT, 'bundles/stateofleonida/solmain2809_4.js')
REC = os.path.join(ROOT, 'recovered/stateofleonida/modules/bundle.js')
OUT = os.path.join(ROOT, 'reconstructions/16_stateofleonida_string_pool.csv')

PATTERNS = {
    'php_script': r"['\"`]([A-Za-z0-9_\-./]+\.php)",
    'php_action': r"api\.php\?action=([a-z_]+)",
    'data_json': r"['\"`](data/[A-Za-z0-9_\-./{}$]+\.json)",
    'data_maps': r"['\"`](data/[A-Za-z0-9_\-./{}$]*maps[A-Za-z0-9_\-./{}$]*)",
    'url_host': r"https?://([A-Za-z0-9.\-]+\.[A-Za-z]{2,})",
    'media_php_action': r"media-admin\.php\?action=([a-z\-]+)",
    'storage_key_prefix': r"['\"`]([a-zA-Z][A-Za-z0-9_]{3,30}(?:Settings|_stats|Enabled|Collapsed|Style|Path|Color|session))['\"`]",
}


def pool(text, pat):
    return sorted(set(re.findall(pat, text)))


raw = open(RAW, encoding='utf8', errors='replace').read()
rec = open(REC, encoding='utf8', errors='replace').read()

rea_literals = set()
with open(os.path.join(ROOT, 'reconstructions/01_stateofleonida_endpoints.csv')) as fh:
    for row in csv.DictReader(fh):
        rea_literals.add(row['literal_or_expression'])
rea_blob = ' '.join(rea_literals)

rows = []
for kind, pat in PATTERNS.items():
    a, b = pool(raw, pat), pool(rec, pat)
    for value in sorted(set(a) | set(b)):
        rows.append([kind, value, 'yes' if value in a else 'no',
                     'yes' if value in b else 'no',
                     'yes' if value and value in rea_blob else 'no'])

with open(OUT, 'w', newline='') as fh:
    w = csv.writer(fh)
    w.writerow(['category', 'literal', 'in_raw_obfuscated_bytes',
                'in_recovered_module', 'contained_in_rea_endpoint_observations'])
    w.writerows(rows)

counts = {}
for r in rows:
    counts.setdefault(r[0], [0, 0, 0])
    counts[r[0]][0] += r[2] == 'yes'
    counts[r[0]][1] += r[3] == 'yes'
    counts[r[0]][2] += r[4] == 'yes'
for k, v in counts.items():
    print(f'{k:18s} raw={v[0]:3d} recovered={v[1]:3d} in_rea_endpoints={v[2]:3d} (total {sum(1 for r in rows if r[0]==k)})')

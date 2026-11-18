#!/usr/bin/env python3
"""Project REA Evidence graphs into the committed reconstruction tables.

Every row keeps its evidence grade: observed/exact rows come from endpoint and
storage node observations with source ranges; inferred rows are labelled.
"""
import csv
import json
import os
import re
import subprocess
from collections import Counter

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
EV = os.path.join(ROOT, 'evidence')
OUT = os.path.join(ROOT, 'reconstructions')
os.makedirs(OUT, exist_ok=True)


def record(path):
    with open(os.path.join(EV, path)) as fh:
        bundle = json.load(fh)
    rec = bundle['records'][0]
    return rec['evidence_id'], rec['normalized_result']


def props(node):
    out = []
    for obs in node.get('observations', []):
        ev = obs.get('evidence') or {}
        p = dict(obs.get('properties') or {})
        p.setdefault('name', obs.get('label') or (node.get('identity') or {}).get('key'))
        p.setdefault('value', obs.get('label') or (node.get('identity') or {}).get('key'))
        p['_label'] = obs.get('label')
        p['_key'] = (node.get('identity') or {}).get('key')
        p['_namespace'] = (node.get('identity') or {}).get('namespace')
        loc = ev.get('location') or {}
        v = loc.get('value') or {}
        start = v.get('start') or {}
        p['_source'] = v.get('source') or v.get('path')
        p['_line'] = start.get('line')
        p['_column'] = start.get('column')
        p['_confidence'] = ev.get('confidence')
        p['_authority'] = ev.get('authority')
        p['_coverage'] = (ev.get('coverage') or {}).get('status')
        p['_operation'] = (ev.get('extractor') or {}).get('operation')
        p['_source_map'] = bool(obs.get('source_map_reference'))
        out.append(p)
    return out


def nodes(nr, kind):
    return [n for n in nr['graph']['nodes'] if n.get('kind') == kind]



def key_of(p):
    """Literal storage key when REA recorded ``properties.name``; otherwise dynamic."""
    name = p.get('name')
    return name if name else '(dynamic)'


def write_rows(name, header, rows):
    with open(os.path.join(OUT, name), 'w', newline='') as fh:
        w = csv.writer(fh)
        w.writerow(header)
        w.writerows(rows)
    print(f'{name}: {len(rows)} rows')


# --- 01 stateofleonida: static endpoint inventory from the recovered module ---
eid, nr = record('stateofleonida_recovered__app.json')
rows = []
for n in nodes(nr, 'endpoint'):
    for p in props(n):
        if p.get('value') is None:
            continue
        rows.append([p.get('value'), p.get('endpoint_kind'), p.get('mechanism'),
                     p.get('resolution'), p.get('interpretation'),
                     p.get('_authority'), p.get('_confidence'), p.get('_source'), p.get('_line')])
rows.sort(key=lambda r: (str(r[7]), r[8] or 0))
write_rows('01_stateofleonida_endpoints.csv',
           ['literal_or_expression', 'endpoint_kind', 'mechanism', 'resolution', 'interpretation',
            'authority', 'confidence', 'source', 'line'], rows)

# --- 02 stateofleonida: client storage keys ---
rows = []
for n in nodes(nr, 'storage'):
    for p in props(n):
        rows.append([key_of(p), p.get('storage_kind'), p.get('mechanism'),
                     p.get('_namespace'), p.get('_authority'), p.get('_confidence'), p.get('_source'), p.get('_line')])
rows.sort(key=lambda r: (str(r[1]), str(r[0])))
write_rows('02_stateofleonida_storage_keys.csv',
           ['key_or_dynamic', 'storage_kind', 'mechanism', 'identity_namespace', 'authority', 'confidence', 'source', 'line'], rows)

# --- 03 Ocarina-Atlas: Electron IPC surface, endpoints, storage, route candidates ---
eid_oa, nr_oa = record('Ocarina-Atlas__app.json')
rows = []
for kind in ('ipc-channel', 'ipc-handler', 'context-bridge-api', 'browser-window', 'service-worker'):
    for n in nodes(nr_oa, kind):
        for p in props(n):
            rows.append([kind, p.get('channel') or p.get('api_key') or p.get('expression') or n.get('label'),
                         p.get('side'), p.get('mode') or p.get('operation'), p.get('handler_kind'),
                         p.get('_authority'), p.get('_confidence'), p.get('_operation'), p.get('_source'), p.get('_line')])
rows.sort(key=lambda r: (str(r[0]), str(r[2]), str(r[1])))
write_rows('03_ocarina_atlas_electron_surface.csv',
           ['node_kind', 'identifier', 'side', 'mode', 'handler_kind', 'authority', 'confidence', 'extractor_operation', 'source', 'line'], rows)

rows = []
for n in nodes(nr_oa, 'endpoint'):
    for p in props(n):
        rows.append([p.get('value'), p.get('endpoint_kind'), p.get('mechanism'), p.get('resolution'),
                     p.get('_authority'), p.get('_confidence'), p.get('_source'), p.get('_line')])
rows.sort(key=lambda r: (str(r[1]), str(r[0])))
write_rows('04_ocarina_atlas_endpoints.csv',
           ['literal_or_expression', 'endpoint_kind', 'mechanism', 'resolution',
            'authority', 'confidence', 'source', 'line'], rows)

rows = []
for n in nodes(nr_oa, 'storage'):
    for p in props(n):
        rows.append([key_of(p), p.get('storage_kind'), p.get('mechanism'),
                     p.get('_namespace'), p.get('_authority'), p.get('_confidence'), p.get('_source'), p.get('_line')])
rows.sort(key=lambda r: (str(r[1]), str(r[0])))
write_rows('05_ocarina_atlas_storage_keys.csv',
           ['key_or_dynamic', 'storage_kind', 'mechanism', 'identity_namespace', 'authority', 'confidence', 'source', 'line'], rows)


# --- 06 txAdmin: router.ts registrations verified against scoped graph ---
router = '/root/orca-workspaces/gta-repos/txAdmin/core/modules/WebServer/router.ts'
pat = re.compile(r"router\.(get|post|put|delete|patch)\(\s*'([^']+)'")
src_rows = []
for i, line in enumerate(open(router), 1):
    m = pat.search(line)
    if m:
        src_rows.append([m.group(1).upper(), m.group(2), i])
_, nr_tr = record('txadmin_router_scoped__app.json')
rea_paths = Counter()
for n in nodes(nr_tr, 'endpoint'):
    for p in props(n):
        if p.get('value'):
            rea_paths[p['value']] += 1
rows = []
for verb, path, line in src_rows:
    rows.append([verb, path, f'router.ts:{line}', rea_paths.get(path, 0),
                 'source+graph' if rea_paths.get(path) else 'source-only'])
write_rows('06_txadmin_routes.csv', ['method', 'path', 'location', 'rea_endpoint_nodes', 'grade'], rows)
with open(os.path.join(OUT, '06_txadmin_routes_unmatched.json'), 'w') as fh:
    json.dump({'rea_paths_not_in_regex_table': sorted(p for p in rea_paths
                                                      if p not in {r[1] for r in src_rows}),
               'regex_rows_without_rea_node': [r[1] for r in rows if r[3] == 0]},
              fh, indent=1, ensure_ascii=False)

# --- 07 npwd and noclip: endpoint tables (network vs non-network) ---
for label, fname in (('npwd', 'npwd__app.json'), ('noclip', 'noclip-website__app.json'),
                     ('gta6_leonida_atlas', 'gta6-leonida-atlas__app.json')):
    _, nr_x = record(fname)
    rows = []
    for n in nodes(nr_x, 'endpoint'):
        for p in props(n):
            rows.append([p.get('value'), p.get('endpoint_kind'), p.get('mechanism'),
                         p.get('interpretation'), p.get('_authority'), p.get('_confidence'),
                         p.get('_source'), p.get('_line')])
    rows.sort(key=lambda r: (str(r[1]), str(r[0])))
    write_rows(f'08_{label}_endpoints.csv',
               ['literal_or_expression', 'endpoint_kind', 'mechanism', 'interpretation',
                'authority', 'confidence', 'source', 'line'], rows)

# --- 09 Electron-entry heuristic audit: every electron-* node with its basis ---
rows = []
for fname in ('npwd__app.json', 'Ocarina-Atlas__app.json', 'noclip-website__app.json'):
    _, nr_x = record(fname)
    for n in nodes(nr_x, 'electron-main'):
        for p in props(n):
            rows.append([fname, p.get('declared_path'), p.get('candidate_path'),
                         json.dumps(p.get('limitations'), ensure_ascii=False),
                         p.get('_authority'), p.get('_source'), p.get('_line')])
write_rows('10_electron_main_node_audit.csv',
           ['evidence_file', 'declared_path', 'candidate_path', 'limitations',
            'authority', 'location_artifact', 'line'], rows)

# --- 10 evidence ledger ---
ledger = []
for fname in sorted(os.listdir(EV)):
    if not fname.endswith('.json'):
        continue
    path = os.path.join(EV, fname)
    size = os.path.getsize(path)
    eid_x = None
    op = None
    if size < 300 * 1024 * 1024:
        try:
            with open(path) as fh:
                b = json.load(fh)
            rec = b['records'][0]
            eid_x = rec.get('evidence_id')
            op = rec.get('operation')
        except Exception:
            pass
    sha = subprocess.run(['sha256sum', path], capture_output=True, text=True).stdout.split()[0]
    ledger.append([fname, size, op, eid_x, sha])
write_rows('11_evidence_ledger.csv', ['file', 'bytes', 'operation', 'evidence_id', 'sha256'], ledger)

print('kinds Ocarina-Atlas:', Counter(n['kind'] for n in nr_oa['graph']['nodes']))
print('stateofleonida kinds:', Counter(n['kind'] for n in nr['graph']['nodes']))
print('txAdmin regex rows:', len(src_rows), 'rea distinct paths:', len(rea_paths))

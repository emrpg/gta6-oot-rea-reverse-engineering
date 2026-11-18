#!/usr/bin/env python3
"""Extract a compact, citable reconstruction profile from a canonical REA Evidence bundle.

Reads records[0].normalized_result of `rea analyze-javascript-application --evidence_output`
JSON and emits a small JSON profile plus markdown-ready counts. The full Evidence file stays
the authoritative source (cited by evidence_id + artifact digest); this only projects it.
"""
import json, sys, collections, re, os

URL_RE = re.compile(r"""https?://[^\s'"`(){}\[\],;<>\\]+""")
SKIP_KINDS = {'javascript-module', 'javascript-asset', 'artifact', 'package'}
MAX_ITEMS = 400


def obs_loc(o):
    v = ((o.get('evidence') or {}).get('location') or {}).get('value') or {}
    return v.get('source'), (v.get('start') or {}).get('line'), (v.get('start') or {}).get('column')


def literal_value(n):
    p = n.get('properties') or {}
    for key in ('value', 'literal', 'raw', 'url', 'path'):
        if isinstance(p.get(key), str):
            return p[key]
    lab = n.get('label')
    if isinstance(lab, str) and len(lab) > 1 and lab[0] in '"`\'':
        return lab.strip('`"\'')
    rk = ((n.get('identity') or {}).get('role_key') or '')
    m = re.search(r':(".*?"|`.*?`|\'.*?\')$', rk)
    return m.group(1)[1:-1] if m else None


def profile(path, out_dir, label=None):
    d = json.load(open(path))
    rec = d['records'][0]
    nr = rec['normalized_result']
    g, sg, ri = nr['graph'], nr['semantic_graph'], nr.get('route_inventory') or {}
    gnodes, sgnodes, sgrels = g['nodes'], sg['nodes'], sg['relations']

    structural = collections.defaultdict(list)
    for n in gnodes:
        k = n.get('kind')
        if k in SKIP_KINDS or len(structural[k]) >= MAX_ITEMS:
            continue
        for o in (n.get('observations') or [{}]):
            ev = o.get('evidence') or {}
            src, line, col = obs_loc(o)
            props = {kk: vv for kk, vv in (o.get('properties') or {}).items()
                     if isinstance(vv, (str, int, float, bool)) or vv is None}
            structural[k].append({
                'key': (n.get('identity') or {}).get('key'),
                'namespace': (n.get('identity') or {}).get('namespace'),
                'label': o.get('label') or n.get('label'),
                'properties': props,
                'confidence': ev.get('confidence'),
                'state': ev.get('state'),
                'authority': ev.get('authority'),
                'source': src, 'line': line, 'column': col,
            })

    boundaries = [n for n in sgnodes if n.get('kind') == 'boundary']
    lit_urls, lit_paths = collections.Counter(), collections.Counter()
    url_where = {}
    for n in lit_nodes_of(sg):
        val = literal_value(n)
        if not isinstance(val, str):
            continue
        for u in URL_RE.findall(val):
            lit_urls[u] += 1
            url_where.setdefault(u, loc_of(n))
        if (val.startswith('/') and len(val) > 2 and ' ' not in val) or val.endswith('.json') \
           or 'api' in val.lower() or val.startswith('data/'):
            lit_paths[val] += 1

    prof = {
        'label': label or os.path.basename(path).replace('.json', ''),
        'evidence_file': os.path.basename(path),
        'evidence_id': rec['evidence_id'],
        'operation': rec['operation'],
        'artifact_sha256': nr.get('root_artifact_sha256'),
        'inventory_manifest_sha256': nr.get('inventory_manifest_id'),
        'input_path': (rec.get('subject') or {}).get('local_path') or nr.get('input_path'),
        'statistics': nr.get('statistics'),
        'app_graph': {
            'graph_id': g.get('graph_id'),
            'nodes': len(gnodes),
            'node_kinds': dict(collections.Counter(str(n.get('kind')) for n in gnodes).most_common()),
            'edges': len(g.get('edges') or []),
            'edge_kinds': dict(collections.Counter(
                str(e.get('kind') or e.get('relation') or e.get('edge_kind') or 'unlabeled')
                for e in (g.get('edges') or [])).most_common()),
            'root_node_ids': g.get('root_node_ids'),
            'coverage': g.get('coverage'),
        },
        'semantic_graph': {
            'graph_id': sg.get('graph_id'),
            'nodes': len(sgnodes),
            'node_kinds': dict(collections.Counter(str(n.get('kind')) for n in sgnodes).most_common()),
            'relations': len(sgrels),
            'relation_kinds': dict(collections.Counter(str(r.get('relation')) for r in sgrels).most_common()),
            'resolution_states': dict(collections.Counter(str(r.get('resolution')) for r in sgrels).most_common()),
            'unknown_records': len(sg.get('unknowns') or []),
        },
        'structural_nodes': {str(k): v for k, v in sorted(structural.items(), key=lambda kv: str(kv[0]))},
        'boundaries_by_kind': dict(collections.Counter(
            str((n.get('properties') or {}).get('boundary_kind')) for n in boundaries).most_common()),
        'string_facts': {
            'urls': [{'url': u, 'count': c, 'location': url_where.get(u)} for u, c in lit_urls.most_common(80)],
            'data_paths': [{'value': p, 'count': c} for p, c in lit_paths.most_common(200)],
        },
        'route_inventory': {
            'inventory_id': ri.get('inventory_id'),
            'coverage': ri.get('coverage'),
            'statistics': ri.get('statistics'),
            'routes': (ri.get('routes') or [])[:200],
            'sources_retained': len(ri.get('sources') or []),
        },
        'limitation_count': len(nr.get('limitations') or []),
        'limitations': nr.get('limitations'),
    }
    os.makedirs(out_dir, exist_ok=True)
    stem = label or os.path.basename(path).replace('.json', '')
    out = os.path.join(out_dir, stem + '.profile.json')
    json.dump(prof, open(out, 'w'), indent=1, ensure_ascii=False, sort_keys=True)
    sk = prof['app_graph']['node_kinds']
    print(f"{out}: parsed={prof['statistics'].get('parsed_javascript_files')} "
          f"nodes={prof['app_graph']['nodes']} structural={ {k: len(v) for k, v in structural.items()} } "
          f"sem_nodes={prof['semantic_graph']['nodes']} urls={len(lit_urls)} paths={len(lit_paths)}")
    return prof


_LIT_CACHE = {}


def lit_nodes_of(sg):
    return [n for n in sg['nodes'] if n.get('kind') in ('literal', 'expression')]


def loc_of(node):
    v = (node.get('evidence') or {}).get('location') or {}
    val = v.get('value') or {}
    return val.get('source'), (val.get('start') or {}).get('line'), (val.get('end') or {}).get('column')


if __name__ == '__main__':
    label = sys.argv[3] if len(sys.argv) > 3 else None
    profile(sys.argv[1], sys.argv[2], label)

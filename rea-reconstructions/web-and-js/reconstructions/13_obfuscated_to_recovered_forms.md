# 混淆形态 → 还原形态：同一端点的两种字面量

配对方法：把两侧端点字面量按规范形（去除引号/反引号/`${}`/`+`/空白，`['k']`→`.k`）折叠后比较，规范形相同而原文不同者计为“形态差异对”。这是本报告的推导规则，非 REA 输出。

- 混淆打包 `bundles/stateofleonida/solmain2809_4.js`（1 行、9,460 个 `_0x…` 标识符、381,100 AST 节点）端点字面量：**125** 个
- Wakaru 结构还原 `recovered/stateofleonida/modules/bundle.js`（44,831+ 行、附 `bundle.js.map`）端点字面量：**125** 个
- 规范形集合：相同 97；形态差异对 28；仅混淆侧 0；仅还原侧 0
- 结论：还原是**语义保持**变换；端点总数与规范形集合不变，改变的只是可读形（拼接→模板字面量、下标属性→点属性、单行→带行号可寻址模块）。

| # | 混淆侧原文 | 还原侧原文 | 规范形 |
|---|---|---|---|
| 1 | `AUTH_CONFIG['api']['baseUrl']+'?action=bookmark'` | ``${AUTH_CONFIG.api.baseUrl}?action=bookmark`` | `AUTH_CONFIG.api.baseUrl?action=bookmark` |
| 2 | `CONTENT_CONFIG['dataFile']+'?t='+Date['now']()` | ``${CONTENT_CONFIG.dataFile}?t=${Date.now()}`` | `CONTENT_CONFIG.dataFile?t=Date.now()` |
| 3 | `_0x3cc11d['file']+'?v='+timestamp` | ``${_0x3cc11d.file}?v=${timestamp}`` | `_0x3cc11d.file?v=timestamp` |
| 4 | `_0x470ae4+'?action=xp_marker_discover'` | ``${_0x470ae4}?action=xp_marker_discover`` | `_0x470ae4?action=xp_marker_discover` |
| 5 | `_0x50b753['toString']()` | `_0x50b753.toString()` | `_0x50b753.toString()` |
| 6 | `'auth/api.php?action=admin_db_integrity&id='+encodeURIComponent(_0x1839cf)` | ``auth/api.php?action=admin_db_integrity&id=${encodeURIComponent(_0x1839cf)}`` | `auth/api.php?action=admin_db_integrity&id=encodeURIComponent` |
| 7 | `'auth/api.php?action=admin_user_details&id='+encodeURIComponent(_0x2bf704)` | ``auth/api.php?action=admin_user_details&id=${encodeURIComponent(_0x2bf704)}`` | `auth/api.php?action=admin_user_details&id=encodeURIComponent` |
| 8 | `'auth/api.php?action=admin_users&'+_0x1f4e74` | ``auth/api.php?action=admin_users&${_0x1f4e74}`` | `auth/api.php?action=admin_users&_0x1f4e74` |
| 9 | `'auth/api.php?action=leaderboard&game=screenshot_challenge&limit='+limit` | ``auth/api.php?action=leaderboard&game=screenshot_challenge&limit=${limit}`` | `auth/api.php?action=leaderboard&game=screenshot_challenge&li` |
| 10 | `'auth/api.php?action=marker_list&type='+this['_state']['type']` | ``auth/api.php?action=marker_list&type=${this._state.type}`` | `auth/api.php?action=marker_list&type=this._state.type` |
| 11 | `'auth/api.php?action=marker_list&type='+type` | ``auth/api.php?action=marker_list&type=${type}`` | `auth/api.php?action=marker_list&type=type` |
| 12 | `'data/'+_0x4400a3` | ``data/${_0x4400a3}`` | `data/_0x4400a3` |
| 13 | `'data/logs_'+_0x3ed687['toLowerCase']()+'.json?v='+timestamp` | ``data/logs_${_0x3ed687.toLowerCase()}.json?v=${timestamp}`` | `data/logs__0x3ed687.toLowerCase().json?v=timestamp` |
| 14 | `'data/maps/'+_0x1877ef+'.json'` | ``data/maps/${_0x1877ef}.json`` | `data/maps/_0x1877ef.json` |
| 15 | `'data/wiki-cache.json?t='+Date['now']()` | ``data/wiki-cache.json?t=${Date.now()}`` | `data/wiki-cache.json?t=Date.now()` |
| 16 | `'get-media-files.php?type='+encodeURIComponent(tab['id'])` | ``get-media-files.php?type=${encodeURIComponent(tab.id)}`` | `get-media-files.php?type=encodeURIComponent(tab.id)` |
| 17 | `'get-media-files.php?type='+encodeURIComponent(type)` | ``get-media-files.php?type=${encodeURIComponent(type)}`` | `get-media-files.php?type=encodeURIComponent(type)` |
| 18 | `'get-media-files.php?type='+type` | ``get-media-files.php?type=${type}`` | `get-media-files.php?type=type` |
| 19 | `sourceConfig['url']` | `sourceConfig.url` | `sourceConfig.url` |
| 20 | `state['originalSrc']` | `state.originalSrc` | `state.originalSrc` |
| 21 | `this['_apiUrl'](action)['toString']()` | `this._apiUrl(action).toString()` | `this._apiUrl(action).toString()` |
| 22 | `this['_apiUrl'](action,_0x99e6dc)['toString']()` | `this._apiUrl(action, _0x99e6dc).toString()` | `this._apiUrl(action,_0x99e6dc).toString()` |
| 23 | `this['_url'](action)` | `this._url(action)` | `this._url(action)` |
| 24 | `this['_url']('profile_save')` | `this._url('profile_save')` | `this._url(profile_save)` |
| 25 | `this['changelogFile']+'?v='+timestamp` | ``${this.changelogFile}?v=${timestamp}`` | `this.changelogFile?v=timestamp` |
| 26 | `this['changelogSaveEndpoint']` | `this.changelogSaveEndpoint` | `this.changelogSaveEndpoint` |
| 27 | `this['proxyUrl']+'?action=wiki_save_cache'` | ``${this.proxyUrl}?action=wiki_save_cache`` | `this.proxyUrl?action=wiki_save_cache` |
| 28 | `this['url']` | `this.url` | `this.url` |

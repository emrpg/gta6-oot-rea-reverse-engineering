# 输入产物与 Evidence 身份对照

| 输入（本次工作区内路径） | 原始字节 SHA-256 | REA root_artifact_sha256 | Evidence ID |
|---|---|---|---|
| `bundles/stateofleonida/solmain2809_4.js` | `0f5ab96c788a6e8d…` | `f62c7c0d7c384d6a…` | `ev_f59510342eac7a56c022…` |
| `recovered/stateofleonida/modules/bundle.js` | `e7ae051d8afec02d…` | `7e075943c7630b4a…` | `ev_5a75e5257d5160f6127c…` |
| `bundles/tootr/index-CHfdltUy.js` | `f1e0ea9556b4b928…` | `d80b532325dd0558…` | `ev_52f57551f29fb436dbf1…` |
| `recovered/tootr/modules/bundle.js` | `d39dafdf7d9525ac…` | `69e2d04f180a18c6…` | `ev_1e4f15fdfbaaab5a2ed4…` |
| `bundles/noclip.website/static/js/8.15df99b7.js` | `ef73ed494db8e130…` | `c2710ed4d9236fa0…` | `ev_709df08d6a93b4591458…` |
| `recovered/noclip-website/modules` | `(目录)…` | `6c79fffebd08d22d…` | `ev_c23667c3156add520ed1…` |
| `bundles/gta6-leonida-atlas/assets` | `(目录)…` | `8eea0aa5c0ee7308…` | `ev_aabf94a72f4e489872d9…` |
| `/root/wsl-demo/gta-projects/gta6-leonida-atlas` | `(目录)…` | `34ec455d882d00f7…` | `ev_136c2b5082f8981572ce…` |
| `/root/wsl-demo/oot-peripherals/Ocarina-Atlas` | `(目录)…` | `451baa2b1946465b…` | `ev_0286954cf953b050e547…` |
| `/root/wsl-demo/gta-peripherals/tools-and-apps/npwd` | `(目录)` | `58e618c485443aca…` | `ev_636c583a17888dde2ed8…` |
| `/root/orca-workspaces/gta-repos/txAdmin` | `(目录)…` | `b27b3e335693576f…` | `ev_b77cf012815b13427dd9…` |
| `local/leonida-source-coords` | `(目录)…` | `9e6de52361c2a434…` | `ev_32825cf5467fed162a15…` |
| `local/txadmin-router` | `(目录)…` | `e34a3aeac3247695…` | `ev_868fb82752f5859d24e0…` |

> `原始字节 SHA-256` 为磁盘文件摘要；`root_artifact_sha256` 为 REA 图内产物身份（规范化摘要），两者不相等属预期，不可互相替代。

## 二次抓取稳定性核验（2026-11-18，UTC+08 会话内）

公开静态资产重复 `GET` 一次并与落地字节比对，7/7 摘要相同；下表为落地文件 SHA-256（全值）。

| 落地文件 | 字节 | SHA-256 | 来源 URL |
|---|---:|---|---|
| `bundles/stateofleonida/solmain2809_4.js` | 2,928,855 | `0f5ab96c788a6e8d61759f7de9f4cfce61ca47f2582ca8b4d203dd0e0008f87f` | https://map.stateofleonida.net/solmain2809_4.js |
| `bundles/stateofleonida/solmain2809_2.css` | 792,112 | `c49fe66e7cd671bf5d9cd686dd4359deda83530441e490c6b3db6c2a93d2236a` | https://map.stateofleonida.net/solmain2809_2.css |
| `bundles/noclip.website/static/js/8.15df99b7.js` | 7,677,171 | `ef73ed494db8e1305a29de840cf9cc5959f71483288441b3b4a561ebe839c071` | https://noclip.website/static/js/8.15df99b7.js |
| `bundles/tootr/index-CHfdltUy.js` | 5,339,984 | `f1e0ea9556b4b92847d7cd3a8902262c2ded88c34687928113d8db27936c4388` | https://tootr.mracsys.com/assets/index-CHfdltUy.js |
| `bundles/gta6-leonida-atlas/assets/index-B34Su3Ax.js` | 579,650 | `d81d24b7da29b91cb51d9c6780face2949d4ad5dba9b35e945c76e1b6a3973f0` | https://gta6state.com/gta6-leonida-atlas/assets/index-B34Su3Ax.js |
| `bundles/gta6-leonida-atlas/assets/leonida-coordinates-CXGuqbYF.js` | 586 | `c65bc30cb288ba0d62c379803efcc921b8d756ce66611721c0ca18d0824d15ec` | https://gta6state.com/gta6-leonida-atlas/assets/leonida-coordinates-CXGuqbYF.js |
| `bundles/gta6-leonida-atlas/assets/public-path-DoqNwZJ4.js` | 300 | `de888f99d993bf96f63e62e4dc421e5d87e78346c9f332495534f87a2bf71e94` | https://gta6state.com/gta6-leonida-atlas/assets/public-path-DoqNwZJ4.js |

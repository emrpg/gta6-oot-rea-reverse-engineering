// Reconstructed from REA static analysis of stateofleonida (solmain2809_4.js)
// Evidence: /root/wsl-demo/rea-reconstructions/web-and-js/evidence/stateofleonida__app.json
// SHA-256: f62c7c0d7c384d6ac3f0b13101277f134989e01cdd1e8d35af264602906e779f (2,928,855 bytes, 381,100 AST nodes)

export type StateOfLeonidaApiAction =
  | 'access_config'
  | 'access_verify'
  | 'gtadb_source_public'
  | 'site_settings_public'
  | 'ranks'
  | 'notifications'
  | 'notifications_read'
  | 'marker_list'
  | 'marker_save'
  | 'marker_update'
  | 'marker_delete'
  | 'game_stats'
  | 'leaderboard';

export type StateOfLeonidaAdminAction =
  | 'admin_community_access_save'
  | 'admin_credits'
  | 'admin_credits_save'
  | 'admin_db_export'
  | 'admin_db_integrity'
  | 'admin_db_overview'
  | 'admin_gtadb_save'
  | 'admin_gtadb_scheduled'
  | 'admin_gtadb_status'
  | 'admin_gtadb_sync'
  | 'admin_gtadb_sync_status'
  | 'admin_news'
  | 'admin_news_save'
  | 'admin_ranks'
  | 'admin_seo'
  | 'admin_seo_save'
  | 'admin_site_settings'
  | 'admin_site_settings_save'
  | 'admin_stats'
  | 'admin_user_delete'
  | 'admin_user_details'
  | 'admin_user_set_rank'
  | 'admin_users';

export const STATE_OF_LEONIDA_DATA_DATASETS = [
  'data/backgrounds.json',
  'data/content.json',
  'data/credits.json',
  'data/database_changelog.json',
  'data/el_screenshots.json',
  'data/en-emojibase-data.json',
  'data/lang.json',
  'data/leaks.json',
  'data/markers.json',
  'data/news.json',
  'data/screenshots.json',
  'data/seo.json',
  'data/supporters.json',
  'data/trailer1.json',
  'data/trailer2.json',
  'data/trailer3.json',
  'data/wiki-cache.json',
] as const;

export async function callStateOfLeonidaApi<T>(
  action: StateOfLeonidaApiAction | StateOfLeonidaAdminAction,
  params: Record<string, string | number> = {},
): Promise<T> {
  const qs = new URLSearchParams({ action });
  for (const [k, v] of Object.entries(params)) {
    qs.set(k, String(v));
  }
  const res = await fetch(`auth/api.php?${qs.toString()}`, {
    credentials: 'same-origin',
  });
  if (!res.ok) {
    throw new Error(`StateOfLeonida API error: ${res.status}`);
  }
  return res.json() as Promise<T>;
}

export async function fetchGtadbPublicSource(): Promise<unknown> {
  return callStateOfLeonidaApi('gtadb_source_public');
}

export async function fetchScreenshotChallengeLeaderboard(limit = 10): Promise<unknown> {
  return callStateOfLeonidaApi('leaderboard', { game: 'screenshot_challenge', limit });
}

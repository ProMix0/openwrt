#include <drv_types.h>

typedef struct rhashtable prestatbl_t;

static u32 rtw_presta_table_hash(const void *addr, u32 len, u32 seed)
{
	/* Use last four bytes of hw addr as hash index */
	return jhash_1word(*(u32 *)(addr+2), seed);
}

const static struct rhashtable_params rtw_presta_params = {
  .automatic_shrinking = true,
  .key_len = ETH_ALEN,
  .key_offset = offsetof(struct rtw_presta_s, mac_addr),
  .head_offset = offsetof(struct rtw_presta_s, linkage),
  .hashfn = rtw_presta_table_hash,
};

static void _rtw_presta_free(void *ptr, void *arg) {
	rtw_mfree(ptr, sizeof(struct rtw_presta_s));
}

static void rtw_presta_tbl_init(prestatbl_t *presta_tbl) {
	rtw_rhashtable_init(presta_tbl, &rtw_presta_params);
}

static void rtw_presta_tbl_deinit(prestatbl_t *presta_tbl) {
	rtw_rhashtable_free_and_destroy(presta_tbl, _rtw_presta_free, NULL);
}

static int rtw_presta_tbl_insert(prestatbl_t *presta_tbl, struct rtw_presta_s *presta) {
  return rtw_rhashtable_lookup_insert_fast(presta_tbl, &presta->linkage, rtw_presta_params);
}

static struct rtw_presta_s *rtw_presta_tbl_get(prestatbl_t *presta_tbl, const u8 sta_mac[ETH_ALEN]) {
  return (struct rtw_presta_s *)rtw_rhashtable_lookup_fast(presta_tbl, sta_mac, rtw_presta_params);
}


struct rtw_presta_s *rtw_get_presta(struct sta_priv *pstapriv, const u8 sta_mac[ETH_ALEN]) {
	prestatbl_t *presta_tbl = pstapriv->presta_tbl;
	struct rtw_presta_s *res = rtw_presta_tbl_get(presta_tbl, sta_mac);
	if (res) return res;

	res = rtw_zmalloc(sizeof(struct rtw_presta_s));
	if (res == NULL) goto fail;
	_rtw_memcpy(res->mac_addr, sta_mac, ETH_ALEN);

	rtw_presta_tbl_insert(presta_tbl, res);

	pstapriv->rtw_presta_s_cnt++;

	return res;
fail:
	if (res) rtw_mfree(res, sizeof(struct rtw_presta_s));
	return NULL;
}

struct rtw_presta_s *rtw_retrieve_presta(struct sta_priv *pstapriv, const u8 sta_mac[ETH_ALEN]) {
  return rtw_presta_tbl_get(pstapriv->presta_tbl, sta_mac);
}

void rtw_init_presta_tbl(struct sta_priv *pstapriv) {
	rtw_presta_tbl_init(pstapriv->presta_tbl);
}

void rtw_deinit_presta_tbl(struct sta_priv *pstapriv) {
	rtw_presta_tbl_deinit(pstapriv->presta_tbl);
}

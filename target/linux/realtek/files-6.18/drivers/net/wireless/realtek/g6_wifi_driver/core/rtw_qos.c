#include <drv_types.h>

#define _bR1(__pStart) BE_BITS_TO_1BYTE(__pStart, 0, 8)
#define _bR2(__pStart) BE_BITS_TO_2BYTE(__pStart, 0, 16)
#define _bR4(__pStart) BE_BITS_TO_4BYTE(__pStart, 0, 32)

#ifdef CONFIG_QOS_MANAGEMENT

typedef u8 qos_scs_hash_t;

static inline qos_scs_hash_t _hsh_generic_1(const u8 *dst, u8 len) {
	qos_scs_hash_t res = 0;
	int i;
	for (i = 0; i < len; i++) {
		res ^= dst[i];
	}
	return res;
}

static u8 skbr_eth_src_addr(u8 *dst, const u8 *data) {
	memcpy(dst, data + ETH_ALEN, ETH_ALEN);
	return 0;
}

static u8 skbr_eth_dst_addr(u8 *dst, const u8 *data) {
	memcpy(dst, data, ETH_ALEN);
	return 0;
}

static qos_scs_hash_t hsh_eth_addr(const u8 *dst) {
	return _hsh_generic_1(dst, ETH_ALEN);
}

static u8 mchf_eth_addr(const u8 *a1, const u8 *a2) {
	return !memcmp(a1, a2, ETH_ALEN);
}

static u8 skbr_eth_type(u8 *dst, const u8 *data) {
	memcpy(dst, data + 2 * ETH_ALEN, 2);
	return 0;
}

static qos_scs_hash_t hsh_eth_type(const u8 *dst) {
	return _hsh_generic_1(dst, 2);
}

static u8 mchf_eth_type(const u8 *a1, const u8 *a2) {
	return !memcmp(a1, a2, 2);
}

static u8 skbr_ip_ver(u8 *dst, const u8 *data) {
	memcpy(dst, data + ETH_HLEN, 1);
	*dst &= ~((u8) 15);
	return 0;
}

static qos_scs_hash_t hsh_ip_ver(const u8 *dst) {
	qos_scs_hash_t res = *dst;
	return res;
}

static u8 mchf_ip_ver(const u8 *a1, const u8 *a2) {
	return !memcmp(a1, a2, 1);
}

static u8 skbr_ip_ihl(u8 *dst, const u8 *data) {
	memcpy(dst, data + ETH_HLEN, 1);
	*dst &= ((u8) 15);
	return 0;
}

static u8 skbr_ip_src_v4(u8 *dst, const u8 *data) {
	u8 ip_ver;
	skbr_ip_ver(&ip_ver, data);
	if (ip_ver == 4) {
		memcpy(dst, data + ETH_HLEN + 12, 4);
	} else {
		memset(dst, 0, 4);
	}
	return 0;
}

static u8 skbr_ip_dst_v4(u8 *dst, const u8 *data) {
	u8 ip_ver;
	skbr_ip_ver(&ip_ver, data);
	if (ip_ver == 4) {
		memcpy(dst, data + ETH_HLEN + 16, 4);
	} else {
		memset(dst, 0, 4);
	}
	return 0;
}

static qos_scs_hash_t hsh_ip_addr_v4(const u8 *dst) {
	return _hsh_generic_1(dst, 4);
}

static u8 mchf_ip_addr_v4(const u8 *a1, const u8 *a2) {
	return !memcmp(a1, a2, 4);
}

static u8 skbr_ip_src_v6(u8 *dst, const u8 *data) {
	u8 ip_ver;
	skbr_ip_ver(&ip_ver, data);
	if (ip_ver == 6) {
		memcpy(dst, data + ETH_HLEN + 8, 16);
	} else {
		memset(dst, 0, 16);
	}
	return 0;
}

static u8 skbr_ip_dst_v6(u8 *dst, const u8 *data) {
	u8 ip_ver;
	skbr_ip_ver(&ip_ver, data);
	if (ip_ver == 6) {
		memcpy(dst, data + ETH_HLEN + 24, 16);
	} else {
		memset(dst, 0, 16);
	}
	return 0;
}

static qos_scs_hash_t hsh_ip_addr_v6(const u8 *dst) {
	return _hsh_generic_1(dst, 16);
}

static u8 mchf_ip_addr_v6(const u8 *a1, const u8 *a2) {
	return !memcmp(a1, a2, 16);
}

static u8 skbr_ip_dscp(u8 *dst, const u8 *data) {
	u8 ip_ver;
	skbr_ip_ver(&ip_ver, data);
	if (ip_ver == 4) {
		memcpy(dst, data + ETH_HLEN + 1, 1);
		*dst &= ~((u8) 3);
	} else {
		memset(dst, 0, 1);
	}
	return 0;
}

static qos_scs_hash_t hsh_ip_dscp(const u8 *dst) {
	return _hsh_generic_1(dst, 1);
}

static u8 mchf_ip_dscp(const u8 *a1, const u8 *a2) {
	return !memcmp(a1, a2, 1);
}

static u8 skbr_ip_prot_or_nhdr(u8 *dst, const u8 *data) {
	u8 ip_ver;
	skbr_ip_ver(&ip_ver, data);
	if (ip_ver == 6) {
		memcpy(dst, data + ETH_HLEN + 6, 1);
	} else if (ip_ver == 4) {
		memcpy(dst, data + ETH_HLEN + 9, 1);
	} else {
		memset(dst, 0, 1);
	}
	return 0;
}

static qos_scs_hash_t hsh_ip_prot_or_nhdr(const u8 *dst) {
	return _hsh_generic_1(dst, 1);
}

static u8 mchf_ip_prot_or_nhdr(const u8 *a1, const u8 *a2) {
	return !memcmp(a1, a2, 1);
}

static u8 skbr_ip_flow_lbl(u8 *dst, const u8 *data) {
	u8 ip_ver;
	skbr_ip_ver(&ip_ver, data);
	if (ip_ver == 6) {
		memcpy(dst, data + ETH_HLEN + 1, 3);
		dst[0] &= 15;
	} else {
		memset(dst, 0, 3);
	}
	return 0;
}

static qos_scs_hash_t hsh_ip_flow_lbl(const u8 *dst) {
	return _hsh_generic_1(dst, 3);
}

static u8 mchf_ip_flow_lbl(const u8 *a1, const u8 *a2) {
	return !memcmp(a1, a2, 3);
}

static inline u8 _calc_ip_hdr_len(const u8 *data) {
	u8 ip_ver, ihl, ip_hdr_len = 0;
	skbr_ip_ver(&ip_ver, data);
	if (ip_ver == 4) {
		skbr_ip_ihl(&ihl, data);
		ip_hdr_len = ihl << 2;
	} else if (ip_ver == 6) {
		ip_hdr_len = 40;
		// TODO next header
	} else {
		ip_hdr_len = (u8)-1;
	}
	return ip_hdr_len;
}

static u8 _det_tsp_typ(const u8 *data) {
	u8 protocol;
	skbr_ip_prot_or_nhdr(&protocol, data);
	return protocol;
}

static u8 skbr_tsp_srcport(u8 *dst, const u8 *data) {
	u8 ip_hdr_len, prtc;
	prtc = _det_tsp_typ(data);
	if (prtc != IPP_TCP && prtc != IPP_UDP) return 1;
	ip_hdr_len = _calc_ip_hdr_len(data);
	if (ip_hdr_len == (u8) -1) return 1;
	memcpy(dst, data + ETH_HLEN + ip_hdr_len, 2);
	return 0;
}

static u8 skbr_tsp_dstport(u8 *dst, const u8 *data) {
	u8 ip_hdr_len, prtc;
	prtc = _det_tsp_typ(data);
	if (prtc != IPP_TCP && prtc != IPP_UDP) return 1;
	ip_hdr_len = _calc_ip_hdr_len(data);
	if (ip_hdr_len == (u8) -1) return 1;
	memcpy(dst, data + ETH_HLEN + ip_hdr_len + 2, 2);
	return 0;
}

static qos_scs_hash_t hsh_tsp_port(const u8 *dst) {
	return _hsh_generic_1(dst, 2);
}

static u8 mchf_tsp_port(const u8 *a1, const u8 *a2) {
	return !memcmp(a1, a2, 2);
}

struct offset_tbl_entry {
	u8 idx;
	size_t dst;
	u8 (*parser)(u8 *dst, const u8 *data);
	u8 (*parser2)(u8 *dst, const u8 *data);
	qos_scs_hash_t (*hasher)(const u8 *dst);
	u8 (*match_f)(const u8 *a1, const u8 *a2);
};

/////

static inline qos_scs_hash_t qos_scs_calc_hash_tclas_generic_1(struct offset_tbl_entry *tbl, u8 tbl_sz, const u8 *s, size_t mask_offset) {
	qos_scs_hash_t res = 0;
	struct offset_tbl_entry e;
	u8 i;
	for (i = 0; i < tbl_sz; i++) {
		e = tbl[i];
		if (!(*(s + mask_offset) & (1 << e.idx))) continue;
		res ^= e.hasher(s + e.dst);
	}
	return res;
}

static inline u8 qos_scs_dd_fill_tclas_generic_1(struct offset_tbl_entry *tbl, u8 tbl_sz, u8 *s, const struct sk_buff *pkt, u8 mirrored, size_t mask_offset) {
	u8 res = 0, i, idx;
	struct offset_tbl_entry e;
	for (i = 0; i < tbl_sz; i++) {
		e = tbl[i];
		if (!(*(s + mask_offset) & (1 << e.idx))) continue;
		if (mirrored) {
			res |= e.parser2(s + e.dst, pkt->data);
		} else {
			res |= e.parser(s + e.dst, pkt->data);
		}
	}
	return res;
}

static inline u8 qos_scs_dd_match_tclas_generic_1(struct offset_tbl_entry *tbl, u8 tbl_sz, const u8 *s1, const u8 *s2, size_t mask_offset) {
	u8 i, idx;
	struct offset_tbl_entry e;
	if (*(s1 + mask_offset) != *(s2 + mask_offset)) return 0;
	for (i = 0; i < tbl_sz; i++) {
		e = tbl[i];
		if (!(*(s1 + mask_offset) & (1 << e.idx))) continue;
		if (!e.match_f(s1 + e.dst, s2 + e.dst)) return 0;
	}
	return 1;
}

static struct offset_tbl_entry qos_scs_tclas_offset_typ0_tbl[] = {
	{ 0, offsetof(struct rtw_phl_tclas_type0, source_addr), skbr_eth_src_addr, skbr_eth_dst_addr, hsh_eth_addr, mchf_eth_addr },
	{ 1, offsetof(struct rtw_phl_tclas_type0, destination_addr), skbr_eth_dst_addr, skbr_eth_src_addr, hsh_eth_addr, mchf_eth_addr },
	{ 2, offsetof(struct rtw_phl_tclas_type0, type), skbr_eth_type, skbr_eth_type, hsh_eth_type, mchf_eth_type },
};

static u8 qos_scs_parse_tclas_typ0(u8 *s, u8 **stream, u16 len) {
	u8 *p = *stream;
	struct rtw_phl_tclas_type0 *ss = (struct rtw_phl_tclas_type0 *)s;
	ss->mask = _bR1(p++);
	memcpy(ss->source_addr, p, ETH_ALEN), p += ETH_ALEN;
	memcpy(ss->destination_addr, p, ETH_ALEN), p += ETH_ALEN;
	ss->type = _bR2((p += 2, p - 2));
	*stream = p;
	return 0;
}

static u8 qos_scs_dd_match_tclas_typ0(const u8 *s1, const u8 *s2) {
	return qos_scs_dd_match_tclas_generic_1(
		qos_scs_tclas_offset_typ0_tbl,
		sizeof(qos_scs_tclas_offset_typ0_tbl) / sizeof(struct offset_tbl_entry),
		s1,
		s2,
		offsetof(struct rtw_phl_tclas_type0, mask)
	);
}

static qos_scs_hash_t qos_scs_calc_hash_tclas_typ0(const u8 *s) {
	return qos_scs_calc_hash_tclas_generic_1(
		qos_scs_tclas_offset_typ0_tbl,
		sizeof(qos_scs_tclas_offset_typ0_tbl) / sizeof(struct offset_tbl_entry),
		s,
		offsetof(struct rtw_phl_tclas_type0, mask)
	);
}

static u8 qos_scs_dd_fill_tclas_typ0(u8 *s, const struct sk_buff *pkt, u8 mirrored) {
	return qos_scs_dd_fill_tclas_generic_1(
		qos_scs_tclas_offset_typ0_tbl,
		sizeof(qos_scs_tclas_offset_typ0_tbl) / sizeof(struct offset_tbl_entry),
		s,
		pkt,
		mirrored,
		offsetof(struct rtw_phl_tclas_type0, mask)
	);
}

static u8 qos_scs_parse_tclas_typ1(u8 *s, u8 **stream, u16 len) {
	// TODO
	return 1;
}

static u8 qos_scs_dd_match_tclas_typ1(const u8 *s1, const u8 *s2) {
	// TODO
	return !memcmp(s1, s2, sizeof(struct rtw_phl_tclas_type1));
}

static qos_scs_hash_t qos_scs_calc_hash_tclas_typ1(const u8 *s) {
	// TODO
	return 0;
}

static u8 qos_scs_dd_fill_tclas_typ1(u8 *s, const struct sk_buff *pkt, u8 mirrored) {
	// TODO
	return 0;
}

static u8 qos_scs_parse_tclas_typ2(u8 *s, u8 **stream, u16 len) {
	// TODO
	return 1;
}

static u8 qos_scs_dd_match_tclas_typ2(const u8 *s1, const u8 *s2) {
	// TODO
	return !memcmp(s1, s2, sizeof(struct rtw_phl_tclas_type2));
}

static qos_scs_hash_t qos_scs_calc_hash_tclas_typ2(const u8 *s) {
	// TODO
	return 0;
}

static u8 qos_scs_dd_fill_tclas_typ2(u8 *s, const struct sk_buff *pkt, u8 mirrored) {
	// TODO
	return 0;
}

static u8 qos_scs_parse_tclas_typ3(u8 *s, u8 **stream, u16 len) {
	// TODO
	return 1;
}

static u8 qos_scs_dd_match_tclas_typ3(const u8 *s1, const u8 *s2) {
	// TODO
	return !memcmp(s1, s2, sizeof(struct rtw_phl_tclas_type3));
}

static qos_scs_hash_t qos_scs_calc_hash_tclas_typ3(const u8 *s) {
	// TODO
	return 0;
}

static u8 qos_scs_dd_fill_tclas_typ3(u8 *s, const struct sk_buff *pkt, u8 mirrored) {
	// TODO
	return 0;
}

static struct offset_tbl_entry qos_scs_tclas_offset_typ4_tbl[] = {
	{ 0, offsetof(struct rtw_phl_tclas_type4, version), skbr_ip_ver, skbr_ip_ver, hsh_ip_ver, mchf_ip_ver },
	{ 1, offsetof(struct rtw_phl_tclas_type4, ip.v4.src_ip), skbr_ip_src_v4, skbr_ip_dst_v4, hsh_ip_addr_v4, mchf_ip_addr_v4 },
	{ 1, offsetof(struct rtw_phl_tclas_type4, ip.v6.src_ip), skbr_ip_src_v6, skbr_ip_dst_v6, hsh_ip_addr_v6, mchf_ip_addr_v6 },
	{ 2, offsetof(struct rtw_phl_tclas_type4, ip.v4.dst_ip), skbr_ip_dst_v4, skbr_ip_src_v4, hsh_ip_addr_v4, mchf_ip_addr_v4 },
	{ 2, offsetof(struct rtw_phl_tclas_type4, ip.v6.dst_ip), skbr_ip_dst_v6, skbr_ip_src_v6, hsh_ip_addr_v6, mchf_ip_addr_v6 },
	{ 3, offsetof(struct rtw_phl_tclas_type4, src_port), skbr_tsp_srcport, skbr_tsp_dstport, hsh_tsp_port, mchf_tsp_port },
	{ 4, offsetof(struct rtw_phl_tclas_type4, dst_port), skbr_tsp_dstport, skbr_tsp_srcport, hsh_tsp_port, mchf_tsp_port },
	{ 5, offsetof(struct rtw_phl_tclas_type4, dscp), skbr_ip_dscp, skbr_ip_dscp, hsh_ip_dscp, mchf_ip_dscp },
	{ 6, offsetof(struct rtw_phl_tclas_type4, prot_or_nhdr), skbr_ip_prot_or_nhdr, skbr_ip_prot_or_nhdr, hsh_ip_prot_or_nhdr, mchf_ip_prot_or_nhdr },
	{ 7, offsetof(struct rtw_phl_tclas_type4, flow_lbl), skbr_ip_flow_lbl, skbr_ip_flow_lbl, hsh_ip_flow_lbl, mchf_ip_flow_lbl },
};

static u8 qos_scs_parse_tclas_typ4(u8 *s, u8 **stream, u16 len) {
	u8 *p = *stream;
	struct rtw_phl_tclas_type4 *ss = (struct rtw_phl_tclas_type4 *)s;
	ss->mask = _bR1(p++);
	ss->version = _bR1(p++);
	if (ss->version == 6) {
		memcpy(ss->ip.v6.src_ip, p, 16), p += 16;
		memcpy(ss->ip.v6.dst_ip, p, 16), p += 16;
		ss->dst_port = _bR2((p += 2, p - 2));
		ss->src_port = _bR2((p += 2, p - 2));
		ss->dscp = _bR1(p++);
		ss->prot_or_nhdr = _bR1(p++);
		memcpy(ss->flow_lbl, p, 3), p += 3;
	} else {// ss->version == 4 OR TCLAS MASK
		memcpy(ss->ip.v4.src_ip, p, 4), p += 4;
		memcpy(ss->ip.v4.dst_ip, p, 4), p += 4;
		ss->dst_port = _bR2((p += 2, p - 2));
		ss->src_port = _bR2((p += 2, p - 2));
		ss->dscp = _bR1(p++);
		ss->prot_or_nhdr = _bR1(p++);
	}

	*stream = p;
	return 0;
}

static u8 qos_scs_dd_match_tclas_typ4(const u8 *s1, const u8 *s2) {
	return qos_scs_dd_match_tclas_generic_1(
		qos_scs_tclas_offset_typ4_tbl,
		sizeof(qos_scs_tclas_offset_typ4_tbl) / sizeof(struct offset_tbl_entry),
		s1,
		s2,
		offsetof(struct rtw_phl_tclas_type4, mask)
	);
}

static qos_scs_hash_t qos_scs_calc_hash_tclas_typ4(const u8 *s) {
	return qos_scs_calc_hash_tclas_generic_1(
		qos_scs_tclas_offset_typ4_tbl,
		sizeof(qos_scs_tclas_offset_typ4_tbl) / sizeof(struct offset_tbl_entry),
		s,
		offsetof(struct rtw_phl_tclas_type4, mask)
	);
}

static u8 qos_scs_dd_fill_tclas_typ4(u8 *s, const struct sk_buff *pkt, u8 mirrored) {
	return qos_scs_dd_fill_tclas_generic_1(
		qos_scs_tclas_offset_typ4_tbl,
		sizeof(qos_scs_tclas_offset_typ4_tbl) / sizeof(struct offset_tbl_entry),
		s,
		pkt,
		mirrored,
		offsetof(struct rtw_phl_tclas_type4, mask)
	);
}

static u8 qos_scs_parse_tclas_typ5(u8 *s, u8 **stream, u16 len) {
	// TODO
	return 1;
}

static u8 qos_scs_dd_match_tclas_typ5(const u8 *s1, const u8 *s2) {
	// TODO
	return !memcmp(s1, s2, sizeof(struct rtw_phl_tclas_type5));
}

static qos_scs_hash_t qos_scs_calc_hash_tclas_typ5(const u8 *s) {
	// TODO
	return 0;
}

static u8 qos_scs_dd_fill_tclas_typ5(u8 *s, const struct sk_buff *pkt, u8 mirrored) {
	// TODO
	return 0;
}

static u8 qos_scs_parse_tclas_typ6(u8 *s, u8 **stream, u16 len) {
	// TODO
	return 1;
}

static u8 qos_scs_dd_match_tclas_typ6(const u8 *s1, const u8 *s2) {
	// TODO
	return !memcmp(s1, s2, sizeof(struct rtw_phl_tclas_type6));
}

static qos_scs_hash_t qos_scs_calc_hash_tclas_typ6(const u8 *s) {
	// TODO
	return 0;
}

static u8 qos_scs_dd_fill_tclas_typ6(u8 *s, const struct sk_buff *pkt, u8 mirrored) {
	// TODO
	return 0;
}

static u8 qos_scs_parse_tclas_typ7(u8 *s, u8 **stream, u16 len) {
	// TODO
	return 1;
}

static u8 qos_scs_dd_match_tclas_typ7(const u8 *s1, const u8 *s2) {
	// TODO
	return !memcmp(s1, s2, sizeof(struct rtw_phl_tclas_type7));
}

static qos_scs_hash_t qos_scs_calc_hash_tclas_typ7(const u8 *s) {
	// TODO
	return 0;
}

static u8 qos_scs_dd_fill_tclas_typ7(u8 *s, const struct sk_buff *pkt, u8 mirrored) {
	// TODO
	return 0;
}

static u8 qos_scs_parse_tclas_typ8(u8 *s, u8 **stream, u16 len) {
	// TODO
	return 1;
}

static u8 qos_scs_dd_match_tclas_typ8(const u8 *s1, const u8 *s2) {
	// TODO
	return !memcmp(s1, s2, sizeof(struct rtw_phl_tclas_type8));
}

static qos_scs_hash_t qos_scs_calc_hash_tclas_typ8(const u8 *s) {
	// TODO
	return 0;
}

static u8 qos_scs_dd_fill_tclas_typ8(u8 *s, const struct sk_buff *pkt, u8 mirrored) {
	// TODO
	return 0;
}

static u8 qos_scs_parse_tclas_typ9(u8 *s, u8 **stream, u16 len) {
	// TODO
	return 1;
}

static u8 qos_scs_dd_match_tclas_typ9(const u8 *s1, const u8 *s2) {
	// TODO
	return !memcmp(s1, s2, sizeof(struct rtw_phl_tclas_type9));
}

static qos_scs_hash_t qos_scs_calc_hash_tclas_typ9(const u8 *s) {
	// TODO
	return 0;
}

static u8 qos_scs_dd_fill_tclas_typ9(u8 *s, const struct sk_buff *pkt, u8 mirrored) {
	// TODO
	return 0;
}

static u8 qos_scs_parse_tclas_typ10(u8 *s, u8 **stream, u16 len) {
	// TODO
	return 1;
}

static u8 qos_scs_dd_match_tclas_typ10(const u8 *s1, const u8 *s2) {
	// TODO
	return !memcmp(s1, s2, sizeof(struct rtw_phl_tclas_type10));
}

static qos_scs_hash_t qos_scs_calc_hash_tclas_typ10(const u8 *s) {
	// TODO
	return 0;
}

static u8 qos_scs_dd_fill_tclas_typ10(u8 *s, const struct sk_buff *pkt, u8 mirrored) {
	// TODO
	return 0;
}

static size_t qos_scs_dd_offset_tbl[_CLSFR_TYPE_MAX] = {
	offsetof(struct rtw_phl2_tclas_element_data_s, u.t0),
	offsetof(struct rtw_phl2_tclas_element_data_s, u.t1),
	offsetof(struct rtw_phl2_tclas_element_data_s, u.t2),
	offsetof(struct rtw_phl2_tclas_element_data_s, u.t3),
	offsetof(struct rtw_phl2_tclas_element_data_s, u.t4),
	offsetof(struct rtw_phl2_tclas_element_data_s, u.t5),
	offsetof(struct rtw_phl2_tclas_element_data_s, u.t6),
	offsetof(struct rtw_phl2_tclas_element_data_s, u.t7),
	offsetof(struct rtw_phl2_tclas_element_data_s, u.t8),
	offsetof(struct rtw_phl2_tclas_element_data_s, u.t9),
	offsetof(struct rtw_phl2_tclas_element_data_s, u.t10),
};

static u8 (*qos_scs_parse_tclas_tbl[_CLSFR_TYPE_MAX])(u8 *s, u8 **stream, u16 len) = {
	qos_scs_parse_tclas_typ0,
	qos_scs_parse_tclas_typ1,
	qos_scs_parse_tclas_typ2,
	qos_scs_parse_tclas_typ3,
	qos_scs_parse_tclas_typ4,
	qos_scs_parse_tclas_typ5,
	qos_scs_parse_tclas_typ6,
	qos_scs_parse_tclas_typ7,
	qos_scs_parse_tclas_typ8,
	qos_scs_parse_tclas_typ9,
	qos_scs_parse_tclas_typ10,
};

static u8 qos_scs_parse_phl2_tclas_element_data(struct rtw_phl2_tclas_element_data_s *s, u8 **stream, u16 len) {
	u8 ret = 1;
	u8 (*f)(u8 *s, u8 **stream, u16 len);
	u8 *p = *stream;
	s->type = _bR1(p++);
	if (s->type >= _CLSFR_TYPE_MAX) return ret;
	f = qos_scs_parse_tclas_tbl[s->type];
	ret = f((u8 *)s + qos_scs_dd_offset_tbl[s->type], &p, len - 1);
	*stream = p;
	return ret;
}

static u8 (*qos_scs_dd_fill_tclas_tbl[_CLSFR_TYPE_MAX])(u8 *s, const struct sk_buff *pkt, u8 mirrored) = {
	qos_scs_dd_fill_tclas_typ0,
	qos_scs_dd_fill_tclas_typ1,
	qos_scs_dd_fill_tclas_typ2,
	qos_scs_dd_fill_tclas_typ3,
	qos_scs_dd_fill_tclas_typ4,
	qos_scs_dd_fill_tclas_typ5,
	qos_scs_dd_fill_tclas_typ6,
	qos_scs_dd_fill_tclas_typ7,
	qos_scs_dd_fill_tclas_typ8,
	qos_scs_dd_fill_tclas_typ9,
	qos_scs_dd_fill_tclas_typ10,
};

static qos_scs_hash_t qos_scs_dd_fill_tclas_mask(struct rtw_phl2_tclas_element_data_s *s, const struct sk_buff *pkt, u8 mirrored) {
	if (s->type >= _CLSFR_TYPE_MAX) return 1;
	return qos_scs_dd_fill_tclas_tbl[s->type]((u8 *)s + qos_scs_dd_offset_tbl[s->type], pkt, mirrored);
}

static u8 mscs_dd_fill(struct rtw_phl2_mscs_desc_data_s *s, const struct rtw_phl2_mscs_desc_data_s *tmpl, const struct sk_buff *pkt) {
	u8 i;

	rtw_phl2_mscs_desc_data_copy(s, tmpl);
	for (i = 0; i < s->tclas_mask_elem_len; i++) {
		if (qos_scs_dd_fill_tclas_mask(&s->tclas_mask_elem[i], pkt, 0)) return 1;
	}
	return 0;
}

static u8 mscs_dd_fill_mirror(struct rtw_phl2_mscs_desc_data_s *s, const struct rtw_phl2_mscs_desc_data_s *tmpl, const struct sk_buff *pkt) {
	u8 i;

	rtw_phl2_mscs_desc_data_copy(s, tmpl);
	for (i = 0; i < s->tclas_mask_elem_len; i++) {
		if (qos_scs_dd_fill_tclas_mask(&s->tclas_mask_elem[i], pkt, 1)) return 1;
	}
	return 0;
}

static u8 scs_dd_fill(struct rtw_phl2_scs_desc_data_s *s, const struct rtw_phl2_scs_desc_data_s *tmpl, const struct sk_buff *pkt) {
	u8 i;

	rtw_phl2_scs_desc_data_copy(s, tmpl);
	for (i = 0; i < s->tclas_len; i++) {
		if (qos_scs_dd_fill_tclas_mask(&s->tclas[i], pkt, 0)) return 1;
	}
	return 0;
}

static qos_scs_hash_t (*qos_scs_calc_hash_tclas_tbl[_CLSFR_TYPE_MAX])(const u8 *s) = {
	qos_scs_calc_hash_tclas_typ0,
	qos_scs_calc_hash_tclas_typ1,
	qos_scs_calc_hash_tclas_typ2,
	qos_scs_calc_hash_tclas_typ3,
	qos_scs_calc_hash_tclas_typ4,
	qos_scs_calc_hash_tclas_typ5,
	qos_scs_calc_hash_tclas_typ6,
	qos_scs_calc_hash_tclas_typ7,
	qos_scs_calc_hash_tclas_typ8,
	qos_scs_calc_hash_tclas_typ9,
	qos_scs_calc_hash_tclas_typ10,
};

static qos_scs_hash_t qos_scs_calc_hash_tclas_mask(const struct rtw_phl2_tclas_element_data_s *s) {
	if (s->type >= _CLSFR_TYPE_MAX) return 1;
	return qos_scs_calc_hash_tclas_tbl[s->type]((u8 *)s + qos_scs_dd_offset_tbl[s->type]);
}

static qos_scs_hash_t mscs_calc_hash(const struct rtw_phl2_mscs_desc_data_s *s) {
	qos_scs_hash_t res = 0, tmp;
	u8 i;
	for (i = 0; i < s->tclas_mask_elem_len; i++) {
		tmp = qos_scs_calc_hash_tclas_mask(&s->tclas_mask_elem[i]);
		if (tmp == 0) return 0;
		res ^= tmp;
	}
	return res;
}

static u8 (*qos_scs_dd_match_tclas_tbl[_CLSFR_TYPE_MAX])(const u8 *s1, const u8 *s2) = {
	qos_scs_dd_match_tclas_typ0,
	qos_scs_dd_match_tclas_typ1,
	qos_scs_dd_match_tclas_typ2,
	qos_scs_dd_match_tclas_typ3,
	qos_scs_dd_match_tclas_typ4,
	qos_scs_dd_match_tclas_typ5,
	qos_scs_dd_match_tclas_typ6,
	qos_scs_dd_match_tclas_typ7,
	qos_scs_dd_match_tclas_typ8,
	qos_scs_dd_match_tclas_typ9,
	qos_scs_dd_match_tclas_typ10,
};

static u8 qos_scs_dd_match_tclas_mask(const struct rtw_phl2_tclas_element_data_s *s1, const struct rtw_phl2_tclas_element_data_s *s2) {
	if (s1->type != s2->type) return 0;
	if (s1->type >= _CLSFR_TYPE_MAX) return 1;
	return qos_scs_dd_match_tclas_tbl[s1->type]((u8 *)s1 + qos_scs_dd_offset_tbl[s1->type], (u8 *)s2 + qos_scs_dd_offset_tbl[s1->type]);
}

// s1 and s2 must come from the same template
static u8 mscs_dd_match(const struct rtw_phl2_mscs_desc_data_s *s1, const struct rtw_phl2_mscs_desc_data_s *s2) {
	u8 i;

	if (s1->tclas_mask_elem_len != s2->tclas_mask_elem_len) return 0;
	for (i = 0; i < s1->tclas_mask_elem_len; i++) {
		if (!qos_scs_dd_match_tclas_mask(&s1->tclas_mask_elem[i], &s2->tclas_mask_elem[i])) return 0;
	}
	return 1;
}

static u8 scs_dd_match(const struct rtw_phl2_scs_desc_data_s *s1, const struct rtw_phl2_scs_desc_data_s *s2) {
	u8 i;

	if (s1->tclas_len != s2->tclas_len) return 0;
	for (i = 0; i < s1->tclas_len; i++) {
		if (!qos_scs_dd_match_tclas_mask(&s1->tclas[i], &s2->tclas[i])) return 0;
	}
	return 1;
}

//////

u16 mscs_parse_descriptor(struct rtw_mscs_desc_s *data,
				u8 *ele_start,
				u16 ele_len) {
	u8 *p = NULL;
	u8 mscs_len = 0, tclas_mask_len, subelem_len;
	u8 tclas_mask_id, subelem_id;
	u8 i = 0;
	u16 scs_ret = SCS_REQ_SUCCESS;

	p = ele_start;
	if (_bR1(p++) != WLAN_EID_EXTENSION) return 1; /* offset start from MSCS Descriptor ELEMENT ID : 255 */
	mscs_len = _bR1(p++);
	if (_bR1(p++) != WLAN_EID_EXT_MSCS_DESCRIPTOR) return 1;	/* MSCS Descriptor ELEMENT ID EXTENSION : 88 */
	data->req_type = _bR1(p++);			/* MSCS Descriptor request type : add/change/remove */
	data->template.up_control_bmp = _bR1(p++);
	data->template.up_control_limit = _bR1(p++);
	data->template.stream_timeout = _bR4((p += 4, p - 4));

	for (data->template.tclas_mask_elem_len = 0; p - ele_start < ele_len && *p == WLAN_EID_EXTENSION;) {
		u8 *pp;
		if (i >= MAX_TCLAS_ELE) {
			scs_ret = INSUFFICIENT_TCLAS_PROCESSING_RESOURCES;
			goto fail;
		}
		p++;
		i++;
		tclas_mask_len = _bR1(p++);
		pp = p;
		if (_bR1(p++) != WLAN_EID_EXT_TCLAS_MASK) continue;
		if (qos_scs_parse_phl2_tclas_element_data(&data->template.tclas_mask_elem[data->template.tclas_mask_elem_len], &p, tclas_mask_len - 1)) {
			RTW_ERR("tclas mask parse fail\n");
			p = pp + tclas_mask_len - 1;
			continue;
		}
		data->template.tclas_mask_elem_len++;
	}

	while (p - ele_start < ele_len) { // optional subelements
		subelem_id = _bR1(p++);
		subelem_len = _bR1(p++);
		switch (subelem_id) {
			case 1: // MSCS_STATUS
			data->mscs_status = _bR2((p += 2, p - 2));
			break;
			case _VENDOR_SPECIFIC_IE_:
			// fallthrough
			default:
			p += subelem_len;
			break;
		}
	}

fail:
	return scs_ret;
}

u16 scs_parse_descriptor(struct rtw_scs_desc_s *data,
				u8 *ele_start,
				u16 ele_len) {
	u8 *p = NULL;
	u8 scs_len = 0, tclas_elem_len, subelem_len;
	u8 tclas_mask_id, subelem_id;
	u8 i = 0;
	u16 scs_ret = SCS_REQ_SUCCESS;

	p = ele_start;
	if (_bR1(p++) != WLAN_EID_SCS_DESCRIPTOR) return 1; /* offset start from MSCS Descriptor ELEMENT ID : 255 */
	scs_len = _bR1(p++);
	data->scsid = _bR1(p++);			/* MSCS Descriptor request type : add/change/remove */
	data->req_type = _bR1(p++);
	if (data->req_type == SCS_ADD || data->req_type == SCS_CHANGE) {
		if (_bR1(p++) != WLAN_EID_INTER_AC_PRIO) return 1;
		if (_bR1(p++) != 1) return 1;
		data->iacp.ac_up = BE_BITS_TO_1BYTE(p, 0, 3);
		data->iacp.alternate_queue = BE_BITS_TO_1BYTE(p, 3, 1);
		data->iacp.drop_eligibility = BE_BITS_TO_1BYTE(p, 4, 1);
		p++;
	}

	for (data->rule.tclas_len = 0, i = 0; p - ele_start < ele_len && *p == WLAN_EID_TCLAS; i++) {
		u8 *pp;
		if (data->rule.tclas_len >= MAX_TCLAS_ELE) {
			scs_ret = INSUFFICIENT_TCLAS_PROCESSING_RESOURCES;
			goto fail;
		}
		p++;
		tclas_elem_len = _bR1(p++);
		data->rule.up = _bR1(p++);
		pp = p;
		if (qos_scs_parse_phl2_tclas_element_data(&data->rule.tclas[data->rule.tclas_len], &p, tclas_elem_len - 1)) {
			p = pp + tclas_elem_len - 1;
			continue;
		}
		data->rule.tclas_len++;
	}

	if (i > 1) {
		if (_bR1(p++) != WLAN_EID_TCLAS_PROCESSING) return 1;
		if (_bR1(p++) != 1) return 1;
		data->rule.processing = _bR1(p++);
	}

	while (p - ele_start < ele_len) { // optional subelements
		subelem_id = _bR1(p++);
		subelem_len = _bR1(p++);
		switch (subelem_id) {
			case _VENDOR_SPECIFIC_IE_:
			// fallthrough
			default:
			p += subelem_len;
			break;
		}
	}

fail:
	return scs_ret;
}

void mscs_htbl_destroy_table(struct hlist_head *htbl, u8 htbl_bitl) {
	struct rtw_phl2_mscs_desc_data_s *obj = NULL;
	rtw_hlist_node *np, *n;
	unsigned int bkt;
	size_t htbl_sz = 1 << htbl_bitl;

	for (bkt =0 ; bkt < htbl_sz; bkt++) {
		rtw_hlist_for_each_entry_safe(obj, np, n, &htbl[bkt], hnode) {
			hash_del(&obj->hnode);
			rtw_phl2_mscs_desc_data_free(obj);
		}
	}
}

void mscs_htbl_init_table(struct hlist_head *htbl, u8 htbl_bitl) {
	__hash_init(htbl, 1 << htbl_bitl);
}

u8 mscs_rec_pkt(struct hlist_head *htbl, u8 htbl_bitl, const struct rtw_phl2_mscs_desc_data_s *template, const struct sk_buff *pkt1) {
	struct rtw_phl2_mscs_desc_data_s tmpl2, *ptr;
	qos_scs_hash_t h1;
	u8 existing = 0;

	if (pkt1->len < QOS_MSCS_MIN_REC_PKTLEN) return QOS_MSCS_REC_IGNORE;

	mscs_dd_fill(&tmpl2, template, pkt1);
	h1 = mscs_calc_hash(&tmpl2);
	hlist_for_each_entry(ptr, &htbl[hash_min(h1, htbl_bitl)], hnode) {
		if (mscs_dd_match(ptr, &tmpl2)) {
			existing = 1;
			break;
		}
	}
	if (existing == 0) {
		ptr = rtw_phl2_mscs_desc_data_copynew(&tmpl2);
		if (!ptr)
			return QOS_MSCS_REC_IGNORE;
		hlist_add_head(&ptr->hnode, &htbl[hash_min(h1, htbl_bitl)]);
	}
	ptr->up = pkt1->priority;
	ptr->last_updated = rtw_get_current_time();
	return existing ? QOS_MSCS_REC_UPDATE : QOS_MSCS_REC_NEW;
}

u8 mscs_retrieve_priority(struct hlist_head *htbl, u8 htbl_bitl, const struct rtw_phl2_mscs_desc_data_s *template, const struct sk_buff *pkt2) {
	struct rtw_phl2_mscs_desc_data_s tmpl2, *ptr;
	qos_scs_hash_t h1;
	mscs_dd_fill_mirror(&tmpl2, template, pkt2);
	h1 = mscs_calc_hash(&tmpl2);
	hlist_for_each_entry(ptr, &htbl[hash_min(h1, htbl_bitl)], hnode) {
		if (mscs_dd_match(ptr, &tmpl2)) {
			return ptr->up;
		}
	}
	return (u8)-1;
}

u8 scs_match_rule(const struct rtw_phl2_scs_desc_data_s *rule, const struct sk_buff *pkt2) {
	struct rtw_phl2_scs_desc_data_s s;
	scs_dd_fill(&s, rule, pkt2);
	return scs_dd_match(&s, rule);
}

#endif

unsigned char *rtw_qos_append_qos_map_ie(unsigned char *pframe, _adapter *padapter, u32 *len) {
	_adapter * primary_adapter = NULL;
	u8 buf[16];
	u8 buflen;

	buflen = sizeof(buf) / sizeof(u8);
	_rtw_memset(buf, 0, buflen);
	primary_adapter = dvobj_get_primary_adapter(padapter->dvobj);
#ifdef CONFIG_RTW_MULTI_AP_R3
	if (primary_adapter->dscp_pcp_table_enabled != 1) goto end;
	if (primary_adapter->dscp_pcp_table == NULL) goto end;

	{
		u8 dv, dv2;
		u8 up, up2;
		u8 *p;

		p = buf;
		dv2 = 0;
		up2 = (primary_adapter->dscp_pcp_table)[dv2];
		for (dv = 0; dv < 64; dv++) {
			up = (primary_adapter->dscp_pcp_table)[dv];
			if (up2 >= up) continue;
			*p = dv2;
			*(p + 1) = dv - 1;
			p += 2;
			dv2 = dv;
			up2 = up;
		}
		*p = dv2;
		*(p + 1) = dv - 1;
	}
#endif
end:
	return rtw_set_ie(pframe, EID_QoSMap, buflen, buf, len);
}


static struct xmit_frame *_rtw_qos_build_xmit_frame(_adapter *padapter, struct qos_txmgmt *ptxmgmt) {
	struct xmit_frame          *pmgntframe;
	struct pkt_attrib          *pattrib;
	u8                         *pframe;
	unsigned short             *fctrl;
	struct rtw_ieee80211_hdr	 *pwlanhdr;
	struct xmit_priv           *pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv       *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info       *pmlmeinfo = &(pmlmeext->mlmext_info);

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return NULL;

	pattrib = &pmgntframe->attrib;
	if (update_mgntframe_attrib(padapter, pattrib) != _SUCCESS) {
		rtw_free_xmitframe(&padapter->xmitpriv, pmgntframe);
		return NULL;
	}

	//pattrib->encrypt = 0;
	//pattrib->key_type = IEEE80211W_NO_KEY;

	_rtw_memcpy(pattrib->dst, ptxmgmt->raddr, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, get_bssid(&padapter->mlmepriv), ETH_ALEN);
	_rtw_memcpy(pattrib->ta, ptxmgmt->raddr, ETH_ALEN);

	_rtw_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);
	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	_rtw_memcpy(pwlanhdr->addr1, ptxmgmt->raddr, ETH_ALEN); /* RA */
	_rtw_memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN); /* TA */
	_rtw_memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	/* category, action */
	{
		u8 category, qos_action;
		category = RTW_WLAN_CATEGORY_QOS;
		qos_action = ptxmgmt->qos_action;

		switch (qos_action) {
			case qos_action_QoS_Map_Configure:
			pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
			pframe = rtw_set_fixed_ie(pframe, 1, &(qos_action), &(pattrib->pktlen));
			pframe = rtw_qos_append_qos_map_ie(pframe, padapter, &(pattrib->pktlen));
			break;
			default:
			break;
		}
	}

end:
	pattrib->last_txcmdsz = pattrib->pktlen;
	return pmgntframe;
}

u8 rtw_qos_issue_action(_adapter *padapter, struct qos_txmgmt *ptxmgmt) {
	struct xmit_frame *pmgntframe;

	pmgntframe = _rtw_qos_build_xmit_frame(padapter, ptxmgmt);
	if (pmgntframe == NULL) return 1;
	dump_mgntframe(padapter, pmgntframe);
	return 0;
}

static void _for_allsta(_adapter *padapter, void (*f)(_adapter *, struct sta_info *, void *), void *arg) {
	bool no_sta;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;
	_list *plist, *phead;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	no_sta = rtw_is_list_empty(&pstapriv->asoc_list);
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);

	if (no_sta) return;

	_rtw_spinlock_bh(&pstapriv->asoc_list_lock);
	phead = &pstapriv->asoc_list;
	plist = get_next(phead);
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE) {
		psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
		f(padapter, psta, arg);
		plist = get_next(plist);
	}
	_rtw_spinunlock_bh(&pstapriv->asoc_list_lock);
}

static void _rtw_qos_issue_action_allsta_f(_adapter *padapter, struct sta_info *psta, void *_ptxmgmt) {
	struct qos_txmgmt *ptxmgmt = (struct qos_txmgmt *)_ptxmgmt;

	switch (ptxmgmt->qos_action) {
		case qos_action_QoS_Map_Configure:
		if (psta->ext_capab_ie_len < 5 || !GET_EXT_CAPABILITY_ELE_QOS_MAP(psta->ext_capab_ie_data)) return;
		_rtw_memcpy(ptxmgmt->raddr, psta->phl_sta->mac_addr, sizeof(ptxmgmt->raddr));
		rtw_qos_issue_action(padapter, ptxmgmt);
		break;
		default:
		break;
	}
}

u8 rtw_qos_issue_action_allsta(_adapter *padapter, struct qos_txmgmt ptxmgmt) {
	_for_allsta(padapter, _rtw_qos_issue_action_allsta_f, (void *)&ptxmgmt);
	return 0;
}


static u8 wifi_alliance_oui[] = {0x50, 0x6f, 0x9a};
static u8 qos_mgmt_oui_typ = QOS_MGMT_OUI_TYPE;

static u8 * _rtw_dscp_build_policy_req(u8 *pframe, struct dscp_txmgmt *ptxmgmt, u32 *len) {
	pframe = rtw_set_fixed_ie(pframe, 1, &(ptxmgmt->dialog_token), len);
	pframe = rtw_set_fixed_ie(pframe, 1, &(ptxmgmt->request_control), len);
	pframe = rtw_set_fixed_ie(pframe, ptxmgmt->qos_mgmt_elem_len, ptxmgmt->qos_mgmt_elem, len);
	return pframe;
}

static u8 *(*pframe_dscp_ftable[])(u8 *, struct dscp_txmgmt *, u32 *) = {
	[RTW_WLAN_ACTION_VSP_QOS_DSCP_POLICY_REQ] = _rtw_dscp_build_policy_req,
	[_RTW_WLAN_ACTION_VSP_QOS_DSCP_POLICY_MAX] = NULL,
};

static struct xmit_frame *_rtw_dscp_build_xmit_frame(_adapter *padapter, struct dscp_txmgmt *ptxmgmt) {
	struct xmit_frame          *pmgntframe;
	struct pkt_attrib          *pattrib;
	u8                         *pframe;
	unsigned short             *fctrl;
	struct rtw_ieee80211_hdr	 *pwlanhdr;
	struct xmit_priv           *pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv       *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info       *pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 *(*f)(u8 *, struct dscp_txmgmt *, u32 *);

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return NULL;

	pattrib = &pmgntframe->attrib;
	if (update_mgntframe_attrib(padapter, pattrib) != _SUCCESS) {
		rtw_free_xmitframe(&padapter->xmitpriv, pmgntframe);
		return NULL;
	}

	//pattrib->encrypt = 0;
	//pattrib->key_type = IEEE80211W_NO_KEY;

	_rtw_memcpy(pattrib->dst, ptxmgmt->raddr, ETH_ALEN);
	_rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pattrib->ra, get_bssid(&padapter->mlmepriv), ETH_ALEN);
	_rtw_memcpy(pattrib->ta, ptxmgmt->raddr, ETH_ALEN);

	_rtw_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);
	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	_rtw_memcpy(pwlanhdr->addr1, ptxmgmt->raddr, ETH_ALEN); /* RA */
	_rtw_memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN); /* TA */
	_rtw_memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	/* category, action */
	{
		u8 category;
		category = RTW_WLAN_CATEGORY_VENDOR_SPECIFIC_PROTECTED;

		pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
		pframe = rtw_set_fixed_ie(pframe, 3, wifi_alliance_oui, &(pattrib->pktlen));
		pframe = rtw_set_fixed_ie(pframe, 1, &qos_mgmt_oui_typ, &(pattrib->pktlen));
		pframe = rtw_set_fixed_ie(pframe, 1, &ptxmgmt->oui_subtyp, &(pattrib->pktlen));
	}

	f = pframe_dscp_ftable[ptxmgmt->oui_subtyp];
	if (f == NULL) {
		RTW_WARN("%s: unknown oui subtype %d\n", __FUNCTION__, ptxmgmt->oui_subtyp);
		goto end;
	}
	pframe = f(pframe, ptxmgmt, &(pattrib->pktlen));

end:
	pattrib->last_txcmdsz = pattrib->pktlen;
	return pmgntframe;
}

u8 rtw_dscp_issue_action(_adapter *padapter, struct dscp_txmgmt *ptxmgmt) {
	struct xmit_frame *pmgntframe;

	pmgntframe = _rtw_dscp_build_xmit_frame(padapter, ptxmgmt);
	if (pmgntframe == NULL) return 1;
	dump_mgntframe(padapter, pmgntframe);
	return 0;
}

u8 rtw_core_vsp_qos_on_action(_adapter *padapter,  u8* frame_body, u32 frame_body_len, struct sta_info *psta) {
	u8 oui_subtyp, dialog_token;
	struct dscp_txmgmt ptxmgmt;

	if (frame_body_len < 7) return _FAIL;
	oui_subtyp = frame_body[5];
	dialog_token = frame_body[6];

	ptxmgmt = (struct dscp_txmgmt){
		.dialog_token = dialog_token,
		0
	};
	_rtw_memcpy(ptxmgmt.raddr, psta->phl_sta->mac_addr, sizeof(ptxmgmt.raddr));

#ifdef CONFIG_RTW_MULTI_AP_R2
	switch(oui_subtyp) {
		case RTW_WLAN_ACTION_VSP_QOS_DSCP_POLICY_QUERY:
			core_map_send_tunneled_message(padapter, psta->phl_sta->mac_addr, TUNNELED_MSG_DSCP_POLICY_QUERY, frame_body_len, frame_body);
			ptxmgmt.oui_subtyp = RTW_WLAN_ACTION_VSP_QOS_DSCP_POLICY_REQ;
			if (rtw_dscp_issue_action(padapter, &ptxmgmt)) return _FAIL;
			break;
		case RTW_WLAN_ACTION_VSP_QOS_DSCP_POLICY_REQ:
			break;
		case RTW_WLAN_ACTION_VSP_QOS_DSCP_POLICY_RESP:
			core_map_send_tunneled_message(padapter, psta->phl_sta->mac_addr, TUNNELED_MSG_DSCP_POLICY_REPSONSE, frame_body_len, frame_body);
			break;
		default:
			break;
	}
#endif

	return _SUCCESS;
}

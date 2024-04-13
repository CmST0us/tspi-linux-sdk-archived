/*
 * (C) Copyright 2008-2017 Fuzhou Rockchip Electronics Co., Ltd
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ROCKCHIP_VOP_H_
#define _ROCKCHIP_VOP_H_
#include "rockchip_display.h"
#include <asm/gpio.h>


#define VOP_REG_SUPPORT(vop, reg) \
		(reg.mask && \
		 (!reg.major || \
		  (reg.major == VOP_MAJOR(vop->version) && \
		   reg.begin_minor <= VOP_MINOR(vop->version) && \
		   reg.end_minor >= VOP_MINOR(vop->version))))

#define VOP_WIN_SUPPORT(vop, win, name) \
		VOP_REG_SUPPORT(vop, win->name)

#define VOP_CTRL_SUPPORT(vop, name) \
		VOP_REG_SUPPORT(vop, vop->ctrl->name)

#define __REG_SET(x, off, mask, shift, v, write_mask) \
		vop_mask_write(x, off, mask, shift, v, write_mask)

#define _REG_SET(vop, name, off, reg, mask, v) \
	do { \
		if (VOP_REG_SUPPORT(vop, reg)) \
			__REG_SET(vop, off + reg.offset, mask, reg.shift, \
				  v, reg.write_mask); \
		else \
			debug("Warning: not support "#name"\n"); \
	} while(0)

#define REG_SET(x, name, off, reg, v) \
		_REG_SET(x, name, off, reg, reg.mask, v)
#define REG_SET_MASK(x, name, off, reg, mask, v, relaxed) \
		_REG_SET(x, name, off, reg, reg.mask & mask, v)

#define VOP_WIN_SET(x, name, v) \
		REG_SET(x, name, x->win_offset, x->win->name, v)
#define VOP_WIN_SET_EXT(x, ext, name, v) \
		REG_SET(x, name, x->win_offset, x->win->ext->name, v)
#define VOP_SCL_SET(x, name, v) \
		REG_SET(x, name, x->win_offset, x->win->scl->name, v)
#define VOP_SCL_SET_EXT(x, name, v) \
		REG_SET(x, name, x->win_offset, x->win->scl->ext->name, v)

#define VOP_CTRL_SET(x, name, v) \
		REG_SET(x, name, 0, (x)->ctrl->name, v)
#define VOP_LINE_FLAG_SET(x, name, v) \
		REG_SET(x, name, 0, (x)->line_flag->name, v)
#define VOP_WIN_CSC_SET(x, name, v) \
		REG_SET(x, name, 0, (x)->win_csc->name, v)

#define VOP_CTRL_GET(x, name) \
		vop_read_reg(x, 0, &vop->ctrl->name)

#define VOP_WIN_GET(x, name) \
		vop_read_reg(x, vop->win->offset, &vop->win->name)

#define VOP_GRF_SET(vop, name, v) \
	do { \
		if (vop->grf_ctrl) { \
			vop_grf_writel(vop, vop->grf_ctrl->name, v); \
		} \
	} while (0)

#define CVBS_PAL_VDISPLAY              288

enum alpha_mode {
	ALPHA_STRAIGHT,
	ALPHA_INVERSE,
};

enum global_blend_mode {
	ALPHA_GLOBAL,
	ALPHA_PER_PIX,
	ALPHA_PER_PIX_GLOBAL,
};

enum alpha_cal_mode {
	ALPHA_SATURATION,
	ALPHA_NO_SATURATION,
};

enum color_mode {
	ALPHA_SRC_PRE_MUL,
	ALPHA_SRC_NO_PRE_MUL,
};

enum factor_mode {
	ALPHA_ZERO,
	ALPHA_ONE,
	ALPHA_SRC,
	ALPHA_SRC_INVERSE,
	ALPHA_SRC_GLOBAL,
};

enum scale_mode {
	SCALE_NONE = 0x0,
	SCALE_UP   = 0x1,
	SCALE_DOWN = 0x2
};

enum lb_mode {
	LB_YUV_3840X5 = 0x0,
	LB_YUV_2560X8 = 0x1,
	LB_RGB_3840X2 = 0x2,
	LB_RGB_2560X4 = 0x3,
	LB_RGB_1920X5 = 0x4,
	LB_RGB_1280X8 = 0x5
};

enum sacle_up_mode {
	SCALE_UP_BIL = 0x0,
	SCALE_UP_BIC = 0x1
};

enum scale_down_mode {
	SCALE_DOWN_BIL = 0x0,
	SCALE_DOWN_AVG = 0x1
};

enum dither_down_mode {
	RGB888_TO_RGB565 = 0x0,
	RGB888_TO_RGB666 = 0x1
};

enum dither_down_mode_sel {
	DITHER_DOWN_ALLEGRO = 0x0,
	DITHER_DOWN_FRC = 0x1
};

enum vop_csc_format {
	CSC_BT601L,
	CSC_BT709L,
	CSC_BT601F,
	CSC_BT2020,
};

#define DSP_BG_SWAP		0x1
#define DSP_RB_SWAP		0x2
#define DSP_RG_SWAP		0x4
#define DSP_DELTA_SWAP		0x8

#define PRE_DITHER_DOWN_EN(x)	((x) << 0)
#define DITHER_DOWN_EN(x)	((x) << 1)
#define DITHER_DOWN_MODE(x)	((x) << 2)
#define DITHER_DOWN_MODE_SEL(x)	((x) << 3)

#define FRAC_16_16(mult, div)    (((mult) << 16) / (div))
#define SCL_FT_DEFAULT_FIXPOINT_SHIFT	12
#define SCL_MAX_VSKIPLINES		4
#define MIN_SCL_FT_AFTER_VSKIP		1

#define VOP_PLANE_NO_SCALING	BIT(16)

static inline uint16_t scl_cal_scale(int src, int dst, int shift)
{
	return ((src * 2 - 3) << (shift - 1)) / (dst - 1);
}

static inline uint16_t scl_cal_scale2(int src, int dst)
{
	return ((src - 1) << 12) / (dst - 1);
}

#define GET_SCL_FT_BILI_DN(src, dst)	scl_cal_scale(src, dst, 12)
#define GET_SCL_FT_BILI_UP(src, dst)	scl_cal_scale(src, dst, 16)
#define GET_SCL_FT_BIC(src, dst)	scl_cal_scale(src, dst, 16)

static inline uint16_t scl_get_bili_dn_vskip(int src_h, int dst_h,
					     int vskiplines)
{
	int act_height;

	act_height = (src_h + vskiplines - 1) / vskiplines;

	return GET_SCL_FT_BILI_DN(act_height, dst_h);
}

static inline enum scale_mode scl_get_scl_mode(int src, int dst)
{
	if (src < dst)
		return SCALE_UP;
	else if (src > dst)
		return SCALE_DOWN;

	return SCALE_NONE;
}

static inline int scl_get_vskiplines(uint32_t srch, uint32_t dsth)
{
	uint32_t vskiplines;

	for (vskiplines = SCL_MAX_VSKIPLINES; vskiplines > 1; vskiplines /= 2)
		if (srch >= vskiplines * dsth * MIN_SCL_FT_AFTER_VSKIP)
			break;

	return vskiplines;
}

static inline int scl_vop_cal_lb_mode(int width, bool is_yuv)
{
	int lb_mode;

	if (width > 2560)
		lb_mode = LB_RGB_3840X2;
	else if (width > 1920)
		lb_mode = LB_RGB_2560X4;
	else if (!is_yuv)
		lb_mode = LB_RGB_1920X5;
	else if (width > 1280)
		lb_mode = LB_YUV_3840X5;
	else
		lb_mode = LB_YUV_2560X8;

	return lb_mode;
}

struct vop_reg_data {
	uint32_t offset;
	uint32_t value;
};

struct vop_reg {
	uint32_t mask;
	uint32_t offset:17;
	uint32_t shift:5;
	uint32_t begin_minor:4;
	uint32_t end_minor:4;
	uint32_t reserved:2;
	uint32_t major:3;
	uint32_t write_mask:1;
};

struct vop_ctrl {
	struct vop_reg standby;
	struct vop_reg axi_outstanding_max_num;
	struct vop_reg axi_max_outstanding_en;
	struct vop_reg htotal_pw;
	struct vop_reg hact_st_end;
	struct vop_reg vtotal_pw;
	struct vop_reg vact_st_end;
	struct vop_reg vact_st_end_f1;
	struct vop_reg vs_st_end_f1;
	struct vop_reg hpost_st_end;
	struct vop_reg vpost_st_end;
	struct vop_reg vpost_st_end_f1;
	struct vop_reg post_scl_factor;
	struct vop_reg post_scl_ctrl;
	struct vop_reg dsp_interlace;
	struct vop_reg global_regdone_en;
	struct vop_reg auto_gate_en;
	struct vop_reg post_lb_mode;
	struct vop_reg dsp_layer_sel;
	struct vop_reg overlay_mode;
	struct vop_reg core_dclk_div;
	struct vop_reg dclk_ddr;
	struct vop_reg p2i_en;
	struct vop_reg hdmi_dclk_out_en;
	struct vop_reg rgb_en;
	struct vop_reg lvds_en;
	struct vop_reg edp_en;
	struct vop_reg hdmi_en;
	struct vop_reg mipi_en;
	struct vop_reg data01_swap;
	struct vop_reg mipi_dual_channel_en;
	struct vop_reg dp_en;
	struct vop_reg dclk_pol;
	struct vop_reg pin_pol;
	struct vop_reg rgb_dclk_pol;
	struct vop_reg rgb_pin_pol;
	struct vop_reg lvds_dclk_pol;
	struct vop_reg lvds_pin_pol;
	struct vop_reg hdmi_dclk_pol;
	struct vop_reg hdmi_pin_pol;
	struct vop_reg edp_dclk_pol;
	struct vop_reg edp_pin_pol;
	struct vop_reg mipi_dclk_pol;
	struct vop_reg mipi_pin_pol;
	struct vop_reg dp_dclk_pol;
	struct vop_reg dp_pin_pol;

	struct vop_reg dither_up;
	struct vop_reg dither_down;

	struct vop_reg sw_dac_sel;
	struct vop_reg tve_sw_mode;
	struct vop_reg tve_dclk_pol;
	struct vop_reg tve_dclk_en;
	struct vop_reg sw_genlock;
	struct vop_reg sw_uv_offset_en;

	struct vop_reg dsp_out_yuv;
	struct vop_reg dsp_data_swap;
	struct vop_reg dsp_bg_swap;
	struct vop_reg dsp_rb_swap;
	struct vop_reg dsp_rg_swap;
	struct vop_reg dsp_delta_swap;
	struct vop_reg dsp_dummy_swap;
	struct vop_reg dsp_ccir656_avg;
	struct vop_reg dsp_black;
	struct vop_reg dsp_blank;
	struct vop_reg dsp_outzero;
	struct vop_reg dsp_lut_en;
	struct vop_reg update_gamma_lut;

	struct vop_reg out_mode;

	struct vop_reg xmirror;
	struct vop_reg ymirror;
	struct vop_reg dsp_background;

	/* CABC */
	struct vop_reg cabc_total_num;
	struct vop_reg cabc_config_mode;
	struct vop_reg cabc_stage_up_mode;
	struct vop_reg cabc_scale_cfg_value;
	struct vop_reg cabc_scale_cfg_enable;
	struct vop_reg cabc_global_dn_limit_en;
	struct vop_reg cabc_lut_en;
	struct vop_reg cabc_en;
	struct vop_reg cabc_handle_en;
	struct vop_reg cabc_stage_up;
	struct vop_reg cabc_stage_down;
	struct vop_reg cabc_global_dn;
	struct vop_reg cabc_calc_pixel_num;

	struct vop_reg win_gate[4];
	struct vop_reg win_channel[4];

	/* BCSH */
	struct vop_reg bcsh_brightness;
	struct vop_reg bcsh_contrast;
	struct vop_reg bcsh_sat_con;
	struct vop_reg bcsh_sin_hue;
	struct vop_reg bcsh_cos_hue;
	struct vop_reg bcsh_r2y_csc_mode;
	struct vop_reg bcsh_r2y_en;
	struct vop_reg bcsh_y2r_csc_mode;
	struct vop_reg bcsh_y2r_en;
	struct vop_reg bcsh_color_bar;
	struct vop_reg bcsh_out_mode;
	struct vop_reg bcsh_en;
	struct vop_reg reg_done_frm;

	/* MCU OUTPUT */
	struct vop_reg mcu_pix_total;
	struct vop_reg mcu_cs_pst;
	struct vop_reg mcu_cs_pend;
	struct vop_reg mcu_rw_pst;
	struct vop_reg mcu_rw_pend;
	struct vop_reg mcu_clk_sel;
	struct vop_reg mcu_hold_mode;
	struct vop_reg mcu_frame_st;
	struct vop_reg mcu_rs;
	struct vop_reg mcu_bypass;
	struct vop_reg mcu_type;
	struct vop_reg mcu_rw_bypass_port;

	/* bt1120 */
	struct vop_reg bt1120_yc_swap;
	struct vop_reg bt1120_en;

	/* bt656 */
	struct vop_reg bt656_en;

	struct vop_reg cfg_done;
};

struct vop_scl_extension {
	struct vop_reg cbcr_vsd_mode;
	struct vop_reg cbcr_vsu_mode;
	struct vop_reg cbcr_hsd_mode;
	struct vop_reg cbcr_ver_scl_mode;
	struct vop_reg cbcr_hor_scl_mode;
	struct vop_reg yrgb_vsd_mode;
	struct vop_reg yrgb_vsu_mode;
	struct vop_reg yrgb_hsd_mode;
	struct vop_reg yrgb_ver_scl_mode;
	struct vop_reg yrgb_hor_scl_mode;
	struct vop_reg line_load_mode;
	struct vop_reg cbcr_axi_gather_num;
	struct vop_reg yrgb_axi_gather_num;
	struct vop_reg vsd_cbcr_gt2;
	struct vop_reg vsd_cbcr_gt4;
	struct vop_reg vsd_yrgb_gt2;
	struct vop_reg vsd_yrgb_gt4;
	struct vop_reg bic_coe_sel;
	struct vop_reg cbcr_axi_gather_en;
	struct vop_reg yrgb_axi_gather_en;
	struct vop_reg lb_mode;
};

struct vop_scl_regs {
	const struct vop_scl_extension *ext;

	struct vop_reg scale_yrgb_x;
	struct vop_reg scale_yrgb_y;
	struct vop_reg scale_cbcr_x;
	struct vop_reg scale_cbcr_y;
};

struct vop_win {
	const struct vop_scl_regs *scl;

	struct vop_reg gate;
	struct vop_reg enable;
	struct vop_reg format;
	struct vop_reg interlace_read;
	struct vop_reg ymirror;
	struct vop_reg rb_swap;
	struct vop_reg act_info;
	struct vop_reg dsp_info;
	struct vop_reg dsp_st;
	struct vop_reg yrgb_mst;
	struct vop_reg uv_mst;
	struct vop_reg yrgb_vir;
	struct vop_reg uv_vir;
	struct vop_reg alpha_mode;
	struct vop_reg alpha_en;

	struct vop_reg dst_alpha_ctl;
	struct vop_reg src_alpha_ctl;
};

struct vop_line_flag {
	struct vop_reg line_flag_num[2];
};

struct vop_grf_ctrl {
	struct vop_reg grf_dclk_inv;
};

struct vop_csc_table {
	const uint32_t *r2y_bt601;
	const uint32_t *r2y_bt601_12_235;
	const uint32_t *r2y_bt709;
	const uint32_t *r2y_bt2020;
};

struct vop_csc {
	struct vop_reg y2r_en;
	struct vop_reg r2r_en;
	struct vop_reg r2y_en;

	uint32_t y2r_offset;
	uint32_t r2r_offset;
	uint32_t r2y_offset;
};

#define VOP_FEATURE_OUTPUT_10BIT	BIT(0)

struct vop_data {
	uint32_t version;
	const struct vop_ctrl *ctrl;
	const struct vop_win *win;
	const struct vop_line_flag *line_flag;
	const struct vop_grf_ctrl *grf_ctrl;
	const struct vop_csc_table *csc_table;
	const struct vop_csc *win_csc;
	int win_offset;
	int reg_len;
	u64 feature;
	struct vop_rect max_output;
};

struct vop {
	u32 *regsbak;
	void *regs;
	void *grf;

	uint32_t version;
	const struct vop_ctrl *ctrl;
	const struct vop_win *win;
	const struct vop_line_flag *line_flag;
	const struct vop_grf_ctrl *grf_ctrl;
	const struct vop_csc_table *csc_table;
	const struct vop_csc *win_csc;
	int win_offset;

	struct gpio_desc mcu_rs_gpio;
};

static inline void vop_writel(struct vop *vop, uint32_t offset, uint32_t v)
{
	writel(v, vop->regs + offset);
	vop->regsbak[offset >> 2] = v;
}

static inline uint32_t vop_readl(struct vop *vop, uint32_t offset)
{
	return readl(vop->regs + offset);
}

static inline uint32_t vop_read_reg(struct vop *vop, uint32_t base,
				    const struct vop_reg *reg)
{
	return (vop_readl(vop, base + reg->offset) >> reg->shift) & reg->mask;
}

static inline void vop_mask_write(struct vop *vop, uint32_t offset,
				  uint32_t mask, uint32_t shift, uint32_t v,
				  bool write_mask)
{
	if (!mask)
		return;

	if (write_mask) {
		v = ((v & mask) << shift) | (mask << (shift + 16));
	} else {
		uint32_t cached_val = vop->regsbak[offset >> 2];

		v = (cached_val & ~(mask << shift)) | ((v & mask) << shift);
		vop->regsbak[offset >> 2] = v;
	}

	writel(v, vop->regs + offset);
}

static inline void vop_cfg_done(struct vop *vop)
{
	VOP_CTRL_SET(vop, cfg_done, 1);
}

static inline void vop_grf_writel(struct vop *vop, struct vop_reg reg, u32 v)
{
	u32 val = 0;

	if (VOP_REG_SUPPORT(vop, reg)) {
		val = (v << reg.shift) | (reg.mask << (reg.shift + 16));
		writel(val, vop->grf + reg.offset);
	}
}

/**
 * drm_format_horz_chroma_subsampling - get the horizontal chroma subsampling factor
 * @format: pixel format (DRM_FORMAT_*)
 *
 * Returns:
 * The horizontal chroma subsampling factor for the
 * specified pixel format.
 */
static inline int drm_format_horz_chroma_subsampling(uint32_t format)
{
	/* uboot only support RGB format */
	return 1;
}

/**
 * drm_format_vert_chroma_subsampling - get the vertical chroma subsampling factor
 * @format: pixel format (DRM_FORMAT_*)
 *
 * Returns:
 * The vertical chroma subsampling factor for the
 * specified pixel format.
 */
static inline int drm_format_vert_chroma_subsampling(uint32_t format)
{
	/* uboot only support RGB format */
	return 1;
}

#endif

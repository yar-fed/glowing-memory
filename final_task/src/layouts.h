#include "jp_hi_ka_ranges.h"
#include "icons.h"
#define SOUND_MARKS_ICON KATAKANA_HIRAGANA_VOICED_SOUND_MARK
#define EMPTY_KEY_ICON 0
#define OPENBOX_ICON IDEOGRAPHIC_SPACE


struct {
	char *keys[16];
	short codes[16];
} kb_layouts[23] = {
	{	// default
		HIRAGANA_LETTER_A_xbm14,
		HIRAGANA_LETTER_KA_xbm14,
		HIRAGANA_LETTER_SA_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		HIRAGANA_LETTER_TA_xbm14,
		HIRAGANA_LETTER_NA_xbm14,
		HIRAGANA_LETTER_HA_xbm14,
		SOUND_MARKS_ICON_xbm14,
		HIRAGANA_LETTER_MA_xbm14,
		HIRAGANA_LETTER_YA_xbm14,
		HIRAGANA_LETTER_RA_xbm14,
		KATAKANA_LETTER_KA_xbm14,
		LEFT_ANGLE_BRACKET_xbm14,
		HIRAGANA_LETTER_WA_xbm14,
		RIGHT_ANGLE_BRACKET_xbm14,
		IDEOGRAPHIC_FULL_STOP_xbm14,
		HIRAGANA_LETTER_A,
		HIRAGANA_LETTER_KA,
		HIRAGANA_LETTER_SA,
		LEFT_DOUBLE_ANGLE_BRACKET,
		HIRAGANA_LETTER_TA,
		HIRAGANA_LETTER_NA,
		HIRAGANA_LETTER_HA,
		SOUND_MARKS_ICON,
		HIRAGANA_LETTER_MA,
		HIRAGANA_LETTER_YA,
		HIRAGANA_LETTER_RA,
		KATAKANA_LETTER_KA,
		LEFT_ANGLE_BRACKET,
		HIRAGANA_LETTER_WA,
		RIGHT_ANGLE_BRACKET,
		IDEOGRAPHIC_FULL_STOP,
	},
	{	//  A column
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_U_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		HIRAGANA_LETTER_I_xbm14,
		HIRAGANA_LETTER_A_xbm14,
		HIRAGANA_LETTER_E_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_O_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_U,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		HIRAGANA_LETTER_I,
		HIRAGANA_LETTER_A,
		HIRAGANA_LETTER_E,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_O,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// KA column
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_KU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		HIRAGANA_LETTER_KI_xbm14,
		HIRAGANA_LETTER_KA_xbm14,
		HIRAGANA_LETTER_KE_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_KO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_KU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		HIRAGANA_LETTER_KI,
		HIRAGANA_LETTER_KA,
		HIRAGANA_LETTER_KE,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_KO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// SA column
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_SU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		HIRAGANA_LETTER_SI_xbm14,
		HIRAGANA_LETTER_SA_xbm14,
		HIRAGANA_LETTER_SE_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_SO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_SU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		HIRAGANA_LETTER_SI,
		HIRAGANA_LETTER_SA,
		HIRAGANA_LETTER_SE,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_SO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// TA column
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_TU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		HIRAGANA_LETTER_TI_xbm14,
		HIRAGANA_LETTER_TA_xbm14,
		HIRAGANA_LETTER_TE_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_TO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_TU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		HIRAGANA_LETTER_TI,
		HIRAGANA_LETTER_TA,
		HIRAGANA_LETTER_TE,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_TO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// NA column
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_NU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		HIRAGANA_LETTER_NI_xbm14,
		HIRAGANA_LETTER_NA_xbm14,
		HIRAGANA_LETTER_NE_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_NO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_NU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		HIRAGANA_LETTER_NI,
		HIRAGANA_LETTER_NA,
		HIRAGANA_LETTER_NE,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_NO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// HA column
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_HU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		HIRAGANA_LETTER_HI_xbm14,
		HIRAGANA_LETTER_HA_xbm14,
		HIRAGANA_LETTER_HE_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_HO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_HU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		HIRAGANA_LETTER_HI,
		HIRAGANA_LETTER_HA,
		HIRAGANA_LETTER_HE,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_HO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// MA column
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_MU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		HIRAGANA_LETTER_MI_xbm14,
		HIRAGANA_LETTER_MA_xbm14,
		HIRAGANA_LETTER_ME_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_MO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_MU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		HIRAGANA_LETTER_MI,
		HIRAGANA_LETTER_MA,
		HIRAGANA_LETTER_ME,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_MO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// YA column
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_YU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		LEFT_BLACK_LENTICULAR_BRACKET_xbm14,
		HIRAGANA_LETTER_YA_xbm14,
		RIGHT_BLACK_LENTICULAR_BRACKET_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_YO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_YU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		LEFT_BLACK_LENTICULAR_BRACKET,
		HIRAGANA_LETTER_YA,
		RIGHT_BLACK_LENTICULAR_BRACKET,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_YO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// RA column
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_RU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		HIRAGANA_LETTER_RI_xbm14,
		HIRAGANA_LETTER_RA_xbm14,
		HIRAGANA_LETTER_RE_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_RO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_RU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		HIRAGANA_LETTER_RI,
		HIRAGANA_LETTER_RA,
		HIRAGANA_LETTER_RE,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_RO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// WA column
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_N_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		KATAKANA_HIRAGANA_PROLONGED_SOUND_MARK_xbm14,
		HIRAGANA_LETTER_WA_xbm14,
		WAVY_DASH_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		HIRAGANA_LETTER_WO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_N,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		KATAKANA_HIRAGANA_PROLONGED_SOUND_MARK,
		HIRAGANA_LETTER_WA,
		WAVY_DASH,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		HIRAGANA_LETTER_WO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// default
		KATAKANA_LETTER_A_xbm14,
		KATAKANA_LETTER_KA_xbm14,
		KATAKANA_LETTER_SA_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		KATAKANA_LETTER_TA_xbm14,
		KATAKANA_LETTER_NA_xbm14,
		KATAKANA_LETTER_HA_xbm14,
		SOUND_MARKS_ICON_xbm14,
		KATAKANA_LETTER_MA_xbm14,
		KATAKANA_LETTER_YA_xbm14,
		KATAKANA_LETTER_RA_xbm14,
		HIRAGANA_LETTER_KA_xbm14,
		LEFT_ANGLE_BRACKET_xbm14,
		KATAKANA_LETTER_WA_xbm14,
		RIGHT_ANGLE_BRACKET_xbm14,
		IDEOGRAPHIC_FULL_STOP_xbm14,
		KATAKANA_LETTER_A,
		KATAKANA_LETTER_KA,
		KATAKANA_LETTER_SA,
		LEFT_DOUBLE_ANGLE_BRACKET,
		KATAKANA_LETTER_TA,
		KATAKANA_LETTER_NA,
		KATAKANA_LETTER_HA,
		SOUND_MARKS_ICON,
		KATAKANA_LETTER_MA,
		KATAKANA_LETTER_YA,
		KATAKANA_LETTER_RA,
		HIRAGANA_LETTER_KA,
		LEFT_ANGLE_BRACKET,
		KATAKANA_LETTER_WA,
		RIGHT_ANGLE_BRACKET,
		IDEOGRAPHIC_FULL_STOP,
	},
	{	//  A column
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_U_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		KATAKANA_LETTER_I_xbm14,
		KATAKANA_LETTER_A_xbm14,
		KATAKANA_LETTER_E_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_O_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_U,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		KATAKANA_LETTER_I,
		KATAKANA_LETTER_A,
		KATAKANA_LETTER_E,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_O,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// KA column
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_KU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		KATAKANA_LETTER_KI_xbm14,
		KATAKANA_LETTER_KA_xbm14,
		KATAKANA_LETTER_KE_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_KO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_KU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		KATAKANA_LETTER_KI,
		KATAKANA_LETTER_KA,
		KATAKANA_LETTER_KE,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_KO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// SA column
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_SU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		KATAKANA_LETTER_SI_xbm14,
		KATAKANA_LETTER_SA_xbm14,
		KATAKANA_LETTER_SE_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_SO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_SU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		KATAKANA_LETTER_SI,
		KATAKANA_LETTER_SA,
		KATAKANA_LETTER_SE,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_SO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// TA column
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_TU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		KATAKANA_LETTER_TI_xbm14,
		KATAKANA_LETTER_TA_xbm14,
		KATAKANA_LETTER_TE_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_TO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_TU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		KATAKANA_LETTER_TI,
		KATAKANA_LETTER_TA,
		KATAKANA_LETTER_TE,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_TO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// NA column
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_NU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		KATAKANA_LETTER_NI_xbm14,
		KATAKANA_LETTER_NA_xbm14,
		KATAKANA_LETTER_NE_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_NO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_NU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		KATAKANA_LETTER_NI,
		KATAKANA_LETTER_NA,
		KATAKANA_LETTER_NE,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_NO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// HA column
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_HU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		KATAKANA_LETTER_HI_xbm14,
		KATAKANA_LETTER_HA_xbm14,
		KATAKANA_LETTER_HE_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_HO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_HU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		KATAKANA_LETTER_HI,
		KATAKANA_LETTER_HA,
		KATAKANA_LETTER_HE,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_HO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// MA column
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_MU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		KATAKANA_LETTER_MI_xbm14,
		KATAKANA_LETTER_MA_xbm14,
		KATAKANA_LETTER_ME_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_MO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_MU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		KATAKANA_LETTER_MI,
		KATAKANA_LETTER_MA,
		KATAKANA_LETTER_ME,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_MO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// YA column
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_YU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		LEFT_BLACK_LENTICULAR_BRACKET_xbm14,
		KATAKANA_LETTER_YA_xbm14,
		RIGHT_BLACK_LENTICULAR_BRACKET_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_YO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_YU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		LEFT_BLACK_LENTICULAR_BRACKET,
		KATAKANA_LETTER_YA,
		RIGHT_BLACK_LENTICULAR_BRACKET,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_YO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// RA column
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_RU_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		KATAKANA_LETTER_RI_xbm14,
		KATAKANA_LETTER_RA_xbm14,
		KATAKANA_LETTER_RE_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_RO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_RU,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		KATAKANA_LETTER_RI,
		KATAKANA_LETTER_RA,
		KATAKANA_LETTER_RE,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_RO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// WA column
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_N_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		KATAKANA_HIRAGANA_PROLONGED_SOUND_MARK_xbm14,
		KATAKANA_LETTER_WA_xbm14,
		WAVY_DASH_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		KATAKANA_LETTER_WO_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_N,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		KATAKANA_HIRAGANA_PROLONGED_SOUND_MARK,
		KATAKANA_LETTER_WA,
		WAVY_DASH,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		KATAKANA_LETTER_WO,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
	{	// Punctuation
		EMPTY_KEY_ICON_xbm14,
		OPENBOX_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		LEFT_DOUBLE_ANGLE_BRACKET_xbm14,
		LEFT_BLACK_LENTICULAR_BRACKET_xbm14,
		IDEOGRAPHIC_FULL_STOP_xbm14,
		RIGHT_BLACK_LENTICULAR_BRACKET_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		IDEOGRAPHIC_COMMA_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON_xbm14,
		EMPTY_KEY_ICON,
		OPENBOX_ICON,
		EMPTY_KEY_ICON,
		LEFT_DOUBLE_ANGLE_BRACKET,
		LEFT_BLACK_LENTICULAR_BRACKET,
		IDEOGRAPHIC_FULL_STOP,
		RIGHT_BLACK_LENTICULAR_BRACKET,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		IDEOGRAPHIC_COMMA,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
		EMPTY_KEY_ICON,
	},
};

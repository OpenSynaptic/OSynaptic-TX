/*
 * ostx_units.h -- OpenSynaptic full unit library (compile-time validation).
 *
 * This header provides every valid OpenSynaptic unit as a preprocessor macro.
 * No code is compiled; all definitions are pure string literals.
 *
 * USAGE
 * -----
 * Pass units to any pack function using the OSTX_UNIT() convenience macro
 * or the OSTX_UNIT_<sym> constant directly:
 *
 *   ostx_stream_pack(&ctx, "T1", OSTX_UNIT(Cel),   value, buf, len);
 *   ostx_stream_pack(&ctx, "H1", OSTX_UNIT(pct),   value, buf, len);
 *   ostx_stream_pack(&ctx, "P1", OSTX_UNIT(Pa),    value, buf, len);
 *   ostx_stream_pack(&ctx, "M1", OSTX_UNIT(deg_s), value, buf, len);
 *
 * COMPILE-TIME VALIDATION
 * -----------------------
 * OSTX_UNIT(sym) expands to OSTX_UNIT_##sym.
 * If <sym> is not a defined unit token, the macro is undefined and the
 * compiler raises an error:
 *
 *   ostx_stream_pack(&ctx, "T1", OSTX_UNIT(Celsius), ...);
 *   => error: 'OSTX_UNIT_Celsius' undeclared
 *
 * IDEs that understand the C preprocessor (VS Code + clangd, CLion, etc.)
 * will red-underline the unknown token immediately.
 *
 * NOTE: OSTX_UNIT(sym) requires the literal token, not a macro-defined alias:
 *   #define MY  Cel
 *   OSTX_UNIT(MY)   -- expands to OSTX_UNIT_MY, not OSTX_UNIT_Cel
 * For macro-indirection use OSTX_UNIT_Cel directly.
 *
 * UNITS WITH NON-IDENTIFIER CHARACTERS
 * -------------------------------------
 * Some OpenSynaptic wire codes contain "/" "[" "]" "." or "%".
 * These are mapped to underscore-separated identifier suffixes:
 *
 *   Wire code   Macro suffix   Example
 *   ---------   --------       -------
 *   %           pct / RH       OSTX_UNIT(pct)
 *   deg/s       deg_s          OSTX_UNIT(deg_s)
 *   rad/s       rad_s          OSTX_UNIT(rad_s)
 *   mm[Hg]      mmHg           OSTX_UNIT(mmHg)
 *   pow.on      pow_on         OSTX_UNIT(pow_on)
 *   pow.off     pow_off        OSTX_UNIT(pow_off)
 *   ... (see Device Operations section below for full list)
 *
 * SOURCES
 * -------
 * All units are derived from the OpenSynaptic UCUM unit library v1.1.0.
 * class_id values match the OS_UNIT_SYMBOLS registry.
 */

#ifndef OSTX_UNITS_H
#define OSTX_UNITS_H

/*
 * Convenience macro -- expands sym to its wire-format string literal.
 * sym must be one of the defined suffixes in this file.
 */
#define OSTX_UNIT(sym)  OSTX_UNIT_##sym

/* =========================================================================
 * CLASS 0x01 -- Length  (base: m)
 * ========================================================================= */
#define OSTX_UNIT_m      "m"        /* meter            (SI, can prefix) */
#define OSTX_UNIT_in     "in"       /* inch */
#define OSTX_UNIT_ft     "ft"       /* foot */
#define OSTX_UNIT_nmi    "nmi"      /* nautical mile */
#define OSTX_UNIT_AU     "AU"       /* astronomical unit */
#define OSTX_UNIT_ang    "ang"      /* angstrom */

/* =========================================================================
 * CLASS 0x02 -- Mass  (base: g)
 * ========================================================================= */
#define OSTX_UNIT_g      "g"        /* gram              (SI, can prefix) */
#define OSTX_UNIT_lb     "lb"       /* pound */
#define OSTX_UNIT_oz     "oz"       /* ounce */
#define OSTX_UNIT_t      "t"        /* metric ton        (can prefix) */
#define OSTX_UNIT_u      "u"        /* unified atomic mass unit */

/* =========================================================================
 * CLASS 0x03 -- Time  (base: s)
 * ========================================================================= */
#define OSTX_UNIT_s      "s"        /* second            (SI, can prefix) */
#define OSTX_UNIT_min    "min"      /* minute */
#define OSTX_UNIT_h      "h"        /* hour */
#define OSTX_UNIT_d      "d"        /* day */
#define OSTX_UNIT_wk     "wk"       /* week */
#define OSTX_UNIT_ann    "ann"      /* year              (can prefix) */

/* =========================================================================
 * CLASS 0x04 -- Temperature  (base: K)
 * ========================================================================= */
#define OSTX_UNIT_K      "K"        /* kelvin            (SI, can prefix) */
#define OSTX_UNIT_Cel    "Cel"      /* degree Celsius */
#define OSTX_UNIT_degF   "degF"     /* degree Fahrenheit */
#define OSTX_UNIT_degRe  "degRe"    /* degree Reaumur */

/* =========================================================================
 * CLASS 0x05 -- Electric Current  (base: A)
 * ========================================================================= */
#define OSTX_UNIT_A      "A"        /* ampere            (SI, can prefix) */
#define OSTX_UNIT_Bi     "Bi"       /* biot              (can prefix) */
#define OSTX_UNIT_Gau    "Gau"      /* gauss unit */

/* =========================================================================
 * CLASS 0x06 -- Amount of Substance  (base: mol)
 * ========================================================================= */
#define OSTX_UNIT_mol    "mol"      /* mole              (SI, can prefix) */
#define OSTX_UNIT_eq     "eq"       /* equivalents       (can prefix) */
#define OSTX_UNIT_osm    "osm"      /* osmole            (can prefix) */
#define OSTX_UNIT_count  "count"    /* particle count */

/* =========================================================================
 * CLASS 0x07 -- Luminous Intensity  (base: cd)
 * ========================================================================= */
#define OSTX_UNIT_cd     "cd"       /* candela           (SI, can prefix) */
#define OSTX_UNIT_cp     "cp"       /* candlepower */
#define OSTX_UNIT_hk     "hk"       /* hefnerkerze */

/* =========================================================================
 * CLASS 0x08 -- Pressure  (base: Pa)
 * ========================================================================= */
#define OSTX_UNIT_Pa     "Pa"       /* pascal            (SI, can prefix) */
#define OSTX_UNIT_bar    "bar"      /* bar               (can prefix) */
#define OSTX_UNIT_psi    "psi"      /* pound-force per square inch */
#define OSTX_UNIT_atm    "atm"      /* standard atmosphere */
#define OSTX_UNIT_mmHg   "mm[Hg]"  /* millimeter of mercury */

/* =========================================================================
 * CLASS 0x09 -- Frequency  (base: Hz)
 * ========================================================================= */
#define OSTX_UNIT_Hz     "Hz"       /* hertz             (SI, can prefix) */
#define OSTX_UNIT_rpm    "rpm"      /* revolutions per minute */
#define OSTX_UNIT_deg_s  "deg/s"    /* degrees per second  (wire: "deg/s") */
#define OSTX_UNIT_rad_s  "rad/s"    /* radians per second  (wire: "rad/s") */

/* =========================================================================
 * CLASS 0x0A -- Energy / Power  (base: W)
 * ========================================================================= */
#define OSTX_UNIT_W      "W"        /* watt              (SI, can prefix) */
#define OSTX_UNIT_J      "J"        /* joule             (SI, can prefix) */
#define OSTX_UNIT_cal    "cal"      /* calorie           (can prefix) */
#define OSTX_UNIT_hp     "hp"       /* horsepower */

/* =========================================================================
 * CLASS 0x0B -- Electromagnetism  (base: V)
 * ========================================================================= */
#define OSTX_UNIT_V      "V"        /* volt              (SI, can prefix) */
#define OSTX_UNIT_Ohm    "Ohm"      /* ohm               (SI, can prefix) */
#define OSTX_UNIT_F      "F"        /* farad             (SI, can prefix) */

/* =========================================================================
 * CLASS 0x0C -- Informatics  (base: bit)
 * ========================================================================= */
#define OSTX_UNIT_bit    "bit"      /* bit               (can prefix) */
#define OSTX_UNIT_By     "By"       /* byte              (can prefix) */
#define OSTX_UNIT_Bd     "Bd"       /* baud              (can prefix) */

/* =========================================================================
 * CLASS 0x0D -- Humidity  (base: %)
 * ========================================================================= */
#define OSTX_UNIT_pct    "%"        /* relative humidity percent */
#define OSTX_UNIT_RH     "%"        /* alias: relative humidity (common notation) */

/* =========================================================================
 * CLASS 0x0E -- Device Operations  (base: cmd)
 * =========================================================================
 *
 * These units are used in command frames, not sensor frames.
 * Wire codes containing "." are mapped to "_" in the macro suffix.
 */
#define OSTX_UNIT_cmd      "cmd"      /* raw command */
#define OSTX_UNIT_pow_on   "pow.on"   /* power on */
#define OSTX_UNIT_pow_off  "pow.off"  /* power off */
#define OSTX_UNIT_set_val  "set.val"  /* set parameter value */
#define OSTX_UNIT_get_val  "get.val"  /* get parameter value */
#define OSTX_UNIT_get_st   "get.st"   /* get device status */
#define OSTX_UNIT_rst      "rst"      /* reset / reboot */
#define OSTX_UNIT_mv_up    "mv.up"    /* move up */
#define OSTX_UNIT_mv_dn    "mv.dn"    /* move down */
#define OSTX_UNIT_mv_lt    "mv.lt"    /* move left */
#define OSTX_UNIT_mv_rt    "mv.rt"    /* move right */
#define OSTX_UNIT_mv_fw    "mv.fw"    /* move forward */
#define OSTX_UNIT_mv_bk    "mv.bk"    /* move backward */
#define OSTX_UNIT_stp      "stp"      /* stop */
#define OSTX_UNIT_stp_e    "stp.e"    /* emergency stop */
#define OSTX_UNIT_mv_to    "mv.to"    /* move to absolute position */
#define OSTX_UNIT_mv_by    "mv.by"    /* move by relative offset */
#define OSTX_UNIT_rot_cw   "rot.cw"   /* rotate clockwise */
#define OSTX_UNIT_rot_cc   "rot.cc"   /* rotate counter-clockwise */

/* Custom command slots A-Z (0x0E13 -- 0x0E2C) */
#define OSTX_UNIT_cmdA   "cmdA"
#define OSTX_UNIT_cmdB   "cmdB"
#define OSTX_UNIT_cmdC   "cmdC"
#define OSTX_UNIT_cmdD   "cmdD"
#define OSTX_UNIT_cmdE   "cmdE"
#define OSTX_UNIT_cmdF   "cmdF"
#define OSTX_UNIT_cmdG   "cmdG"
#define OSTX_UNIT_cmdH   "cmdH"
#define OSTX_UNIT_cmdI   "cmdI"
#define OSTX_UNIT_cmdJ   "cmdJ"
#define OSTX_UNIT_cmdK   "cmdK"
#define OSTX_UNIT_cmdL   "cmdL"
#define OSTX_UNIT_cmdM   "cmdM"
#define OSTX_UNIT_cmdN   "cmdN"
#define OSTX_UNIT_cmdO   "cmdO"
#define OSTX_UNIT_cmdP   "cmdP"
#define OSTX_UNIT_cmdQ   "cmdQ"
#define OSTX_UNIT_cmdR   "cmdR"
#define OSTX_UNIT_cmdS   "cmdS"
#define OSTX_UNIT_cmdT   "cmdT"
#define OSTX_UNIT_cmdU   "cmdU"
#define OSTX_UNIT_cmdV   "cmdV"
#define OSTX_UNIT_cmdW   "cmdW"
#define OSTX_UNIT_cmdX   "cmdX"
#define OSTX_UNIT_cmdY   "cmdY"
#define OSTX_UNIT_cmdZ   "cmdZ"

/* Mode switch slots A-Z (0x0E2D -- 0x0E46) */
#define OSTX_UNIT_modeA  "modeA"
#define OSTX_UNIT_modeB  "modeB"
#define OSTX_UNIT_modeC  "modeC"
#define OSTX_UNIT_modeD  "modeD"
#define OSTX_UNIT_modeE  "modeE"
#define OSTX_UNIT_modeF  "modeF"
#define OSTX_UNIT_modeG  "modeG"
#define OSTX_UNIT_modeH  "modeH"
#define OSTX_UNIT_modeI  "modeI"
#define OSTX_UNIT_modeJ  "modeJ"
#define OSTX_UNIT_modeK  "modeK"
#define OSTX_UNIT_modeL  "modeL"
#define OSTX_UNIT_modeM  "modeM"
#define OSTX_UNIT_modeN  "modeN"
#define OSTX_UNIT_modeO  "modeO"
#define OSTX_UNIT_modeP  "modeP"
#define OSTX_UNIT_modeQ  "modeQ"
#define OSTX_UNIT_modeR  "modeR"
#define OSTX_UNIT_modeS  "modeS"
#define OSTX_UNIT_modeT  "modeT"
#define OSTX_UNIT_modeU  "modeU"
#define OSTX_UNIT_modeV  "modeV"
#define OSTX_UNIT_modeW  "modeW"
#define OSTX_UNIT_modeX  "modeX"
#define OSTX_UNIT_modeY  "modeY"
#define OSTX_UNIT_modeZ  "modeZ"

#endif /* OSTX_UNITS_H */

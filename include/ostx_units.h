/*
 * ostx_units.h -- OpenSynaptic unit wire-code library (compile-time encoding).
 *
 * Every macro expands to the OS_Symbols.json wire code for that unit.
 * No code is compiled; all definitions are pure string literals baked into
 * Flash at compile time (each ~3-4 bytes in the packet body, not in RAM).
 *
 * USAGE
 * -----
 * Pass unit codes to any pack function using OSTX_UNIT(sym):
 *
 *   OSTX_STATIC_DEFINE(s_temp, aid, "T1", OSTX_UNIT(Cel));
 *   ostx_sensor_pack(aid, tid, ts, "T1", OSTX_UNIT(Cel),  215000L, buf);
 *   ostx_stream_pack(&ctx, "H1",   OSTX_UNIT(pct),        620000L, buf, len);
 *
 * The unit field in the wire body will contain the OS_Symbols.json code:
 *   "T1|A01|3Qw"   (not "T1|Cel|3Qw")
 *
 * COMPILE-TIME VALIDATION
 * -----------------------
 * OSTX_UNIT(sym) expands to OSTX_UNIT_##sym.
 * If <sym> is not defined, the compiler raises:
 *   error: 'OSTX_UNIT_Celsius' undeclared
 *
 * NOTE: OSTX_UNIT requires the literal token, not a macro alias:
 *   #define MY  Cel
 *   OSTX_UNIT(MY)   -- expands to OSTX_UNIT_MY, not OSTX_UNIT_Cel
 * For macro-indirection use OSTX_UNIT_Cel directly.
 *
 * UNITS WITH NON-IDENTIFIER CHARACTERS
 * --------------------------------------
 *   OS name    Macro suffix   Wire code
 *   ---------  -----------    ---------
 *   %          pct / RH       "C00"
 *   deg/s      deg_s          "402"
 *   rad/s      rad_s          "403"
 *   mm[Hg]     mmHg           "903"
 *   pow.on     pow_on         "D01"
 *   pow.off    pow_off        "D02"
 *   ... (see Device Operations section below)
 *
 * SOURCES
 * -------
 * Wire codes derived from OpenSynaptic OS_Symbols.json v1.1.0.
 * All codes are the string value in the "units" object.
 */

#ifndef OSTX_UNITS_H
#define OSTX_UNITS_H

/*
 * Convenience macro -- expands sym to its wire-code string literal.
 * sym must be one of the defined suffixes in this file.
 */
#define OSTX_UNIT(sym)  OSTX_UNIT_##sym

/* =========================================================================
 * CLASS 6 -- Length  (base: m = "600")
 * ========================================================================= */
#define OSTX_UNIT_m      "600"      /* meter            (SI, can prefix) */
#define OSTX_UNIT_in     "601"      /* inch */
#define OSTX_UNIT_ft     "602"      /* foot */
#define OSTX_UNIT_nmi    "603"      /* nautical mile */
#define OSTX_UNIT_AU     "604"      /* astronomical unit */
#define OSTX_UNIT_ang    "605"      /* angstrom */

/* =========================================================================
 * CLASS 8 -- Mass  (base: g = "800")
 * ========================================================================= */
#define OSTX_UNIT_g      "800"      /* gram              (SI, can prefix) */
#define OSTX_UNIT_lb     "801"      /* pound */
#define OSTX_UNIT_oz     "802"      /* ounce */
#define OSTX_UNIT_t      "803"      /* metric ton        (can prefix) */
#define OSTX_UNIT_u      "804"      /* unified atomic mass unit */

/* =========================================================================
 * CLASS B -- Time  (base: s = "B00")
 * ========================================================================= */
#define OSTX_UNIT_s      "B00"      /* second            (SI, can prefix) */
#define OSTX_UNIT_min    "B01"      /* minute */
#define OSTX_UNIT_h      "B02"      /* hour */
#define OSTX_UNIT_d      "B03"      /* day */
#define OSTX_UNIT_wk     "B04"      /* week */
#define OSTX_UNIT_ann    "B05"      /* year              (can prefix) */

/* =========================================================================
 * CLASS A -- Temperature  (base: K = "A00")
 * ========================================================================= */
#define OSTX_UNIT_K      "A00"      /* kelvin            (SI, can prefix) */
#define OSTX_UNIT_Cel    "A01"      /* degree Celsius    (OS: cel) */
#define OSTX_UNIT_degF   "A02"      /* degree Fahrenheit (OS: degf) */
#define OSTX_UNIT_degRe  "A03"      /* degree Reaumur    (OS: degre) */

/* =========================================================================
 * CLASS 1 -- Electric Current  (base: A = "100")
 * ========================================================================= */
#define OSTX_UNIT_A      "100"      /* ampere            (SI, can prefix) */
#define OSTX_UNIT_Bi     "101"      /* biot              (can prefix) */
#define OSTX_UNIT_Gau    "102"      /* gauss unit */

/* =========================================================================
 * CLASS 0 -- Amount of Substance  (base: mol = "000")
 * ========================================================================= */
#define OSTX_UNIT_mol    "000"      /* mole              (SI, can prefix) */
#define OSTX_UNIT_eq     "001"      /* equivalents       (can prefix) */
#define OSTX_UNIT_osm    "002"      /* osmole            (can prefix) */
#define OSTX_UNIT_count  "003"      /* particle count */

/* =========================================================================
 * CLASS 7 -- Luminous Intensity  (base: cd = "700")
 * ========================================================================= */
#define OSTX_UNIT_cd     "700"      /* candela           (SI, can prefix) */
#define OSTX_UNIT_cp     "701"      /* candlepower */
#define OSTX_UNIT_hk     "702"      /* hefnerkerze */

/* =========================================================================
 * CLASS 9 -- Pressure  (base: Pa = "900")
 * ========================================================================= */
#define OSTX_UNIT_Pa     "900"      /* pascal            (SI, can prefix) */
#define OSTX_UNIT_bar    "901"      /* bar               (can prefix) */
#define OSTX_UNIT_psi    "902"      /* pound-force per square inch */
#define OSTX_UNIT_atm    "900"      /* standard atmosphere (same code as Pa) */
#define OSTX_UNIT_mmHg   "903"      /* millimeter of mercury (OS: mm[hg]) */

/* =========================================================================
 * CLASS 4 -- Frequency  (base: Hz = "400")
 * ========================================================================= */
#define OSTX_UNIT_Hz     "400"      /* hertz             (SI, can prefix) */
#define OSTX_UNIT_rpm    "401"      /* revolutions per minute */
#define OSTX_UNIT_deg_s  "402"      /* degrees per second  (OS: deg/s) */
#define OSTX_UNIT_rad_s  "403"      /* radians per second  (OS: rad/s) */

/* =========================================================================
 * CLASS 3 -- Energy / Power  (base: W = "300")
 * ========================================================================= */
#define OSTX_UNIT_W      "300"      /* watt              (SI, can prefix) */
#define OSTX_UNIT_J      "301"      /* joule             (SI, can prefix) */
#define OSTX_UNIT_cal    "302"      /* calorie           (can prefix) */
#define OSTX_UNIT_hp     "303"      /* horsepower */

/* =========================================================================
 * CLASS 2 -- Electromagnetism  (base: V = "200")
 * ========================================================================= */
#define OSTX_UNIT_V      "200"      /* volt              (SI, can prefix) */
#define OSTX_UNIT_Ohm    "201"      /* ohm               (SI, can prefix) */
#define OSTX_UNIT_F      "202"      /* farad             (SI, can prefix) */

/* =========================================================================
 * CLASS 5 -- Informatics  (base: bit = "500")
 * ========================================================================= */
#define OSTX_UNIT_bit    "500"      /* bit               (can prefix) */
#define OSTX_UNIT_By     "501"      /* byte              (can prefix) */
#define OSTX_UNIT_Bd     "502"      /* baud              (can prefix) */

/* =========================================================================
 * CLASS C -- Humidity  (base: % = "C00")
 * ========================================================================= */
#define OSTX_UNIT_pct    "C00"      /* relative humidity percent (OS: %) */
#define OSTX_UNIT_RH     "C00"      /* alias: relative humidity */

/* =========================================================================
 * CLASS D -- Device Operations
 * =========================================================================
 *
 * These codes are used in command frames, not sensor frames.
 * "." in OS names is mapped to "_" in the macro suffix.
 */
#define OSTX_UNIT_cmd      "D00"    /* raw command */
#define OSTX_UNIT_pow_on   "D01"    /* power on */
#define OSTX_UNIT_pow_off  "D02"    /* power off */
#define OSTX_UNIT_set_val  "D03"    /* set parameter value */
#define OSTX_UNIT_get_val  "D04"    /* get parameter value */
#define OSTX_UNIT_get_st   "D05"    /* get device status */
#define OSTX_UNIT_rst      "D06"    /* reset / reboot */
#define OSTX_UNIT_mv_up    "D07"    /* move up */
#define OSTX_UNIT_mv_dn    "D08"    /* move down */
#define OSTX_UNIT_mv_lt    "D09"    /* move left */
#define OSTX_UNIT_mv_rt    "D0A"    /* move right */
#define OSTX_UNIT_mv_fw    "D0B"    /* move forward */
#define OSTX_UNIT_mv_bk    "D0C"    /* move backward */
#define OSTX_UNIT_stp      "D0D"    /* stop */
#define OSTX_UNIT_stp_e    "D0E"    /* emergency stop */
#define OSTX_UNIT_mv_to    "D0F"    /* move to absolute position */
#define OSTX_UNIT_mv_by    "D10"    /* move by relative offset */
#define OSTX_UNIT_rot_cw   "D11"    /* rotate clockwise */
#define OSTX_UNIT_rot_cc   "D12"    /* rotate counter-clockwise */

/* Custom command slots A-Z (D13 -- D2C) */
#define OSTX_UNIT_cmdA   "D13"
#define OSTX_UNIT_cmdB   "D14"
#define OSTX_UNIT_cmdC   "D15"
#define OSTX_UNIT_cmdD   "D16"
#define OSTX_UNIT_cmdE   "D17"
#define OSTX_UNIT_cmdF   "D18"
#define OSTX_UNIT_cmdG   "D19"
#define OSTX_UNIT_cmdH   "D1A"
#define OSTX_UNIT_cmdI   "D1B"
#define OSTX_UNIT_cmdJ   "D1C"
#define OSTX_UNIT_cmdK   "D1D"
#define OSTX_UNIT_cmdL   "D1E"
#define OSTX_UNIT_cmdM   "D1F"
#define OSTX_UNIT_cmdN   "D20"
#define OSTX_UNIT_cmdO   "D21"
#define OSTX_UNIT_cmdP   "D22"
#define OSTX_UNIT_cmdQ   "D23"
#define OSTX_UNIT_cmdR   "D24"
#define OSTX_UNIT_cmdS   "D25"
#define OSTX_UNIT_cmdT   "D26"
#define OSTX_UNIT_cmdU   "D27"
#define OSTX_UNIT_cmdV   "D28"
#define OSTX_UNIT_cmdW   "D29"
#define OSTX_UNIT_cmdX   "D2A"
#define OSTX_UNIT_cmdY   "D2B"
#define OSTX_UNIT_cmdZ   "D2C"

/* Mode switch slots A-Z (D2D -- D46) */
#define OSTX_UNIT_modeA  "D2D"
#define OSTX_UNIT_modeB  "D2E"
#define OSTX_UNIT_modeC  "D2F"
#define OSTX_UNIT_modeD  "D30"
#define OSTX_UNIT_modeE  "D31"
#define OSTX_UNIT_modeF  "D32"
#define OSTX_UNIT_modeG  "D33"
#define OSTX_UNIT_modeH  "D34"
#define OSTX_UNIT_modeI  "D35"
#define OSTX_UNIT_modeJ  "D36"
#define OSTX_UNIT_modeK  "D37"
#define OSTX_UNIT_modeL  "D38"
#define OSTX_UNIT_modeM  "D39"
#define OSTX_UNIT_modeN  "D3A"
#define OSTX_UNIT_modeO  "D3B"
#define OSTX_UNIT_modeP  "D3C"
#define OSTX_UNIT_modeQ  "D3D"
#define OSTX_UNIT_modeR  "D3E"
#define OSTX_UNIT_modeS  "D3F"
#define OSTX_UNIT_modeT  "D40"
#define OSTX_UNIT_modeU  "D41"
#define OSTX_UNIT_modeV  "D42"
#define OSTX_UNIT_modeW  "D43"
#define OSTX_UNIT_modeX  "D44"
#define OSTX_UNIT_modeY  "D45"
#define OSTX_UNIT_modeZ  "D46"

#endif /* OSTX_UNITS_H */

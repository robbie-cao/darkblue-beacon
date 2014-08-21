// BCM2045 direct address map, automatically generated
// DO NOT MODIFY BY HAND
// Program /projects/BCM2039/vault/ml5001/bin/map2direct
// Generated on Wed Mar 30 17:32:20 PDT 2011
// By user xiegx
// Host Linux lc-irv-1525 2.6.9-89.0.25.0.1.ELlargesmp #1 SMP Wed May 5 17:33:50 EDT 2010 x86_64 x86_64 x86_64 GNU/Linux

#define base_code_adr        0x00000000
#define base_code            (*(volatile unsigned int *)base_code_adr)
#define base_sram_adr        0x20000000
#define base_sram            (*(volatile unsigned int *)base_sram_adr)
#define base_peripherals_adr 0x40000000
#define base_peripherals     (*(volatile unsigned int *)base_peripherals_adr)
#define base_extern_sram_adr 0x60000000
#define base_extern_sram     (*(volatile unsigned int *)base_extern_sram_adr)
#define base_extern_device_adr 0xa0000000
#define base_extern_device   (*(volatile unsigned int *)base_extern_device_adr)
#define base_ppb_adr         0xe0000000
#define base_ppb             (*(volatile unsigned int *)base_ppb_adr)
#define base_IntIRom_adr     0x00000000
#define base_IntIRom         (*(volatile unsigned int *)base_IntIRom_adr)
#define last_IntIRom         0x0004ffff
#define base_flash_bank0_adr 0x00100000
#define base_flash_bank0     (*(volatile unsigned int *)base_flash_bank0_adr)
#define base_flash_bank1_adr 0x00180000
#define base_flash_bank1     (*(volatile unsigned int *)base_flash_bank1_adr)
#define base_IntMemCm3_adr   0x00200000
#define base_IntMemCm3       (*(volatile unsigned int *)base_IntMemCm3_adr)
#define last_IntMemCm3       0x0020deff
#define base_IntMemPrc_adr   0x0020ef00
#define base_IntMemPrc       (*(volatile unsigned int *)base_IntMemPrc_adr)
#define last_IntMemPrc       0x0020efff
#define base_hw_regs_cm3_adr 0x00300000
#define base_hw_regs_cm3     (*(volatile unsigned int *)base_hw_regs_cm3_adr)
#define base_hw_regs_dma_adr 0x00370000
#define base_hw_regs_dma     (*(volatile unsigned int *)base_hw_regs_dma_adr)
#define base_rtx_fifo_adr    0x00370000
#define base_rtx_fifo        (*(volatile unsigned int *)base_rtx_fifo_adr)
#define base_rf_regs_adr     0x00600000
#define base_rf_regs         (*(volatile unsigned int *)base_rf_regs_adr)
#define rom_adr_base         0x00000000
#define smi_mem_adr_base     0xf0000000
#define ram_adr_base         0x00200000
#define pmu_adr_base         0x00320000
#define mtu_adr_base         0x00324000
#define fmm_fifo_adr_base    0x00325000
#define aud_fifo_adr_base    0x00326000
#define aud_fifo_regs_adr_base 0x00326c00
#define audio_adr_base       0x00327000
#define apb_adr_base         0x00328000
#define modem_adr_base       0x0031fc00
#define rf_adr_base          0x00600000
#define sri_bk_adr_base      0x0031e000
#define aoa2_adr_base        0x0031b000
#define aoa_adr_base         0x0031a000
#define phy_adr_base         0x00318000
#define ptu_adr_base         0x00360000
#define lhl_adr_base         0x00390000
#define prc_adr_base         0x00310000
#define smi_ahb_adr_base     0x00308000
#define dmac_adr_base        0x00300000
#define rmppause_adr_base    0x00328000
#define watchdog_adr_base    0x00329000
#define timers_adr_base      0x0032a000
#define gpio_adr_base        0x0032c000
#define wdogload_adr         0x00329000
#define wdogload             (*(volatile unsigned int *)wdogload_adr)
#define wdogvalue_adr        0x00329004
#define wdogvalue            (*(volatile unsigned int *)wdogvalue_adr)
#define wdogcontrol_adr      0x00329008
#define wdogcontrol          (*(volatile unsigned int *)wdogcontrol_adr)
#define wdogintclr_adr       0x0032900c
#define wdogintclr           (*(volatile unsigned int *)wdogintclr_adr)
#define wdogris_adr          0x00329010
#define wdogris              (*(volatile unsigned int *)wdogris_adr)
#define wdogmis_adr          0x00329014
#define wdogmis              (*(volatile unsigned int *)wdogmis_adr)
#define wdoglock_adr         0x00329c00
#define wdoglock             (*(volatile unsigned int *)wdoglock_adr)
#define wdogitcr_adr         0x00329f00
#define wdogitcr             (*(volatile unsigned int *)wdogitcr_adr)
#define wdogitop_adr         0x00329f04
#define wdogitop             (*(volatile unsigned int *)wdogitop_adr)
#define wdogperiphid0_adr    0x00329fe0
#define wdogperiphid0        (*(volatile unsigned int *)wdogperiphid0_adr)
#define wdogperiphid1_adr    0x00329fe4
#define wdogperiphid1        (*(volatile unsigned int *)wdogperiphid1_adr)
#define wdogperiphid2_adr    0x00329fe8
#define wdogperiphid2        (*(volatile unsigned int *)wdogperiphid2_adr)
#define wdogperiphid3_adr    0x00329fec
#define wdogperiphid3        (*(volatile unsigned int *)wdogperiphid3_adr)
#define wdogpcellid0_adr     0x00329ff0
#define wdogpcellid0         (*(volatile unsigned int *)wdogpcellid0_adr)
#define wdogpcellid1_adr     0x00329ff4
#define wdogpcellid1         (*(volatile unsigned int *)wdogpcellid1_adr)
#define wdogpcellid2_adr     0x00329ff8
#define wdogpcellid2         (*(volatile unsigned int *)wdogpcellid2_adr)
#define wdogpcellid3_adr     0x00329ffc
#define wdogpcellid3         (*(volatile unsigned int *)wdogpcellid3_adr)
#define timer1load_adr       0x0032a000
#define timer1load           (*(volatile unsigned int *)timer1load_adr)
#define timer1value_adr      0x0032a004
#define timer1value          (*(volatile unsigned int *)timer1value_adr)
#define timer1control_adr    0x0032a008
#define timer1control        (*(volatile unsigned int *)timer1control_adr)
#define timer1intclr_adr     0x0032a00c
#define timer1intclr         (*(volatile unsigned int *)timer1intclr_adr)
#define timer1ris_adr        0x0032a010
#define timer1ris            (*(volatile unsigned int *)timer1ris_adr)
#define timer1mis_adr        0x0032a014
#define timer1mis            (*(volatile unsigned int *)timer1mis_adr)
#define timer1bgload_adr     0x0032a018
#define timer1bgload         (*(volatile unsigned int *)timer1bgload_adr)
#define timer1test_adr       0x0032a01c
#define timer1test           (*(volatile unsigned int *)timer1test_adr)
#define timer2load_adr       0x0032a020
#define timer2load           (*(volatile unsigned int *)timer2load_adr)
#define timer2value_adr      0x0032a024
#define timer2value          (*(volatile unsigned int *)timer2value_adr)
#define timer2control_adr    0x0032a028
#define timer2control        (*(volatile unsigned int *)timer2control_adr)
#define timer2intclr_adr     0x0032a02c
#define timer2intclr         (*(volatile unsigned int *)timer2intclr_adr)
#define timer2ris_adr        0x0032a030
#define timer2ris            (*(volatile unsigned int *)timer2ris_adr)
#define timer2mis_adr        0x0032a034
#define timer2mis            (*(volatile unsigned int *)timer2mis_adr)
#define timer2bgload_adr     0x0032a038
#define timer2bgload         (*(volatile unsigned int *)timer2bgload_adr)
#define timer2test_adr       0x0032a03c
#define timer2test           (*(volatile unsigned int *)timer2test_adr)
#define timeritcr_adr        0x0032af00
#define timeritcr            (*(volatile unsigned int *)timeritcr_adr)
#define timeritop_adr        0x0032af04
#define timeritop            (*(volatile unsigned int *)timeritop_adr)
#define tperiphid0_adr       0x0032afe0
#define tperiphid0           (*(volatile unsigned int *)tperiphid0_adr)
#define tperiphid1_adr       0x0032afe4
#define tperiphid1           (*(volatile unsigned int *)tperiphid1_adr)
#define tperiphid2_adr       0x0032afe8
#define tperiphid2           (*(volatile unsigned int *)tperiphid2_adr)
#define tperiphid3_adr       0x0032afec
#define tperiphid3           (*(volatile unsigned int *)tperiphid3_adr)
#define tpcellid0_adr        0x0032aff0
#define tpcellid0            (*(volatile unsigned int *)tpcellid0_adr)
#define tpcellid1_adr        0x0032aff4
#define tpcellid1            (*(volatile unsigned int *)tpcellid1_adr)
#define tpcellid2_adr        0x0032aff8
#define tpcellid2            (*(volatile unsigned int *)tpcellid2_adr)
#define tpcellid3_adr        0x0032affc
#define tpcellid3            (*(volatile unsigned int *)tpcellid3_adr)
#define gpiodata_adr         0x0032c000
// GPIO data covers a range of 256 words (one KB).  See ARM DDI 0190B, PL061 documentation
#define gpiodir_adr          0x0032c400
#define gpiodir              (*(volatile unsigned int *)gpiodir_adr)
#define gpiois_adr           0x0032c404
#define gpiois               (*(volatile unsigned int *)gpiois_adr)
#define gpioibe_adr          0x0032c408
#define gpioibe              (*(volatile unsigned int *)gpioibe_adr)
#define gpioiev_adr          0x0032c40c
#define gpioiev              (*(volatile unsigned int *)gpioiev_adr)
#define gpioie_adr           0x0032c410
#define gpioie               (*(volatile unsigned int *)gpioie_adr)
#define gpioris_adr          0x0032c414
#define gpioris              (*(volatile unsigned int *)gpioris_adr)
#define gpiomis_adr          0x0032c418
#define gpiomis              (*(volatile unsigned int *)gpiomis_adr)
#define gpioic_adr           0x0032c41c
#define gpioic               (*(volatile unsigned int *)gpioic_adr)
#define gpioafsel_adr        0x0032c420
#define gpioafsel            (*(volatile unsigned int *)gpioafsel_adr)
#define gpioitcr_adr         0x0032c600
#define gpioitcr             (*(volatile unsigned int *)gpioitcr_adr)
#define gpioitip1_adr        0x0032c604
#define gpioitip1            (*(volatile unsigned int *)gpioitip1_adr)
#define gpioitip2_adr        0x0032c608
#define gpioitip2            (*(volatile unsigned int *)gpioitip2_adr)
#define gpioitop1_adr        0x0032c60c
#define gpioitop1            (*(volatile unsigned int *)gpioitop1_adr)
#define gpioitop2_adr        0x0032c610
#define gpioitop2            (*(volatile unsigned int *)gpioitop2_adr)
#define gpioitop3_adr        0x0032c614
#define gpioitop3            (*(volatile unsigned int *)gpioitop3_adr)
#define gpioperiphid0_adr    0x0032cfe0
#define gpioperiphid0        (*(volatile unsigned int *)gpioperiphid0_adr)
#define gpioperiphid1_adr    0x0032cfe4
#define gpioperiphid1        (*(volatile unsigned int *)gpioperiphid1_adr)
#define gpioperiphid2_adr    0x0032cfe8
#define gpioperiphid2        (*(volatile unsigned int *)gpioperiphid2_adr)
#define gpioperiphid3_adr    0x0032cfec
#define gpioperiphid3        (*(volatile unsigned int *)gpioperiphid3_adr)
#define gpiopcellid0_adr     0x0032cff0
#define gpiopcellid0         (*(volatile unsigned int *)gpiopcellid0_adr)
#define gpiopcellid1_adr     0x0032cff4
#define gpiopcellid1         (*(volatile unsigned int *)gpiopcellid1_adr)
#define gpiopcellid2_adr     0x0032cff8
#define gpiopcellid2         (*(volatile unsigned int *)gpiopcellid2_adr)
#define gpiopcellid3_adr     0x0032cffc
#define gpiopcellid3         (*(volatile unsigned int *)gpiopcellid3_adr)
#define cr_clk_div_sel_adr   0x00320000
#define cr_clk_div_sel       (*(volatile unsigned int *)cr_clk_div_sel_adr)
#define cr_clk_src_sel_adr   0x00320004
#define cr_clk_src_sel       (*(volatile unsigned int *)cr_clk_src_sel_adr)
#define cr_clk_switch_adr    0x00320008
#define cr_clk_switch        (*(volatile unsigned int *)cr_clk_switch_adr)
#define cr_xtal_lpo_div_adr  0x0032000c
#define cr_xtal_lpo_div      (*(volatile unsigned int *)cr_xtal_lpo_div_adr)
#define cr_lpo_sel_adr       0x00320010
#define cr_lpo_sel           (*(volatile unsigned int *)cr_lpo_sel_adr)
#define cr_err_est_cnt_adr   0x00320014
#define cr_err_est_cnt       (*(volatile unsigned int *)cr_err_est_cnt_adr)
#define cr_err_result_adr    0x00320018
#define cr_err_result        (*(volatile unsigned int *)cr_err_result_adr)
#define cr_err_en_adr        0x0032001c
#define cr_err_en            (*(volatile unsigned int *)cr_err_en_adr)
#define cr_sleep_cnt_adr     0x00320020
#define cr_sleep_cnt         (*(volatile unsigned int *)cr_sleep_cnt_adr)
#define cr_warmup_cnt_adr    0x00320024
#define cr_warmup_cnt        (*(volatile unsigned int *)cr_warmup_cnt_adr)
#define cr_sleep_en_adr      0x00320028
#define cr_sleep_en          (*(volatile unsigned int *)cr_sleep_en_adr)
#define cr_sleep_remain_adr  0x0032002c
#define cr_sleep_remain      (*(volatile unsigned int *)cr_sleep_remain_adr)
#define version_adr          0x00320030
#define version              (*(volatile unsigned int *)version_adr)
#define xtal_strap_adr       0x00320034
#define xtal_strap           (*(volatile unsigned int *)xtal_strap_adr)
#define cr_rpu_gate_ctl_adr  0x00320038
#define cr_rpu_gate_ctl      (*(volatile unsigned int *)cr_rpu_gate_ctl_adr)
#define cr_mem_stby_ctl_adr0 0x0032003c
#define cr_mem_stby_ctl0     (*(volatile unsigned int *)cr_mem_stby_ctl_adr0)
#define cr_xtal_1m_int_adr   0x00320040
#define cr_xtal_1m_int       (*(volatile unsigned int *)cr_xtal_1m_int_adr)
#define cr_xtal_1m_frac_adr  0x00320044
#define cr_xtal_1m_frac      (*(volatile unsigned int *)cr_xtal_1m_frac_adr)
#define cr_i2c_pump_adr      0x00320048
#define cr_i2c_pump          (*(volatile unsigned int *)cr_i2c_pump_adr)
#define cr_wake_int_en_adr   0x00320058
#define cr_wake_int_en       (*(volatile unsigned int *)cr_wake_int_en_adr)
#define cr_pulse_reset_adr   0x0032005c
#define cr_pulse_reset       (*(volatile unsigned int *)cr_pulse_reset_adr)
#define cr_level_reset_adr   0x00320060
#define cr_level_reset       (*(volatile unsigned int *)cr_level_reset_adr)
#define cr_mem_ctl_adr       0x00320064
#define cr_mem_ctl           (*(volatile unsigned int *)cr_mem_ctl_adr)
#define cr_pad_config_adr0   0x00320068
#define cr_pad_config0       (*(volatile unsigned int *)cr_pad_config_adr0)
#define cr_pad_config_adr1   0x0032006c
#define cr_pad_config1       (*(volatile unsigned int *)cr_pad_config_adr1)
#define cr_pad_config_adr2   0x00320070
#define cr_pad_config2       (*(volatile unsigned int *)cr_pad_config_adr2)
#define cr_pad_config_adr3   0x00320074
#define cr_pad_config3       (*(volatile unsigned int *)cr_pad_config_adr3)
#define cr_pad_config_adr4   0x00320078
#define cr_pad_config4       (*(volatile unsigned int *)cr_pad_config_adr4)
#define cr_pad_config_adr5   0x0032007c
#define cr_pad_config5       (*(volatile unsigned int *)cr_pad_config_adr5)
#define cr_pad_config_adr6   0x00320080
#define cr_pad_config6       (*(volatile unsigned int *)cr_pad_config_adr6)
#define cr_pad_dbg_ctl_adr   0x00320084
#define cr_pad_dbg_ctl       (*(volatile unsigned int *)cr_pad_dbg_ctl_adr)
#define cr_pad_fcn_ctl_adr0  0x00320088
#define cr_pad_fcn_ctl0      (*(volatile unsigned int *)cr_pad_fcn_ctl_adr0)
#define cr_pad_fcn_ctl_adr1  0x0032008c
#define cr_pad_fcn_ctl1      (*(volatile unsigned int *)cr_pad_fcn_ctl_adr1)
#define cr_pad_fcn_ctl_adr2  0x00320090
#define cr_pad_fcn_ctl2      (*(volatile unsigned int *)cr_pad_fcn_ctl_adr2)
#define cr_ptu_clk_en_adr    0x00320094
#define cr_ptu_clk_en        (*(volatile unsigned int *)cr_ptu_clk_en_adr)
#define cr_bpl_upi_gate_ctl_adr 0x00320098
#define cr_bpl_upi_gate_ctl  (*(volatile unsigned int *)cr_bpl_upi_gate_ctl_adr)
#define cr_pll_wrmup_cnt_adr 0x0032009c
#define cr_pll_wrmup_cnt     (*(volatile unsigned int *)cr_pll_wrmup_cnt_adr)
#define cr_sdio_clk_sel_adr  0x003200a0
#define cr_sdio_clk_sel      (*(volatile unsigned int *)cr_sdio_clk_sel_adr)
#define cr_clk_fmref_ctl_adr 0x003200a0
#define cr_clk_fmref_ctl     (*(volatile unsigned int *)cr_clk_fmref_ctl_adr)
#define cr_pwr_ctl_adr       0x003200a4
#define cr_pwr_ctl           (*(volatile unsigned int *)cr_pwr_ctl_adr)
#define cr_mem_ovst_ctl_adr  0x003200a8
#define cr_mem_ovst_ctl      (*(volatile unsigned int *)cr_mem_ovst_ctl_adr)
#define cr_xtal_buf_dly_byp_adr 0x003200ac
#define cr_xtal_buf_dly_byp  (*(volatile unsigned int *)cr_xtal_buf_dly_byp_adr)
#define cr_tport_clk_sel_adr 0x003200b4
#define cr_tport_clk_sel     (*(volatile unsigned int *)cr_tport_clk_sel_adr)
#define cr_ptu_clk_gate_on_adr 0x003200b8
#define cr_ptu_clk_gate_on   (*(volatile unsigned int *)cr_ptu_clk_gate_on_adr)
#define cr_ptu_clk_gate_off_adr 0x003200bc
#define cr_ptu_clk_gate_off  (*(volatile unsigned int *)cr_ptu_clk_gate_off_adr)
#define sr_pmu_status_adr    0x003200c0
#define sr_pmu_status        (*(volatile unsigned int *)sr_pmu_status_adr)
#define sr_pmu_en_adr        0x003200c4
#define sr_pmu_en            (*(volatile unsigned int *)sr_pmu_en_adr)
#define sr_pmu_sdis_adr      0x003200c8
#define sr_pmu_sdis          (*(volatile unsigned int *)sr_pmu_sdis_adr)
#define fm_sleep_cnt_adr     0x003200cc
#define fm_sleep_cnt         (*(volatile unsigned int *)fm_sleep_cnt_adr)
#define fm_warn_cnt_adr      0x003200d0
#define fm_warn_cnt          (*(volatile unsigned int *)fm_warn_cnt_adr)
#define fm_warmup_cnt_adr    0x003200d8
#define fm_warmup_cnt        (*(volatile unsigned int *)fm_warmup_cnt_adr)
#define cr_mtu_clk_en_adr    0x003200dc
#define cr_mtu_clk_en        (*(volatile unsigned int *)cr_mtu_clk_en_adr)
#define cr_clk_req_cfg_adr   0x003200e0
#define cr_clk_req_cfg       (*(volatile unsigned int *)cr_clk_req_cfg_adr)
#define cr_pad_config_adr7   0x003200e4
#define cr_pad_config7       (*(volatile unsigned int *)cr_pad_config_adr7)
#define fm_sleep_en_adr      0x003200e8
#define fm_sleep_en          (*(volatile unsigned int *)fm_sleep_en_adr)
#define fm_sleep_remain_adr  0x003200ec
#define fm_sleep_remain      (*(volatile unsigned int *)fm_sleep_remain_adr)
#define cr_mem_tm_ctl_adr    0x003200f0
#define cr_mem_tm_ctl        (*(volatile unsigned int *)cr_mem_tm_ctl_adr)
#define cr_mem_stby_ctl_adr1 0x003200f4
#define cr_mem_stby_ctl1     (*(volatile unsigned int *)cr_mem_stby_ctl_adr1)
#define cr_wake_int_en_2_adr 0x003200f8
#define cr_wake_int_en_2     (*(volatile unsigned int *)cr_wake_int_en_2_adr)
#define cr_buf_wrmup_cnt_adr 0x003200fc
#define cr_buf_wrmup_cnt     (*(volatile unsigned int *)cr_buf_wrmup_cnt_adr)
#define cr_pulse_reset_peri_adr 0x00320100
#define cr_pulse_reset_peri  (*(volatile unsigned int *)cr_pulse_reset_peri_adr)
#define cr_level_reset_peri_adr 0x00320104
#define cr_level_reset_peri  (*(volatile unsigned int *)cr_level_reset_peri_adr)
#define fm_hclk_fm_on_adr    0x00320108
#define fm_hclk_fm_on        (*(volatile unsigned int *)fm_hclk_fm_on_adr)
#define cr_pad_config_adr8   0x00320110
#define cr_pad_config8       (*(volatile unsigned int *)cr_pad_config_adr8)
#define cr_pad_config_adr9   0x00320114
#define cr_pad_config9       (*(volatile unsigned int *)cr_pad_config_adr9)
#define cr_pad_fcn_ctl_adr3  0x00320118
#define cr_pad_fcn_ctl3      (*(volatile unsigned int *)cr_pad_fcn_ctl_adr3)
#define cr_smux_ctl_adr0     0x0032011c
#define cr_smux_ctl0         (*(volatile unsigned int *)cr_smux_ctl_adr0)
#define cr_smux_ctl_adr1     0x00320120
#define cr_smux_ctl1         (*(volatile unsigned int *)cr_smux_ctl_adr1)
#define cr_smux_ctl_adr2     0x00320124
#define cr_smux_ctl2         (*(volatile unsigned int *)cr_smux_ctl_adr2)
#define cr_smux_ctl_adr3     0x00320128
#define cr_smux_ctl3         (*(volatile unsigned int *)cr_smux_ctl_adr3)
#define cr_smux_ctl_adr4     0x0032012c
#define cr_smux_ctl4         (*(volatile unsigned int *)cr_smux_ctl_adr4)
#define cr_smux_ctl_adr5     0x00320130
#define cr_smux_ctl5         (*(volatile unsigned int *)cr_smux_ctl_adr5)
#define cr_smux_ctl_adr8     0x00320134
#define cr_smux_ctl8         (*(volatile unsigned int *)cr_smux_ctl_adr8)
#define cr_smux_ctl_adr6     0x00320138
#define cr_smux_ctl6         (*(volatile unsigned int *)cr_smux_ctl_adr6)
#define cr_smux_ctl_adr7     0x0032013c
#define cr_smux_ctl7         (*(volatile unsigned int *)cr_smux_ctl_adr7)
#define cr_smux_ctl_adr9     0x00320150
#define cr_smux_ctl9         (*(volatile unsigned int *)cr_smux_ctl_adr9)
#define cr_err_ovr_val_adr   0x00320140
#define cr_err_ovr_val       (*(volatile unsigned int *)cr_err_ovr_val_adr)
#define cr_err_format_adr    0x00320144
#define cr_err_format        (*(volatile unsigned int *)cr_err_format_adr)
#define cr_sleep_slots_adr   0x00320148
#define cr_sleep_slots       (*(volatile unsigned int *)cr_sleep_slots_adr)
#define cr_dap_clk_sel_adr   0x0032014c
#define cr_dap_clk_sel       (*(volatile unsigned int *)cr_dap_clk_sel_adr)
#define cr_jtmode_status_adr 0x00320180
#define cr_jtmode_status     (*(volatile unsigned int *)cr_jtmode_status_adr)
#define cr_aes_clk_cfg_adr   0x0032016c
#define cr_aes_clk_cfg       (*(volatile unsigned int *)cr_aes_clk_cfg_adr)
#define cr_pad_fcn_ctl_adr4  0x00320300
#define cr_pad_fcn_ctl4      (*(volatile unsigned int *)cr_pad_fcn_ctl_adr4)
#define cr_cm3_bmx_bug_fix_adr 0x0032103c
#define cr_cm3_bmx_bug_fix   (*(volatile unsigned int *)cr_cm3_bmx_bug_fix_adr)
#define dc_ptu_addr_adr      0x00360000
#define dc_ptu_addr          (*(volatile unsigned int *)dc_ptu_addr_adr)
#define dp_uart2_data_adr    0x00360004
#define dp_uart2_data        (*(volatile unsigned int *)dp_uart2_data_adr)
#define dc_ptu_rw_adr        0x00360008
#define dc_ptu_rw            (*(volatile unsigned int *)dc_ptu_rw_adr)
#define dc_ptu_status_adr    0x0036000c
#define dc_ptu_status        (*(volatile unsigned int *)dc_ptu_status_adr)
#define PTU_available02_adr  0x00360010
#define PTU_available02      (*(volatile unsigned int *)PTU_available02_adr)
#define dc_ptu_hc_sel_adr    0x00360014
#define dc_ptu_hc_sel        (*(volatile unsigned int *)dc_ptu_hc_sel_adr)
#define dc_ptu_sel_ctl_adr   0x00360018
#define dc_ptu_sel_ctl       (*(volatile unsigned int *)dc_ptu_sel_ctl_adr)
#define dp_uart_data_adr     0x0036001c
#define dp_uart_data         (*(volatile unsigned int *)dp_uart_data_adr)
#define dp_ep0_data_adr      0x00360020
#define dp_ep0_data          (*(volatile unsigned int *)dp_ep0_data_adr)
#define dp_ep1_data_adr      0x00360024
#define dp_ep1_data          (*(volatile unsigned int *)dp_ep1_data_adr)
#define dp_ep2_data_adr      0x00360028
#define dp_ep2_data          (*(volatile unsigned int *)dp_ep2_data_adr)
#define dp_ep3_data_adr      0x0036002c
#define dp_ep3_data          (*(volatile unsigned int *)dp_ep3_data_adr)
#define dp_ep4_data_adr      0x00360030
#define dp_ep4_data          (*(volatile unsigned int *)dp_ep4_data_adr)
#define dp_ep_trg_adr        0x00360034
#define dp_ep_trg            (*(volatile unsigned int *)dp_ep_trg_adr)
#define dp_ep0_pklen_adr     0x00360038
#define dp_ep0_pklen         (*(volatile unsigned int *)dp_ep0_pklen_adr)
#define dp_ep2_pklen_adr     0x0036003c
#define dp_ep2_pklen         (*(volatile unsigned int *)dp_ep2_pklen_adr)
#define dp_ep3_pklen_adr     0x00360040
#define dp_ep3_pklen         (*(volatile unsigned int *)dp_ep3_pklen_adr)
#define dp_ep4_pklen_adr     0x00360044
#define dp_ep4_pklen         (*(volatile unsigned int *)dp_ep4_pklen_adr)
#define dp_ep2_dp_adr        0x00360048
#define dp_ep2_dp            (*(volatile unsigned int *)dp_ep2_dp_adr)
#define dp2_ep0_data_adr     0x0036004c
#define dp2_ep0_data         (*(volatile unsigned int *)dp2_ep0_data_adr)
#define dp2_ep1_data_adr     0x00360050
#define dp2_ep1_data         (*(volatile unsigned int *)dp2_ep1_data_adr)
#define dp2_ep_trg_adr       0x00360054
#define dp2_ep_trg           (*(volatile unsigned int *)dp2_ep_trg_adr)
#define dp2_ep0_pklen_adr    0x00360058
#define dp2_ep0_pklen        (*(volatile unsigned int *)dp2_ep0_pklen_adr)
#define dp3_ep0_data_adr     0x0036005c
#define dp3_ep0_data         (*(volatile unsigned int *)dp3_ep0_data_adr)
#define dp3_ep1_data_adr     0x00360060
#define dp3_ep1_data         (*(volatile unsigned int *)dp3_ep1_data_adr)
#define dp3_ep_trg_adr       0x00360064
#define dp3_ep_trg           (*(volatile unsigned int *)dp3_ep_trg_adr)
#define dp3_ep0_pklen_adr    0x00360068
#define dp3_ep0_pklen        (*(volatile unsigned int *)dp3_ep0_pklen_adr)
#define dc_ptu_hubcur_adr    0x0036006c
#define dc_ptu_hubcur        (*(volatile unsigned int *)dc_ptu_hubcur_adr)
#define srptu_base           0x00360074
#define sr_ptu_status_adr0   0x00360074
#define sr_ptu_status0       (*(volatile unsigned int *)sr_ptu_status_adr0)
#define sr_ptu_status_adr1   0x00360078
#define sr_ptu_status1       (*(volatile unsigned int *)sr_ptu_status_adr1)
#define sr_ptu_status_adr2   0x0036007c
#define sr_ptu_status2       (*(volatile unsigned int *)sr_ptu_status_adr2)
#define sr_ptu_status_adr3   0x00360080
#define sr_ptu_status3       (*(volatile unsigned int *)sr_ptu_status_adr3)
#define sr_ptu_status_adr4   0x00360084
#define sr_ptu_status4       (*(volatile unsigned int *)sr_ptu_status_adr4)
#define sr_ptu_status_adr5   0x00360088
#define sr_ptu_status5       (*(volatile unsigned int *)sr_ptu_status_adr5)
#define sr_ptu_status_adr6   0x0036008c
#define sr_ptu_status6       (*(volatile unsigned int *)sr_ptu_status_adr6)
#define sr_ptu_status_adr7   0x00360090
#define sr_ptu_status7       (*(volatile unsigned int *)sr_ptu_status_adr7)
#define sr_ptu_status_adr8   0x00360094
#define sr_ptu_status8       (*(volatile unsigned int *)sr_ptu_status_adr8)
#define sr_ptu_en_adr0       0x00360098
#define sr_ptu_en0           (*(volatile unsigned int *)sr_ptu_en_adr0)
#define sr_ptu_en_adr1       0x0036009c
#define sr_ptu_en1           (*(volatile unsigned int *)sr_ptu_en_adr1)
#define sr_ptu_en_adr2       0x003600a0
#define sr_ptu_en2           (*(volatile unsigned int *)sr_ptu_en_adr2)
#define sr_ptu_en_adr3       0x003600a4
#define sr_ptu_en3           (*(volatile unsigned int *)sr_ptu_en_adr3)
#define sr_ptu_en_adr4       0x003600a8
#define sr_ptu_en4           (*(volatile unsigned int *)sr_ptu_en_adr4)
#define sr_ptu_en_adr5       0x003600ac
#define sr_ptu_en5           (*(volatile unsigned int *)sr_ptu_en_adr5)
#define sr_ptu_en_adr6       0x003600b0
#define sr_ptu_en6           (*(volatile unsigned int *)sr_ptu_en_adr6)
#define sr_ptu_en_adr7       0x003600b4
#define sr_ptu_en7           (*(volatile unsigned int *)sr_ptu_en_adr7)
#define sr_ptu_en_adr8       0x003600b8
#define sr_ptu_en8           (*(volatile unsigned int *)sr_ptu_en_adr8)
#define sr_ptu_dis_adr0      0x003600bc
#define sr_ptu_dis0          (*(volatile unsigned int *)sr_ptu_dis_adr0)
#define sr_ptu_dis_adr1      0x003600c0
#define sr_ptu_dis1          (*(volatile unsigned int *)sr_ptu_dis_adr1)
#define sr_ptu_dis_adr2      0x003600c4
#define sr_ptu_dis2          (*(volatile unsigned int *)sr_ptu_dis_adr2)
#define sr_ptu_dis_adr3      0x003600c8
#define sr_ptu_dis3          (*(volatile unsigned int *)sr_ptu_dis_adr3)
#define sr_ptu_dis_adr4      0x003600cc
#define sr_ptu_dis4          (*(volatile unsigned int *)sr_ptu_dis_adr4)
#define sr_ptu_test_adr0     0x003600d0
#define sr_ptu_test0         (*(volatile unsigned int *)sr_ptu_test_adr0)
#define sr_ptu_test_adr1     0x003600d4
#define sr_ptu_test1         (*(volatile unsigned int *)sr_ptu_test_adr1)
#define sr_ptu_test_adr2     0x003600d8
#define sr_ptu_test2         (*(volatile unsigned int *)sr_ptu_test_adr2)
#define sr_ptu_test_adr3     0x003600dc
#define sr_ptu_test3         (*(volatile unsigned int *)sr_ptu_test_adr3)
#define sr_ptu_test_adr4     0x003600e0
#define sr_ptu_test4         (*(volatile unsigned int *)sr_ptu_test_adr4)
#define sr_ptu_inv_adr       0x003600e4
#define sr_ptu_inv           (*(volatile unsigned int *)sr_ptu_inv_adr)
#define dc_ptu_pwron2good_adr 0x003600e8
#define dc_ptu_pwron2good    (*(volatile unsigned int *)dc_ptu_pwron2good_adr)
#define sr_ptu_status_adr9   0x003600ec
#define sr_ptu_status9       (*(volatile unsigned int *)sr_ptu_status_adr9)
#define sr_ptu_en_adr9       0x003600f0
#define sr_ptu_en9           (*(volatile unsigned int *)sr_ptu_en_adr9)
#define sr_ptu_dis_adr9      0x003600f4
#define sr_ptu_dis9          (*(volatile unsigned int *)sr_ptu_dis_adr9)
#define sr_ptu_test_adr9     0x003600f8
#define sr_ptu_test9         (*(volatile unsigned int *)sr_ptu_test_adr9)
#define dp_ep2o_hci_pkglen_adr 0x003600fc
#define dp_ep2o_hci_pkglen   (*(volatile unsigned int *)dp_ep2o_hci_pkglen_adr)
#define WLAN_status_adr      0x00360100
#define WLAN_status          (*(volatile unsigned int *)WLAN_status_adr)
#define WLAN_ctrl_adr        0x00360104
#define WLAN_ctrl            (*(volatile unsigned int *)WLAN_ctrl_adr)
#define WLAN_dis_adr         0x00360108
#define WLAN_dis             (*(volatile unsigned int *)WLAN_dis_adr)
#define WLAN_level_mask_adr  0x0036010c
#define WLAN_level_mask      (*(volatile unsigned int *)WLAN_level_mask_adr)
#define WLAN_edge_mode_adr   0x00360110
#define WLAN_edge_mode       (*(volatile unsigned int *)WLAN_edge_mode_adr)
#define uart_base            0x00360400
#define uart2_base           0x00360480
#define iic_base             0x00360500
#define spi_base             0x00360600
#define aa_base              0x00360700
#define db_uart_base         0x00360800
#define rtc_base             0x00360900
#define pcm_base             0x00360e00
#define dsdio_base           0x00361000
#define spi2_base            0x00361000
#define hub_base             0x00362000
#define usb0_usb_base        0x00363000
#define usb1_usb_base        0x00363400
#define usb2_usb_base        0x00363800
#define dc_usb_cfg_off       0x00000000
#define dc_interfaceval_off  0x00000004
#define dc_maxpower_off      0x00000008
#define dc_cfgnum_off        0x0000000c
#define dc_bmrt_off          0x00000010
#define dc_brequest_off      0x00000014
#define dc_wvaluel_off       0x00000018
#define dc_wvalueh_off       0x0000001c
#define dc_windexl_off       0x00000020
#define dc_windexh_off       0x00000024
#define dc_wlengthl_off      0x00000028
#define dc_wlengthh_off      0x0000002c
#define usb0_dc_usb_cfg_adr  0x00363000
#define usb0_dc_usb_cfg      (*(volatile unsigned int *)usb0_dc_usb_cfg_adr)
#define usb0_dc_interfaceval_adr 0x00363004
#define usb0_dc_interfaceval (*(volatile unsigned int *)usb0_dc_interfaceval_adr)
#define usb0_dc_maxpower_adr 0x00363008
#define usb0_dc_maxpower     (*(volatile unsigned int *)usb0_dc_maxpower_adr)
#define usb0_dc_cfgnum_adr   0x0036300c
#define usb0_dc_cfgnum       (*(volatile unsigned int *)usb0_dc_cfgnum_adr)
#define usb0_dc_bmrt_adr     0x00363010
#define usb0_dc_bmrt         (*(volatile unsigned int *)usb0_dc_bmrt_adr)
#define usb0_dc_brequest_adr 0x00363014
#define usb0_dc_brequest     (*(volatile unsigned int *)usb0_dc_brequest_adr)
#define usb0_dc_wvaluel_adr  0x00363018
#define usb0_dc_wvaluel      (*(volatile unsigned int *)usb0_dc_wvaluel_adr)
#define usb0_dc_wvalueh_adr  0x0036301c
#define usb0_dc_wvalueh      (*(volatile unsigned int *)usb0_dc_wvalueh_adr)
#define usb0_dc_windexl_adr  0x00363020
#define usb0_dc_windexl      (*(volatile unsigned int *)usb0_dc_windexl_adr)
#define usb0_dc_windexh_adr  0x00363024
#define usb0_dc_windexh      (*(volatile unsigned int *)usb0_dc_windexh_adr)
#define usb0_dc_wlengthl_adr 0x00363028
#define usb0_dc_wlengthl     (*(volatile unsigned int *)usb0_dc_wlengthl_adr)
#define usb0_dc_wlengthh_adr 0x0036302c
#define usb0_dc_wlengthh     (*(volatile unsigned int *)usb0_dc_wlengthh_adr)
#define usb1_dc_usb_cfg_adr  0x00363400
#define usb1_dc_usb_cfg      (*(volatile unsigned int *)usb1_dc_usb_cfg_adr)
#define usb1_dc_interfaceval_adr 0x00363404
#define usb1_dc_interfaceval (*(volatile unsigned int *)usb1_dc_interfaceval_adr)
#define usb1_dc_maxpower_adr 0x00363408
#define usb1_dc_maxpower     (*(volatile unsigned int *)usb1_dc_maxpower_adr)
#define usb1_dc_cfgnum_adr   0x0036340c
#define usb1_dc_cfgnum       (*(volatile unsigned int *)usb1_dc_cfgnum_adr)
#define usb1_dc_bmrt_adr     0x00363410
#define usb1_dc_bmrt         (*(volatile unsigned int *)usb1_dc_bmrt_adr)
#define usb1_dc_brequest_adr 0x00363414
#define usb1_dc_brequest     (*(volatile unsigned int *)usb1_dc_brequest_adr)
#define usb1_dc_wvaluel_adr  0x00363418
#define usb1_dc_wvaluel      (*(volatile unsigned int *)usb1_dc_wvaluel_adr)
#define usb1_dc_wvalueh_adr  0x0036341c
#define usb1_dc_wvalueh      (*(volatile unsigned int *)usb1_dc_wvalueh_adr)
#define usb1_dc_windexl_adr  0x00363420
#define usb1_dc_windexl      (*(volatile unsigned int *)usb1_dc_windexl_adr)
#define usb1_dc_windexh_adr  0x00363424
#define usb1_dc_windexh      (*(volatile unsigned int *)usb1_dc_windexh_adr)
#define usb1_dc_wlengthl_adr 0x00363428
#define usb1_dc_wlengthl     (*(volatile unsigned int *)usb1_dc_wlengthl_adr)
#define usb1_dc_wlengthh_adr 0x0036342c
#define usb1_dc_wlengthh     (*(volatile unsigned int *)usb1_dc_wlengthh_adr)
#define usb2_dc_usb_cfg_adr  0x00363800
#define usb2_dc_usb_cfg      (*(volatile unsigned int *)usb2_dc_usb_cfg_adr)
#define usb2_dc_interfaceval_adr 0x00363804
#define usb2_dc_interfaceval (*(volatile unsigned int *)usb2_dc_interfaceval_adr)
#define usb2_dc_maxpower_adr 0x00363808
#define usb2_dc_maxpower     (*(volatile unsigned int *)usb2_dc_maxpower_adr)
#define usb2_dc_cfgnum_adr   0x0036380c
#define usb2_dc_cfgnum       (*(volatile unsigned int *)usb2_dc_cfgnum_adr)
#define usb2_dc_bmrt_adr     0x00363810
#define usb2_dc_bmrt         (*(volatile unsigned int *)usb2_dc_bmrt_adr)
#define usb2_dc_brequest_adr 0x00363814
#define usb2_dc_brequest     (*(volatile unsigned int *)usb2_dc_brequest_adr)
#define usb2_dc_wvaluel_adr  0x00363818
#define usb2_dc_wvaluel      (*(volatile unsigned int *)usb2_dc_wvaluel_adr)
#define usb2_dc_wvalueh_adr  0x0036381c
#define usb2_dc_wvalueh      (*(volatile unsigned int *)usb2_dc_wvalueh_adr)
#define usb2_dc_windexl_adr  0x00363820
#define usb2_dc_windexl      (*(volatile unsigned int *)usb2_dc_windexl_adr)
#define usb2_dc_windexh_adr  0x00363824
#define usb2_dc_windexh      (*(volatile unsigned int *)usb2_dc_windexh_adr)
#define usb2_dc_wlengthl_adr 0x00363828
#define usb2_dc_wlengthl     (*(volatile unsigned int *)usb2_dc_wlengthl_adr)
#define usb2_dc_wlengthh_adr 0x0036382c
#define usb2_dc_wlengthh     (*(volatile unsigned int *)usb2_dc_wlengthh_adr)
#define dc_ptu_uart_dhbr_adr 0x00360400
#define dc_ptu_uart_dhbr     (*(volatile unsigned int *)dc_ptu_uart_dhbr_adr)
#define dc_ptu_uart_dlbr_adr 0x00360404
#define dc_ptu_uart_dlbr     (*(volatile unsigned int *)dc_ptu_uart_dlbr_adr)
#define dc_ptu_uart_ab0_adr  0x00360408
#define dc_ptu_uart_ab0      (*(volatile unsigned int *)dc_ptu_uart_ab0_adr)
#define dc_ptu_uart_fcr_adr  0x00360410
#define dc_ptu_uart_fcr      (*(volatile unsigned int *)dc_ptu_uart_fcr_adr)
#define dc_ptu_uart_ab1_adr  0x00360414
#define dc_ptu_uart_ab1      (*(volatile unsigned int *)dc_ptu_uart_ab1_adr)
#define dc_ptu_uart_lcr_adr  0x0036041c
#define dc_ptu_uart_lcr      (*(volatile unsigned int *)dc_ptu_uart_lcr_adr)
#define dc_ptu_uart_mcr_adr  0x00360420
#define dc_ptu_uart_mcr      (*(volatile unsigned int *)dc_ptu_uart_mcr_adr)
#define dc_ptu_uart_lsr_adr  0x00360424
#define dc_ptu_uart_lsr      (*(volatile unsigned int *)dc_ptu_uart_lsr_adr)
#define dc_ptu_uart_msr_adr  0x00360428
#define dc_ptu_uart_msr      (*(volatile unsigned int *)dc_ptu_uart_msr_adr)
#define dc_ptu_uart_rfl_adr  0x0036042c
#define dc_ptu_uart_rfl      (*(volatile unsigned int *)dc_ptu_uart_rfl_adr)
#define dc_ptu_uart_tfl_adr  0x00360430
#define dc_ptu_uart_tfl      (*(volatile unsigned int *)dc_ptu_uart_tfl_adr)
#define dc_ptu_uart_rfc_adr  0x00360434
#define dc_ptu_uart_rfc      (*(volatile unsigned int *)dc_ptu_uart_rfc_adr)
#define dc_ptu_uart_esc_adr  0x00360438
#define dc_ptu_uart_esc      (*(volatile unsigned int *)dc_ptu_uart_esc_adr)
#define dc_ptu_uart_abr_adr  0x0036043c
#define dc_ptu_uart_abr      (*(volatile unsigned int *)dc_ptu_uart_abr_adr)
#define dc_free32_adr        0x00360440
#define dc_free32            (*(volatile unsigned int *)dc_free32_adr)
#define dc_free32_ctl_adr    0x00360444
#define dc_free32_ctl        (*(volatile unsigned int *)dc_free32_ctl_adr)
#define uart_ho_pklen_adr    0x00360448
#define uart_ho_pklen        (*(volatile unsigned int *)uart_ho_pklen_adr)
#define uart_hi_pklen_adr    0x0036044c
#define uart_hi_pklen        (*(volatile unsigned int *)uart_hi_pklen_adr)
#define uart_ho_dma_ctl_adr  0x00360450
#define uart_ho_dma_ctl      (*(volatile unsigned int *)uart_ho_dma_ctl_adr)
#define uart_hi_dma_ctl_adr  0x00360454
#define uart_hi_dma_ctl      (*(volatile unsigned int *)uart_hi_dma_ctl_adr)
#define uart_ho_bsize_adr    0x00360458
#define uart_ho_bsize        (*(volatile unsigned int *)uart_ho_bsize_adr)
#define uart_hi_bsize_adr    0x0036045c
#define uart_hi_bsize        (*(volatile unsigned int *)uart_hi_bsize_adr)
#define fpga_id_adr          0x00360460
#define fpga_id              (*(volatile unsigned int *)fpga_id_adr)
#define dc_ptu_uart2_dhbr_adr 0x00360480
#define dc_ptu_uart2_dhbr    (*(volatile unsigned int *)dc_ptu_uart2_dhbr_adr)
#define dc_ptu_uart2_dlbr_adr 0x00360484
#define dc_ptu_uart2_dlbr    (*(volatile unsigned int *)dc_ptu_uart2_dlbr_adr)
#define dc_ptu_uart2_ab0_adr 0x00360488
#define dc_ptu_uart2_ab0     (*(volatile unsigned int *)dc_ptu_uart2_ab0_adr)
#define dc_ptu_uart2_fcr_adr 0x00360490
#define dc_ptu_uart2_fcr     (*(volatile unsigned int *)dc_ptu_uart2_fcr_adr)
#define dc_ptu_uart2_ab1_adr 0x00360494
#define dc_ptu_uart2_ab1     (*(volatile unsigned int *)dc_ptu_uart2_ab1_adr)
#define dc_ptu_uart2_lcr_adr 0x0036049c
#define dc_ptu_uart2_lcr     (*(volatile unsigned int *)dc_ptu_uart2_lcr_adr)
#define dc_ptu_uart2_mcr_adr 0x003604a0
#define dc_ptu_uart2_mcr     (*(volatile unsigned int *)dc_ptu_uart2_mcr_adr)
#define dc_ptu_uart2_lsr_adr 0x003604a4
#define dc_ptu_uart2_lsr     (*(volatile unsigned int *)dc_ptu_uart2_lsr_adr)
#define dc_ptu_uart2_msr_adr 0x003604a8
#define dc_ptu_uart2_msr     (*(volatile unsigned int *)dc_ptu_uart2_msr_adr)
#define dc_ptu_uart2_rfl_adr 0x003604ac
#define dc_ptu_uart2_rfl     (*(volatile unsigned int *)dc_ptu_uart2_rfl_adr)
#define dc_ptu_uart2_tfl_adr 0x003604b0
#define dc_ptu_uart2_tfl     (*(volatile unsigned int *)dc_ptu_uart2_tfl_adr)
#define dc_ptu_uart2_rfc_adr 0x003604b4
#define dc_ptu_uart2_rfc     (*(volatile unsigned int *)dc_ptu_uart2_rfc_adr)
#define dc_ptu_uart2_esc_adr 0x003604b8
#define dc_ptu_uart2_esc     (*(volatile unsigned int *)dc_ptu_uart2_esc_adr)
#define dc_ptu_uart2_abr_adr 0x003604bc
#define dc_ptu_uart2_abr     (*(volatile unsigned int *)dc_ptu_uart2_abr_adr)
#define DC_avail01_adr       0x003604c0
#define DC_avail01           (*(volatile unsigned int *)DC_avail01_adr)
#define DC_avail02_adr       0x003604c4
#define DC_avail02           (*(volatile unsigned int *)DC_avail02_adr)
#define uart2_ho_pklen_adr   0x003604c8
#define uart2_ho_pklen       (*(volatile unsigned int *)uart2_ho_pklen_adr)
#define uart2_hi_pklen_adr   0x003604cc
#define uart2_hi_pklen       (*(volatile unsigned int *)uart2_hi_pklen_adr)
#define uart2_ho_dma_ctl_adr 0x003604d0
#define uart2_ho_dma_ctl     (*(volatile unsigned int *)uart2_ho_dma_ctl_adr)
#define uart2_hi_dma_ctl_adr 0x003604d4
#define uart2_hi_dma_ctl     (*(volatile unsigned int *)uart2_hi_dma_ctl_adr)
#define uart2_ho_bsize_adr   0x003604d8
#define uart2_ho_bsize       (*(volatile unsigned int *)uart2_ho_bsize_adr)
#define uart2_hi_bsize_adr   0x003604dc
#define uart2_hi_bsize       (*(volatile unsigned int *)uart2_hi_bsize_adr)
#define iic_caddr_wctl_adr   0x00360500
#define iic_caddr_wctl       (*(volatile unsigned int *)iic_caddr_wctl_adr)
#define iic_wdata_adr0       0x00360504
#define iic_wdata0           (*(volatile unsigned int *)iic_wdata_adr0)
#define iic_wdata_adr1       0x00360508
#define iic_wdata1           (*(volatile unsigned int *)iic_wdata_adr1)
#define iic_wdata_adr2       0x0036050c
#define iic_wdata2           (*(volatile unsigned int *)iic_wdata_adr2)
#define iic_wdata_adr3       0x00360510
#define iic_wdata3           (*(volatile unsigned int *)iic_wdata_adr3)
#define iic_wdata_adr4       0x00360514
#define iic_wdata4           (*(volatile unsigned int *)iic_wdata_adr4)
#define iic_wdata_adr5       0x00360518
#define iic_wdata5           (*(volatile unsigned int *)iic_wdata_adr5)
#define iic_wdata_adr6       0x0036051c
#define iic_wdata6           (*(volatile unsigned int *)iic_wdata_adr6)
#define iic_wdata_adr7       0x00360520
#define iic_wdata7           (*(volatile unsigned int *)iic_wdata_adr7)
#define iic_wdata_adr8       0x00360524
#define iic_wdata8           (*(volatile unsigned int *)iic_wdata_adr8)
#define iic_wdata_adr9       0x00360528
#define iic_wdata9           (*(volatile unsigned int *)iic_wdata_adr9)
#define iic_wdata_adr10      0x0036052c
#define iic_wdata10          (*(volatile unsigned int *)iic_wdata_adr10)
#define iic_wdata_adr11      0x00360530
#define iic_wdata11          (*(volatile unsigned int *)iic_wdata_adr11)
#define iic_wdata_adr12      0x00360534
#define iic_wdata12          (*(volatile unsigned int *)iic_wdata_adr12)
#define iic_wdata_adr13      0x00360538
#define iic_wdata13          (*(volatile unsigned int *)iic_wdata_adr13)
#define iic_wdata_adr14      0x0036053c
#define iic_wdata14          (*(volatile unsigned int *)iic_wdata_adr14)
#define iic_wdata_adr15      0x00360540
#define iic_wdata15          (*(volatile unsigned int *)iic_wdata_adr15)
#define iic_byte_cnt_adr     0x00360544
#define iic_byte_cnt         (*(volatile unsigned int *)iic_byte_cnt_adr)
#define iic_ctl_adr          0x00360548
#define iic_ctl              (*(volatile unsigned int *)iic_ctl_adr)
#define iic_rd_en_int_adr    0x0036054c
#define iic_rd_en_int        (*(volatile unsigned int *)iic_rd_en_int_adr)
#define iic_rdata_adr0       0x00360550
#define iic_rdata0           (*(volatile unsigned int *)iic_rdata_adr0)
#define iic_rdata_adr1       0x00360554
#define iic_rdata1           (*(volatile unsigned int *)iic_rdata_adr1)
#define iic_rdata_adr2       0x00360558
#define iic_rdata2           (*(volatile unsigned int *)iic_rdata_adr2)
#define iic_rdata_adr3       0x0036055c
#define iic_rdata3           (*(volatile unsigned int *)iic_rdata_adr3)
#define iic_rdata_adr4       0x00360560
#define iic_rdata4           (*(volatile unsigned int *)iic_rdata_adr4)
#define iic_rdata_adr5       0x00360564
#define iic_rdata5           (*(volatile unsigned int *)iic_rdata_adr5)
#define iic_rdata_adr6       0x00360568
#define iic_rdata6           (*(volatile unsigned int *)iic_rdata_adr6)
#define iic_rdata_adr7       0x0036056c
#define iic_rdata7           (*(volatile unsigned int *)iic_rdata_adr7)
#define iic_rdata_adr8       0x00360570
#define iic_rdata8           (*(volatile unsigned int *)iic_rdata_adr8)
#define iic_rdata_adr9       0x00360574
#define iic_rdata9           (*(volatile unsigned int *)iic_rdata_adr9)
#define iic_rdata_adr10      0x00360578
#define iic_rdata10          (*(volatile unsigned int *)iic_rdata_adr10)
#define iic_rdata_adr11      0x0036057c
#define iic_rdata11          (*(volatile unsigned int *)iic_rdata_adr11)
#define iic_rdata_adr12      0x00360580
#define iic_rdata12          (*(volatile unsigned int *)iic_rdata_adr12)
#define iic_rdata_adr13      0x00360584
#define iic_rdata13          (*(volatile unsigned int *)iic_rdata_adr13)
#define iic_rdata_adr14      0x00360588
#define iic_rdata14          (*(volatile unsigned int *)iic_rdata_adr14)
#define iic_rdata_adr15      0x0036058c
#define iic_rdata15          (*(volatile unsigned int *)iic_rdata_adr15)
#define iic_scl_maxcnt_adr   0x00360590
#define iic_scl_maxcnt       (*(volatile unsigned int *)iic_scl_maxcnt_adr)
#define iic_scl_div_adr      0x00360594
#define iic_scl_div          (*(volatile unsigned int *)iic_scl_div_adr)
#define db_uart_dhbr_adr     0x00360800
#define db_uart_dhbr         (*(volatile unsigned int *)db_uart_dhbr_adr)
#define db_uart_dlbr_adr     0x00360804
#define db_uart_dlbr         (*(volatile unsigned int *)db_uart_dlbr_adr)
#define db_uart_fcr_adr      0x00360808
#define db_uart_fcr          (*(volatile unsigned int *)db_uart_fcr_adr)
#define db_uart_lcr_adr      0x0036080c
#define db_uart_lcr          (*(volatile unsigned int *)db_uart_lcr_adr)
#define db_uart_mcr_adr      0x00360810
#define db_uart_mcr          (*(volatile unsigned int *)db_uart_mcr_adr)
#define db_uart_lsr_adr      0x00360814
#define db_uart_lsr          (*(volatile unsigned int *)db_uart_lsr_adr)
#define db_uart_msr_adr      0x00360818
#define db_uart_msr          (*(volatile unsigned int *)db_uart_msr_adr)
#define db_uart_data_adr     0x0036081c
#define db_uart_data         (*(volatile unsigned int *)db_uart_data_adr)
#define db_uart_snap_adr     0x00360820
#define db_uart_snap         (*(volatile unsigned int *)db_uart_snap_adr)
#define db_uart_txw_adr      0x00360824
#define db_uart_txw          (*(volatile unsigned int *)db_uart_txw_adr)
#define db_uart_rxw_adr      0x00360828
#define db_uart_rxw          (*(volatile unsigned int *)db_uart_rxw_adr)
#define rtc_ucodem_start_adr 0x00360900
#define rtc_ucodem_start     (*(volatile unsigned int *)rtc_ucodem_start_adr)
#define rtc_ucodem_stop_adr  0x00360aff
#define rtc_ucodem_stop      (*(volatile unsigned int *)rtc_ucodem_stop_adr)
#define rtc_tapm_start_adr   0x00360b00
#define rtc_tapm_start       (*(volatile unsigned int *)rtc_tapm_start_adr)
#define rtc_tapm_stop_adr    0x00360bff
#define rtc_tapm_stop        (*(volatile unsigned int *)rtc_tapm_stop_adr)
#define rtc_coeffm_start_adr 0x00360c00
#define rtc_coeffm_start     (*(volatile unsigned int *)rtc_coeffm_start_adr)
#define rtc_coeffm_stop_adr  0x00360cff
#define rtc_coeffm_stop      (*(volatile unsigned int *)rtc_coeffm_stop_adr)
#define rtc_integerd_adr     0x00360d00
#define rtc_integerd         (*(volatile unsigned int *)rtc_integerd_adr)
#define rtc_fractiond_adr    0x00360d04
#define rtc_fractiond        (*(volatile unsigned int *)rtc_fractiond_adr)
#define rtc_initial_pc_adr   0x00360d08
#define rtc_initial_pc       (*(volatile unsigned int *)rtc_initial_pc_adr)
#define rtc_config_adr       0x00360d0c
#define rtc_config           (*(volatile unsigned int *)rtc_config_adr)
#define pcm_control_adr      0x00360e00
#define pcm_control          (*(volatile unsigned int *)pcm_control_adr)
#define pcm_ch0_din_adr      0x00360e04
#define pcm_ch0_din          (*(volatile unsigned int *)pcm_ch0_din_adr)
#define pcm_ch0_dout_adr     0x00360e08
#define pcm_ch0_dout         (*(volatile unsigned int *)pcm_ch0_dout_adr)
#define pcm_ch1_din_adr      0x00360e0c
#define pcm_ch1_din          (*(volatile unsigned int *)pcm_ch1_din_adr)
#define pcm_ch1_dout_adr     0x00360e10
#define pcm_ch1_dout         (*(volatile unsigned int *)pcm_ch1_dout_adr)
#define pcm_ch2_din_adr      0x00360e14
#define pcm_ch2_din          (*(volatile unsigned int *)pcm_ch2_din_adr)
#define pcm_ch2_dout_adr     0x00360e18
#define pcm_ch2_dout         (*(volatile unsigned int *)pcm_ch2_dout_adr)
#define pcm_waterline_adr    0x00360e1c
#define pcm_waterline        (*(volatile unsigned int *)pcm_waterline_adr)
#define pcm_fifo_rst_adr     0x00360e20
#define pcm_fifo_rst         (*(volatile unsigned int *)pcm_fifo_rst_adr)
#define pcm_fifo_stat_adr    0x00360e24
#define pcm_fifo_stat        (*(volatile unsigned int *)pcm_fifo_stat_adr)
#define pcm_misc_ctrl_adr    0x00360e28
#define pcm_misc_ctrl        (*(volatile unsigned int *)pcm_misc_ctrl_adr)
#define pcm_control_2_adr    0x00360e2c
#define pcm_control_2        (*(volatile unsigned int *)pcm_control_2_adr)
#define pcm_waterlineo_adr   0x00360e30
#define pcm_waterlineo       (*(volatile unsigned int *)pcm_waterlineo_adr)
#define hub_std_descriptor_base 0x00362000
#define hub_dev_desc_adr     0x00362000
#define hub_dev_desc         (*(volatile unsigned int *)hub_dev_desc_adr)
#define hub_stringbuf2_adr   0x00362808
#define hub_stringbuf2       (*(volatile unsigned int *)hub_stringbuf2_adr)
#define hub_cfgram_test_adr  0x0036280c
#define hub_cfgram_test      (*(volatile unsigned int *)hub_cfgram_test_adr)
#define dmacintstat_adr      0x00300000
#define dmacintstat          (*(volatile unsigned int *)dmacintstat_adr)
#define dmacinttcstat_adr    0x00300004
#define dmacinttcstat        (*(volatile unsigned int *)dmacinttcstat_adr)
#define dmacinttcclr_adr     0x00300008
#define dmacinttcclr         (*(volatile unsigned int *)dmacinttcclr_adr)
#define dmacinterrstat_adr   0x0030000c
#define dmacinterrstat       (*(volatile unsigned int *)dmacinterrstat_adr)
#define dmacinterrclr_adr    0x00300010
#define dmacinterrclr        (*(volatile unsigned int *)dmacinterrclr_adr)
#define dmacrawintc_adr      0x00300014
#define dmacrawintc          (*(volatile unsigned int *)dmacrawintc_adr)
#define dmacrawinterr_adr    0x00300018
#define dmacrawinterr        (*(volatile unsigned int *)dmacrawinterr_adr)
#define dmacenbldchns_adr    0x0030001c
#define dmacenbldchns        (*(volatile unsigned int *)dmacenbldchns_adr)
#define dmacsoftbreq_adr     0x00300020
#define dmacsoftbreq         (*(volatile unsigned int *)dmacsoftbreq_adr)
#define dmacsoftsreq_adr     0x00300024
#define dmacsoftsreq         (*(volatile unsigned int *)dmacsoftsreq_adr)
#define dmacsoftlbreq_adr    0x00300028
#define dmacsoftlbreq        (*(volatile unsigned int *)dmacsoftlbreq_adr)
#define dmacsoftlsreq_adr    0x0030002c
#define dmacsoftlsreq        (*(volatile unsigned int *)dmacsoftlsreq_adr)
#define dmacconfig_adr       0x00300030
#define dmacconfig           (*(volatile unsigned int *)dmacconfig_adr)
#define dmacsync_adr         0x00300034
#define dmacsync             (*(volatile unsigned int *)dmacsync_adr)
#define dmacc0srcaddr_adr    0x00300100
#define dmacc0srcaddr        (*(volatile unsigned int *)dmacc0srcaddr_adr)
#define dmacc0destaddr_adr   0x00300104
#define dmacc0destaddr       (*(volatile unsigned int *)dmacc0destaddr_adr)
#define dmacc0llireg_adr     0x00300108
#define dmacc0llireg         (*(volatile unsigned int *)dmacc0llireg_adr)
#define dmacc0control_adr    0x0030010c
#define dmacc0control        (*(volatile unsigned int *)dmacc0control_adr)
#define dmacc0config_adr     0x00300110
#define dmacc0config         (*(volatile unsigned int *)dmacc0config_adr)
#define dmacc1srcaddr_adr    0x00300120
#define dmacc1srcaddr        (*(volatile unsigned int *)dmacc1srcaddr_adr)
#define dmacc1destaddr_adr   0x00300124
#define dmacc1destaddr       (*(volatile unsigned int *)dmacc1destaddr_adr)
#define dmacc1llireg_adr     0x00300128
#define dmacc1llireg         (*(volatile unsigned int *)dmacc1llireg_adr)
#define dmacc1control_adr    0x0030012c
#define dmacc1control        (*(volatile unsigned int *)dmacc1control_adr)
#define dmacc1config_adr     0x00300130
#define dmacc1config         (*(volatile unsigned int *)dmacc1config_adr)
#define dmacc2srcaddr_adr    0x00300140
#define dmacc2srcaddr        (*(volatile unsigned int *)dmacc2srcaddr_adr)
#define dmacc2destaddr_adr   0x00300144
#define dmacc2destaddr       (*(volatile unsigned int *)dmacc2destaddr_adr)
#define dmacc2llireg_adr     0x00300148
#define dmacc2llireg         (*(volatile unsigned int *)dmacc2llireg_adr)
#define dmacc2control_adr    0x0030014c
#define dmacc2control        (*(volatile unsigned int *)dmacc2control_adr)
#define dmacc2config_adr     0x00300150
#define dmacc2config         (*(volatile unsigned int *)dmacc2config_adr)
#define dmacc3srcaddr_adr    0x00300160
#define dmacc3srcaddr        (*(volatile unsigned int *)dmacc3srcaddr_adr)
#define dmacc3destaddr_adr   0x00300164
#define dmacc3destaddr       (*(volatile unsigned int *)dmacc3destaddr_adr)
#define dmacc3llireg_adr     0x00300168
#define dmacc3llireg         (*(volatile unsigned int *)dmacc3llireg_adr)
#define dmacc3control_adr    0x0030016c
#define dmacc3control        (*(volatile unsigned int *)dmacc3control_adr)
#define dmacc3config_adr     0x00300170
#define dmacc3config         (*(volatile unsigned int *)dmacc3config_adr)
#define dmacc4srcaddr_adr    0x00300180
#define dmacc4srcaddr        (*(volatile unsigned int *)dmacc4srcaddr_adr)
#define dmacc4destaddr_adr   0x00300184
#define dmacc4destaddr       (*(volatile unsigned int *)dmacc4destaddr_adr)
#define dmacc4llireg_adr     0x00300188
#define dmacc4llireg         (*(volatile unsigned int *)dmacc4llireg_adr)
#define dmacc4control_adr    0x0030018c
#define dmacc4control        (*(volatile unsigned int *)dmacc4control_adr)
#define dmacc4config_adr     0x00300190
#define dmacc4config         (*(volatile unsigned int *)dmacc4config_adr)
#define dmacc5srcaddr_adr    0x003001a0
#define dmacc5srcaddr        (*(volatile unsigned int *)dmacc5srcaddr_adr)
#define dmacc5destaddr_adr   0x003001a4
#define dmacc5destaddr       (*(volatile unsigned int *)dmacc5destaddr_adr)
#define dmacc5llireg_adr     0x003001a8
#define dmacc5llireg         (*(volatile unsigned int *)dmacc5llireg_adr)
#define dmacc5control_adr    0x003001ac
#define dmacc5control        (*(volatile unsigned int *)dmacc5control_adr)
#define dmacc5config_adr     0x003001b0
#define dmacc5config         (*(volatile unsigned int *)dmacc5config_adr)
#define dmacc6srcaddr_adr    0x003001c0
#define dmacc6srcaddr        (*(volatile unsigned int *)dmacc6srcaddr_adr)
#define dmacc6destaddr_adr   0x003001c4
#define dmacc6destaddr       (*(volatile unsigned int *)dmacc6destaddr_adr)
#define dmacc6llireg_adr     0x003001c8
#define dmacc6llireg         (*(volatile unsigned int *)dmacc6llireg_adr)
#define dmacc6control_adr    0x003001cc
#define dmacc6control        (*(volatile unsigned int *)dmacc6control_adr)
#define dmacc6config_adr     0x003001d0
#define dmacc6config         (*(volatile unsigned int *)dmacc6config_adr)
#define dmacc7srcaddr_adr    0x003001e0
#define dmacc7srcaddr        (*(volatile unsigned int *)dmacc7srcaddr_adr)
#define dmacc7destaddr_adr   0x003001e4
#define dmacc7destaddr       (*(volatile unsigned int *)dmacc7destaddr_adr)
#define dmacc7llireg_adr     0x003001e8
#define dmacc7llireg         (*(volatile unsigned int *)dmacc7llireg_adr)
#define dmacc7control_adr    0x003001ec
#define dmacc7control        (*(volatile unsigned int *)dmacc7control_adr)
#define dmacc7config_adr     0x003001f0
#define dmacc7config         (*(volatile unsigned int *)dmacc7config_adr)
#define dmactcr_adr          0x00300500
#define dmactcr              (*(volatile unsigned int *)dmactcr_adr)
#define dmacitop1_adr        0x00300504
#define dmacitop1            (*(volatile unsigned int *)dmacitop1_adr)
#define dmacitop2_adr        0x00300508
#define dmacitop2            (*(volatile unsigned int *)dmacitop2_adr)
#define dmacitop3_adr        0x0030050c
#define dmacitop3            (*(volatile unsigned int *)dmacitop3_adr)
#define dmacperiphid0_adr    0x00300fe0
#define dmacperiphid0        (*(volatile unsigned int *)dmacperiphid0_adr)
#define dmacperiphid1_adr    0x00300fe4
#define dmacperiphid1        (*(volatile unsigned int *)dmacperiphid1_adr)
#define dmacperiphid2_adr    0x00300fe8
#define dmacperiphid2        (*(volatile unsigned int *)dmacperiphid2_adr)
#define dmacperiphid3_adr    0x00300fec
#define dmacperiphid3        (*(volatile unsigned int *)dmacperiphid3_adr)
#define dmacpcellid0_adr     0x00300ff0
#define dmacpcellid0         (*(volatile unsigned int *)dmacpcellid0_adr)
#define dmacpcellid1_adr     0x00300ff4
#define dmacpcellid1         (*(volatile unsigned int *)dmacpcellid1_adr)
#define dmacpcellid2_adr     0x00300ff8
#define dmacpcellid2         (*(volatile unsigned int *)dmacpcellid2_adr)
#define dmacpcellid3_adr     0x00300ffc
#define dmacpcellid3         (*(volatile unsigned int *)dmacpcellid3_adr)
#define prc_brk_out0_adr     0x00310000
#define prc_brk_out0         (*(volatile unsigned int *)prc_brk_out0_adr)
#define prc_brk_out1_adr     0x00310004
#define prc_brk_out1         (*(volatile unsigned int *)prc_brk_out1_adr)
#define prc_brk_out2_adr     0x00310008
#define prc_brk_out2         (*(volatile unsigned int *)prc_brk_out2_adr)
#define prc_brk_out3_adr     0x0031000c
#define prc_brk_out3         (*(volatile unsigned int *)prc_brk_out3_adr)
#define prc_brk_out4_adr     0x00310010
#define prc_brk_out4         (*(volatile unsigned int *)prc_brk_out4_adr)
#define prc_brk_out5_adr     0x00310014
#define prc_brk_out5         (*(volatile unsigned int *)prc_brk_out5_adr)
#define prc_brk_out6_adr     0x00310018
#define prc_brk_out6         (*(volatile unsigned int *)prc_brk_out6_adr)
#define prc_brk_out7_adr     0x0031001c
#define prc_brk_out7         (*(volatile unsigned int *)prc_brk_out7_adr)
#define prc_brk_out8_adr     0x00310020
#define prc_brk_out8         (*(volatile unsigned int *)prc_brk_out8_adr)
#define prc_brk_out9_adr     0x00310024
#define prc_brk_out9         (*(volatile unsigned int *)prc_brk_out9_adr)
#define prc_brk_outA_adr     0x00310028
#define prc_brk_outA         (*(volatile unsigned int *)prc_brk_outA_adr)
#define prc_brk_outB_adr     0x0031002c
#define prc_brk_outB         (*(volatile unsigned int *)prc_brk_outB_adr)
#define prc_brk_outC_adr     0x00310030
#define prc_brk_outC         (*(volatile unsigned int *)prc_brk_outC_adr)
#define prc_brk_outD_adr     0x00310034
#define prc_brk_outD         (*(volatile unsigned int *)prc_brk_outD_adr)
#define prc_brk_outE_adr     0x00310038
#define prc_brk_outE         (*(volatile unsigned int *)prc_brk_outE_adr)
#define prc_brk_outF_adr     0x0031003c
#define prc_brk_outF         (*(volatile unsigned int *)prc_brk_outF_adr)
#define prc_brk_out10_adr    0x00310040
#define prc_brk_out10        (*(volatile unsigned int *)prc_brk_out10_adr)
#define prc_brk_out11_adr    0x00310044
#define prc_brk_out11        (*(volatile unsigned int *)prc_brk_out11_adr)
#define prc_brk_out12_adr    0x00310048
#define prc_brk_out12        (*(volatile unsigned int *)prc_brk_out12_adr)
#define prc_brk_out13_adr    0x0031004c
#define prc_brk_out13        (*(volatile unsigned int *)prc_brk_out13_adr)
#define prc_brk_out14_adr    0x00310050
#define prc_brk_out14        (*(volatile unsigned int *)prc_brk_out14_adr)
#define prc_brk_out15_adr    0x00310054
#define prc_brk_out15        (*(volatile unsigned int *)prc_brk_out15_adr)
#define prc_brk_out16_adr    0x00310058
#define prc_brk_out16        (*(volatile unsigned int *)prc_brk_out16_adr)
#define prc_brk_out17_adr    0x0031005c
#define prc_brk_out17        (*(volatile unsigned int *)prc_brk_out17_adr)
#define prc_brk_out18_adr    0x00310060
#define prc_brk_out18        (*(volatile unsigned int *)prc_brk_out18_adr)
#define prc_brk_out19_adr    0x00310064
#define prc_brk_out19        (*(volatile unsigned int *)prc_brk_out19_adr)
#define prc_brk_out1A_adr    0x00310068
#define prc_brk_out1A        (*(volatile unsigned int *)prc_brk_out1A_adr)
#define prc_brk_out1B_adr    0x0031006c
#define prc_brk_out1B        (*(volatile unsigned int *)prc_brk_out1B_adr)
#define prc_brk_out1C_adr    0x00310070
#define prc_brk_out1C        (*(volatile unsigned int *)prc_brk_out1C_adr)
#define prc_brk_out1D_adr    0x00310074
#define prc_brk_out1D        (*(volatile unsigned int *)prc_brk_out1D_adr)
#define prc_brk_out1E_adr    0x00310078
#define prc_brk_out1E        (*(volatile unsigned int *)prc_brk_out1E_adr)
#define prc_brk_out1F_adr    0x0031007c
#define prc_brk_out1F        (*(volatile unsigned int *)prc_brk_out1F_adr)
#define prc_brk_out20_adr    0x00310080
#define prc_brk_out20        (*(volatile unsigned int *)prc_brk_out20_adr)
#define prc_brk_out21_adr    0x00310084
#define prc_brk_out21        (*(volatile unsigned int *)prc_brk_out21_adr)
#define prc_brk_out22_adr    0x00310088
#define prc_brk_out22        (*(volatile unsigned int *)prc_brk_out22_adr)
#define prc_brk_out23_adr    0x0031008c
#define prc_brk_out23        (*(volatile unsigned int *)prc_brk_out23_adr)
#define prc_brk_out24_adr    0x00310090
#define prc_brk_out24        (*(volatile unsigned int *)prc_brk_out24_adr)
#define prc_brk_out25_adr    0x00310094
#define prc_brk_out25        (*(volatile unsigned int *)prc_brk_out25_adr)
#define prc_brk_out26_adr    0x00310098
#define prc_brk_out26        (*(volatile unsigned int *)prc_brk_out26_adr)
#define prc_brk_out27_adr    0x0031009c
#define prc_brk_out27        (*(volatile unsigned int *)prc_brk_out27_adr)
#define prc_brk_out28_adr    0x003100a0
#define prc_brk_out28        (*(volatile unsigned int *)prc_brk_out28_adr)
#define prc_brk_out29_adr    0x003100a4
#define prc_brk_out29        (*(volatile unsigned int *)prc_brk_out29_adr)
#define prc_brk_out2A_adr    0x003100a8
#define prc_brk_out2A        (*(volatile unsigned int *)prc_brk_out2A_adr)
#define prc_brk_out2B_adr    0x003100ac
#define prc_brk_out2B        (*(volatile unsigned int *)prc_brk_out2B_adr)
#define prc_brk_out2C_adr    0x003100b0
#define prc_brk_out2C        (*(volatile unsigned int *)prc_brk_out2C_adr)
#define prc_brk_out2D_adr    0x003100b4
#define prc_brk_out2D        (*(volatile unsigned int *)prc_brk_out2D_adr)
#define prc_brk_out2E_adr    0x003100b8
#define prc_brk_out2E        (*(volatile unsigned int *)prc_brk_out2E_adr)
#define prc_brk_out2F_adr    0x003100bc
#define prc_brk_out2F        (*(volatile unsigned int *)prc_brk_out2F_adr)
#define prc_brk_out30_adr    0x003100c0
#define prc_brk_out30        (*(volatile unsigned int *)prc_brk_out30_adr)
#define prc_brk_out31_adr    0x003100c4
#define prc_brk_out31        (*(volatile unsigned int *)prc_brk_out31_adr)
#define prc_brk_out32_adr    0x003100c8
#define prc_brk_out32        (*(volatile unsigned int *)prc_brk_out32_adr)
#define prc_brk_out33_adr    0x003100cc
#define prc_brk_out33        (*(volatile unsigned int *)prc_brk_out33_adr)
#define prc_brk_out34_adr    0x003100d0
#define prc_brk_out34        (*(volatile unsigned int *)prc_brk_out34_adr)
#define prc_brk_out35_adr    0x003100d4
#define prc_brk_out35        (*(volatile unsigned int *)prc_brk_out35_adr)
#define prc_brk_out36_adr    0x003100d8
#define prc_brk_out36        (*(volatile unsigned int *)prc_brk_out36_adr)
#define prc_brk_out37_adr    0x003100dc
#define prc_brk_out37        (*(volatile unsigned int *)prc_brk_out37_adr)
#define prc_brk_out38_adr    0x003100e0
#define prc_brk_out38        (*(volatile unsigned int *)prc_brk_out38_adr)
#define prc_brk_out39_adr    0x003100e4
#define prc_brk_out39        (*(volatile unsigned int *)prc_brk_out39_adr)
#define prc_brk_out3A_adr    0x003100e8
#define prc_brk_out3A        (*(volatile unsigned int *)prc_brk_out3A_adr)
#define prc_brk_out3B_adr    0x003100ec
#define prc_brk_out3B        (*(volatile unsigned int *)prc_brk_out3B_adr)
#define prc_brk_out3C_adr    0x003100f0
#define prc_brk_out3C        (*(volatile unsigned int *)prc_brk_out3C_adr)
#define prc_brk_out3D_adr    0x003100f4
#define prc_brk_out3D        (*(volatile unsigned int *)prc_brk_out3D_adr)
#define prc_brk_out3E_adr    0x003100f8
#define prc_brk_out3E        (*(volatile unsigned int *)prc_brk_out3E_adr)
#define prc_brk_out3F_adr    0x003100fc
#define prc_brk_out3F        (*(volatile unsigned int *)prc_brk_out3F_adr)
#define prc_brk_out40_adr    0x00310100
#define prc_brk_out40        (*(volatile unsigned int *)prc_brk_out40_adr)
#define prc_brk_out41_adr    0x00310104
#define prc_brk_out41        (*(volatile unsigned int *)prc_brk_out41_adr)
#define prc_brk_out42_adr    0x00310108
#define prc_brk_out42        (*(volatile unsigned int *)prc_brk_out42_adr)
#define prc_brk_out43_adr    0x0031010c
#define prc_brk_out43        (*(volatile unsigned int *)prc_brk_out43_adr)
#define prc_brk_out44_adr    0x00310110
#define prc_brk_out44        (*(volatile unsigned int *)prc_brk_out44_adr)
#define prc_brk_out45_adr    0x00310114
#define prc_brk_out45        (*(volatile unsigned int *)prc_brk_out45_adr)
#define prc_brk_out46_adr    0x00310118
#define prc_brk_out46        (*(volatile unsigned int *)prc_brk_out46_adr)
#define prc_brk_out47_adr    0x0031011c
#define prc_brk_out47        (*(volatile unsigned int *)prc_brk_out47_adr)
#define prc_brk_out48_adr    0x00310120
#define prc_brk_out48        (*(volatile unsigned int *)prc_brk_out48_adr)
#define prc_brk_out49_adr    0x00310124
#define prc_brk_out49        (*(volatile unsigned int *)prc_brk_out49_adr)
#define prc_brk_out4A_adr    0x00310128
#define prc_brk_out4A        (*(volatile unsigned int *)prc_brk_out4A_adr)
#define prc_brk_out4B_adr    0x0031012c
#define prc_brk_out4B        (*(volatile unsigned int *)prc_brk_out4B_adr)
#define prc_brk_out4C_adr    0x00310130
#define prc_brk_out4C        (*(volatile unsigned int *)prc_brk_out4C_adr)
#define prc_brk_out4D_adr    0x00310134
#define prc_brk_out4D        (*(volatile unsigned int *)prc_brk_out4D_adr)
#define prc_brk_out4E_adr    0x00310138
#define prc_brk_out4E        (*(volatile unsigned int *)prc_brk_out4E_adr)
#define prc_brk_out4F_adr    0x0031013c
#define prc_brk_out4F        (*(volatile unsigned int *)prc_brk_out4F_adr)
#define prc_brk_out50_adr    0x00310140
#define prc_brk_out50        (*(volatile unsigned int *)prc_brk_out50_adr)
#define prc_brk_out51_adr    0x00310144
#define prc_brk_out51        (*(volatile unsigned int *)prc_brk_out51_adr)
#define prc_brk_out52_adr    0x00310148
#define prc_brk_out52        (*(volatile unsigned int *)prc_brk_out52_adr)
#define prc_brk_out53_adr    0x0031014c
#define prc_brk_out53        (*(volatile unsigned int *)prc_brk_out53_adr)
#define prc_brk_out54_adr    0x00310150
#define prc_brk_out54        (*(volatile unsigned int *)prc_brk_out54_adr)
#define prc_brk_out55_adr    0x00310154
#define prc_brk_out55        (*(volatile unsigned int *)prc_brk_out55_adr)
#define prc_brk_out56_adr    0x00310158
#define prc_brk_out56        (*(volatile unsigned int *)prc_brk_out56_adr)
#define prc_brk_out57_adr    0x0031015c
#define prc_brk_out57        (*(volatile unsigned int *)prc_brk_out57_adr)
#define prc_brk_out58_adr    0x00310160
#define prc_brk_out58        (*(volatile unsigned int *)prc_brk_out58_adr)
#define prc_brk_out59_adr    0x00310164
#define prc_brk_out59        (*(volatile unsigned int *)prc_brk_out59_adr)
#define prc_brk_out5A_adr    0x00310168
#define prc_brk_out5A        (*(volatile unsigned int *)prc_brk_out5A_adr)
#define prc_brk_out5B_adr    0x0031016c
#define prc_brk_out5B        (*(volatile unsigned int *)prc_brk_out5B_adr)
#define prc_brk_out5C_adr    0x00310170
#define prc_brk_out5C        (*(volatile unsigned int *)prc_brk_out5C_adr)
#define prc_brk_out5D_adr    0x00310174
#define prc_brk_out5D        (*(volatile unsigned int *)prc_brk_out5D_adr)
#define prc_brk_out5E_adr    0x00310178
#define prc_brk_out5E        (*(volatile unsigned int *)prc_brk_out5E_adr)
#define prc_brk_out5F_adr    0x0031017c
#define prc_brk_out5F        (*(volatile unsigned int *)prc_brk_out5F_adr)
#define prc_brk_out60_adr    0x00310180
#define prc_brk_out60        (*(volatile unsigned int *)prc_brk_out60_adr)
#define prc_brk_out61_adr    0x00310184
#define prc_brk_out61        (*(volatile unsigned int *)prc_brk_out61_adr)
#define prc_brk_out62_adr    0x00310188
#define prc_brk_out62        (*(volatile unsigned int *)prc_brk_out62_adr)
#define prc_brk_out63_adr    0x0031018c
#define prc_brk_out63        (*(volatile unsigned int *)prc_brk_out63_adr)
#define prc_brk_out64_adr    0x00310190
#define prc_brk_out64        (*(volatile unsigned int *)prc_brk_out64_adr)
#define prc_brk_out65_adr    0x00310194
#define prc_brk_out65        (*(volatile unsigned int *)prc_brk_out65_adr)
#define prc_brk_out66_adr    0x00310198
#define prc_brk_out66        (*(volatile unsigned int *)prc_brk_out66_adr)
#define prc_brk_out67_adr    0x0031019c
#define prc_brk_out67        (*(volatile unsigned int *)prc_brk_out67_adr)
#define prc_brk_out68_adr    0x003101a0
#define prc_brk_out68        (*(volatile unsigned int *)prc_brk_out68_adr)
#define prc_brk_out69_adr    0x003101a4
#define prc_brk_out69        (*(volatile unsigned int *)prc_brk_out69_adr)
#define prc_brk_out6A_adr    0x003101a8
#define prc_brk_out6A        (*(volatile unsigned int *)prc_brk_out6A_adr)
#define prc_brk_out6B_adr    0x003101ac
#define prc_brk_out6B        (*(volatile unsigned int *)prc_brk_out6B_adr)
#define prc_brk_out6C_adr    0x003101b0
#define prc_brk_out6C        (*(volatile unsigned int *)prc_brk_out6C_adr)
#define prc_brk_out6D_adr    0x003101b4
#define prc_brk_out6D        (*(volatile unsigned int *)prc_brk_out6D_adr)
#define prc_brk_out6E_adr    0x003101b8
#define prc_brk_out6E        (*(volatile unsigned int *)prc_brk_out6E_adr)
#define prc_brk_out6F_adr    0x003101bc
#define prc_brk_out6F        (*(volatile unsigned int *)prc_brk_out6F_adr)
#define prc_brk_out70_adr    0x003101c0
#define prc_brk_out70        (*(volatile unsigned int *)prc_brk_out70_adr)
#define prc_brk_out71_adr    0x003101c4
#define prc_brk_out71        (*(volatile unsigned int *)prc_brk_out71_adr)
#define prc_brk_out72_adr    0x003101c8
#define prc_brk_out72        (*(volatile unsigned int *)prc_brk_out72_adr)
#define prc_brk_out73_adr    0x003101cc
#define prc_brk_out73        (*(volatile unsigned int *)prc_brk_out73_adr)
#define prc_brk_out74_adr    0x003101d0
#define prc_brk_out74        (*(volatile unsigned int *)prc_brk_out74_adr)
#define prc_brk_out75_adr    0x003101d4
#define prc_brk_out75        (*(volatile unsigned int *)prc_brk_out75_adr)
#define prc_brk_out76_adr    0x003101d8
#define prc_brk_out76        (*(volatile unsigned int *)prc_brk_out76_adr)
#define prc_brk_out77_adr    0x003101dc
#define prc_brk_out77        (*(volatile unsigned int *)prc_brk_out77_adr)
#define prc_brk_out78_adr    0x003101e0
#define prc_brk_out78        (*(volatile unsigned int *)prc_brk_out78_adr)
#define prc_brk_out79_adr    0x003101e4
#define prc_brk_out79        (*(volatile unsigned int *)prc_brk_out79_adr)
#define prc_brk_out7A_adr    0x003101e8
#define prc_brk_out7A        (*(volatile unsigned int *)prc_brk_out7A_adr)
#define prc_brk_out7B_adr    0x003101ec
#define prc_brk_out7B        (*(volatile unsigned int *)prc_brk_out7B_adr)
#define prc_brk_out7C_adr    0x003101f0
#define prc_brk_out7C        (*(volatile unsigned int *)prc_brk_out7C_adr)
#define prc_brk_out7D_adr    0x003101f4
#define prc_brk_out7D        (*(volatile unsigned int *)prc_brk_out7D_adr)
#define prc_brk_out7E_adr    0x003101f8
#define prc_brk_out7E        (*(volatile unsigned int *)prc_brk_out7E_adr)
#define prc_brk_out7F_adr    0x003101fc
#define prc_brk_out7F        (*(volatile unsigned int *)prc_brk_out7F_adr)
#define prc_patch_in_adr     0x00310200
#define prc_patch_in         (*(volatile unsigned int *)prc_patch_in_adr)
#define prc_patch_reg_en0_adr 0x00310204
#define prc_patch_reg_en0    (*(volatile unsigned int *)prc_patch_reg_en0_adr)
#define prc_patch_reg_en1_adr 0x00310208
#define prc_patch_reg_en1    (*(volatile unsigned int *)prc_patch_reg_en1_adr)
#define prc_patch_reg_en2_adr 0x0031020c
#define prc_patch_reg_en2    (*(volatile unsigned int *)prc_patch_reg_en2_adr)
#define prc_patch_reg_en3_adr 0x00310210
#define prc_patch_reg_en3    (*(volatile unsigned int *)prc_patch_reg_en3_adr)
#define prc_dpatch_en_adr    0x00310214
#define prc_dpatch_en        (*(volatile unsigned int *)prc_dpatch_en_adr)
#define prc_data_brk0_adr    0x00310220
#define prc_data_brk0        (*(volatile unsigned int *)prc_data_brk0_adr)
#define prc_data_brk1_adr    0x00310224
#define prc_data_brk1        (*(volatile unsigned int *)prc_data_brk1_adr)
#define prc_data_brk2_adr    0x00310228
#define prc_data_brk2        (*(volatile unsigned int *)prc_data_brk2_adr)
#define prc_data_brk3_adr    0x0031022c
#define prc_data_brk3        (*(volatile unsigned int *)prc_data_brk3_adr)
#define prc_data_pat0_adr    0x00310230
#define prc_data_pat0        (*(volatile unsigned int *)prc_data_pat0_adr)
#define prc_data_pat1_adr    0x00310234
#define prc_data_pat1        (*(volatile unsigned int *)prc_data_pat1_adr)
#define prc_data_pat2_adr    0x00310238
#define prc_data_pat2        (*(volatile unsigned int *)prc_data_pat2_adr)
#define prc_data_pat3_adr    0x0031023c
#define prc_data_pat3        (*(volatile unsigned int *)prc_data_pat3_adr)
#define prcprotection_adr    0x00310300
#define prcprotection        (*(volatile unsigned int *)prcprotection_adr)
#define prc_periphid0_adr    0x00310fe0
#define prc_periphid0        (*(volatile unsigned int *)prc_periphid0_adr)
#define prc_periphid1_adr    0x00310fe4
#define prc_periphid1        (*(volatile unsigned int *)prc_periphid1_adr)
#define prc_periphid2_adr    0x00310fe8
#define prc_periphid2        (*(volatile unsigned int *)prc_periphid2_adr)
#define prc_periphid3_adr    0x00310fec
#define prc_periphid3        (*(volatile unsigned int *)prc_periphid3_adr)
#define prc_pcellid0_adr     0x00310ff0
#define prc_pcellid0         (*(volatile unsigned int *)prc_pcellid0_adr)
#define prc_pcellid1_adr     0x00310ff4
#define prc_pcellid1         (*(volatile unsigned int *)prc_pcellid1_adr)
#define prc_pcellid2_adr     0x00310ff8
#define prc_pcellid2         (*(volatile unsigned int *)prc_pcellid2_adr)
#define prc_pcellid3_adr     0x00310ffc
#define prc_pcellid3         (*(volatile unsigned int *)prc_pcellid3_adr)
#define smbidcyr0_adr        0x00308000
#define smbidcyr0            (*(volatile unsigned int *)smbidcyr0_adr)
#define smbwst1r0_adr        0x00308004
#define smbwst1r0            (*(volatile unsigned int *)smbwst1r0_adr)
#define smbwst2r0_adr        0x00308008
#define smbwst2r0            (*(volatile unsigned int *)smbwst2r0_adr)
#define smbwstoenr0_adr      0x0030800c
#define smbwstoenr0          (*(volatile unsigned int *)smbwstoenr0_adr)
#define smbwstwenr0_adr      0x00308010
#define smbwstwenr0          (*(volatile unsigned int *)smbwstwenr0_adr)
#define smbcr0_adr           0x00308014
#define smbcr0               (*(volatile unsigned int *)smbcr0_adr)
#define smbsr0_adr           0x00308018
#define smbsr0               (*(volatile unsigned int *)smbsr0_adr)
#define smbidcyr1_adr        0x0030801c
#define smbidcyr1            (*(volatile unsigned int *)smbidcyr1_adr)
#define smbwst1r1_adr        0x00308020
#define smbwst1r1            (*(volatile unsigned int *)smbwst1r1_adr)
#define smbwst2r1_adr        0x00308024
#define smbwst2r1            (*(volatile unsigned int *)smbwst2r1_adr)
#define smbwstoenr1_adr      0x00308028
#define smbwstoenr1          (*(volatile unsigned int *)smbwstoenr1_adr)
#define smbwstwenr1_adr      0x0030802c
#define smbwstwenr1          (*(volatile unsigned int *)smbwstwenr1_adr)
#define smbcr1_adr           0x00308030
#define smbcr1               (*(volatile unsigned int *)smbcr1_adr)
#define smbsr1_adr           0x00308034
#define smbsr1               (*(volatile unsigned int *)smbsr1_adr)
#define smbidcyr2_adr        0x00308038
#define smbidcyr2            (*(volatile unsigned int *)smbidcyr2_adr)
#define smbwst1r2_adr        0x0030803c
#define smbwst1r2            (*(volatile unsigned int *)smbwst1r2_adr)
#define smbwst2r2_adr        0x00308040
#define smbwst2r2            (*(volatile unsigned int *)smbwst2r2_adr)
#define smbwstoenr2_adr      0x00308044
#define smbwstoenr2          (*(volatile unsigned int *)smbwstoenr2_adr)
#define smbwstwenr2_adr      0x00308048
#define smbwstwenr2          (*(volatile unsigned int *)smbwstwenr2_adr)
#define smbcr2_adr           0x0030804c
#define smbcr2               (*(volatile unsigned int *)smbcr2_adr)
#define smbsr2_adr           0x00308050
#define smbsr2               (*(volatile unsigned int *)smbsr2_adr)
#define smbidcyr3_adr        0x00308054
#define smbidcyr3            (*(volatile unsigned int *)smbidcyr3_adr)
#define smbwst1r3_adr        0x00308058
#define smbwst1r3            (*(volatile unsigned int *)smbwst1r3_adr)
#define smbwst2r3_adr        0x0030805c
#define smbwst2r3            (*(volatile unsigned int *)smbwst2r3_adr)
#define smbwstoenr3_adr      0x00308060
#define smbwstoenr3          (*(volatile unsigned int *)smbwstoenr3_adr)
#define smbwstwenr3_adr      0x00308064
#define smbwstwenr3          (*(volatile unsigned int *)smbwstwenr3_adr)
#define smbcr3_adr           0x00308068
#define smbcr3               (*(volatile unsigned int *)smbcr3_adr)
#define smbsr3_adr           0x0030806c
#define smbsr3               (*(volatile unsigned int *)smbsr3_adr)
#define smbidcyr4_adr        0x00308070
#define smbidcyr4            (*(volatile unsigned int *)smbidcyr4_adr)
#define smbwst1r4_adr        0x00308074
#define smbwst1r4            (*(volatile unsigned int *)smbwst1r4_adr)
#define smbwst2r4_adr        0x00308078
#define smbwst2r4            (*(volatile unsigned int *)smbwst2r4_adr)
#define smbwstoenr4_adr      0x0030807c
#define smbwstoenr4          (*(volatile unsigned int *)smbwstoenr4_adr)
#define smbwstwenr4_adr      0x00308080
#define smbwstwenr4          (*(volatile unsigned int *)smbwstwenr4_adr)
#define smbcr4_adr           0x00308084
#define smbcr4               (*(volatile unsigned int *)smbcr4_adr)
#define smbsr4_adr           0x00308088
#define smbsr4               (*(volatile unsigned int *)smbsr4_adr)
#define smbidcyr5_adr        0x0030808c
#define smbidcyr5            (*(volatile unsigned int *)smbidcyr5_adr)
#define smbwst1r5_adr        0x00308090
#define smbwst1r5            (*(volatile unsigned int *)smbwst1r5_adr)
#define smbwst2r5_adr        0x00308094
#define smbwst2r5            (*(volatile unsigned int *)smbwst2r5_adr)
#define smbwstoenr5_adr      0x00308098
#define smbwstoenr5          (*(volatile unsigned int *)smbwstoenr5_adr)
#define smbwstwenr5_adr      0x0030809c
#define smbwstwenr5          (*(volatile unsigned int *)smbwstwenr5_adr)
#define smbcr5_adr           0x003080a0
#define smbcr5               (*(volatile unsigned int *)smbcr5_adr)
#define smbsr5_adr           0x003080a4
#define smbsr5               (*(volatile unsigned int *)smbsr5_adr)
#define smbidcyr6_adr        0x003080a8
#define smbidcyr6            (*(volatile unsigned int *)smbidcyr6_adr)
#define smbwst1r6_adr        0x003080ac
#define smbwst1r6            (*(volatile unsigned int *)smbwst1r6_adr)
#define smbwst2r6_adr        0x003080b0
#define smbwst2r6            (*(volatile unsigned int *)smbwst2r6_adr)
#define smbwstoenr6_adr      0x003080b4
#define smbwstoenr6          (*(volatile unsigned int *)smbwstoenr6_adr)
#define smbwstwenr6_adr      0x003080b8
#define smbwstwenr6          (*(volatile unsigned int *)smbwstwenr6_adr)
#define smbcr6_adr           0x003080bc
#define smbcr6               (*(volatile unsigned int *)smbcr6_adr)
#define smbsr6_adr           0x003080c0
#define smbsr6               (*(volatile unsigned int *)smbsr6_adr)
#define smbidcyr7_adr        0x003080c4
#define smbidcyr7            (*(volatile unsigned int *)smbidcyr7_adr)
#define smbwst1r7_adr        0x003080c8
#define smbwst1r7            (*(volatile unsigned int *)smbwst1r7_adr)
#define smbwst2r7_adr        0x003080cc
#define smbwst2r7            (*(volatile unsigned int *)smbwst2r7_adr)
#define smbwstoenr7_adr      0x003080d0
#define smbwstoenr7          (*(volatile unsigned int *)smbwstoenr7_adr)
#define smbwstwenr7_adr      0x003080d4
#define smbwstwenr7          (*(volatile unsigned int *)smbwstwenr7_adr)
#define smbcr7_adr           0x003080d8
#define smbcr7               (*(volatile unsigned int *)smbcr7_adr)
#define smbsr7_adr           0x003080dc
#define smbsr7               (*(volatile unsigned int *)smbsr7_adr)
#define smbews_adr           0x003080e0
#define smbews               (*(volatile unsigned int *)smbews_adr)
#define smcperiphid0_adr     0x00308fe0
#define smcperiphid0         (*(volatile unsigned int *)smcperiphid0_adr)
#define smcperiphid1_adr     0x00308fe4
#define smcperiphid1         (*(volatile unsigned int *)smcperiphid1_adr)
#define smcperiphid2_adr     0x00308fe8
#define smcperiphid2         (*(volatile unsigned int *)smcperiphid2_adr)
#define smcperiphid3_adr     0x00308fec
#define smcperiphid3         (*(volatile unsigned int *)smcperiphid3_adr)
#define smcpcellid0_adr      0x00308ff0
#define smcpcellid0          (*(volatile unsigned int *)smcpcellid0_adr)
#define smcpcellid1_adr      0x00308ff4
#define smcpcellid1          (*(volatile unsigned int *)smcpcellid1_adr)
#define smcpcellid2_adr      0x00308ff8
#define smcpcellid2          (*(volatile unsigned int *)smcpcellid2_adr)
#define smcpcellid3_adr      0x00308ffc
#define smcpcellid3          (*(volatile unsigned int *)smcpcellid3_adr)
#define bpl_base             0x00318000
#define cr_base              0x00318000
#define cr_adr0              0x00318000
#define cr0                  (*(volatile unsigned int *)cr_adr0)
#define sr_op_state_adr      0x00318004
#define sr_op_state          (*(volatile unsigned int *)sr_op_state_adr)
#define sr_base              0x00318008
#define sr_status_adr0       0x00318008
#define sr_status0           (*(volatile unsigned int *)sr_status_adr0)
#define phy_status_adr       0x0031800c
#define phy_status           (*(volatile unsigned int *)phy_status_adr)
#define phy_status_int_en_adr 0x00318010
#define phy_status_int_en    (*(volatile unsigned int *)phy_status_int_en_adr)
#define wib_cr_op_state_adr  0x00318014
#define wib_cr_op_state      (*(volatile unsigned int *)wib_cr_op_state_adr)
#define cr_adr1              0x00318018
#define cr1                  (*(volatile unsigned int *)cr_adr1)
#define wib_time_offset_adr  0x0031801c
#define wib_time_offset      (*(volatile unsigned int *)wib_time_offset_adr)
#define dc_base              0x00318024
#define dc_pc_ctx_idx_adr    0x00318024
#define dc_pc_ctx_idx        (*(volatile unsigned int *)dc_pc_ctx_idx_adr)
#define dc_hopst_adr         0x00318028
#define dc_hopst             (*(volatile unsigned int *)dc_hopst_adr)
#define dc_prty_lo_adr       0x00318030
#define dc_prty_lo           (*(volatile unsigned int *)dc_prty_lo_adr)
#define dc_prty_hi_adr       0x00318034
#define dc_prty_hi           (*(volatile unsigned int *)dc_prty_hi_adr)
#define dc_bta_lo_adr        0x00318038
#define dc_bta_lo            (*(volatile unsigned int *)dc_bta_lo_adr)
#define dc_bta_hi_adr        0x0031803c
#define dc_bta_hi            (*(volatile unsigned int *)dc_bta_hi_adr)
#define dc_clsdev_adr        0x00318040
#define dc_clsdev            (*(volatile unsigned int *)dc_clsdev_adr)
#define dc_ipa_lo_adr        0x00318044
#define dc_ipa_lo            (*(volatile unsigned int *)dc_ipa_lo_adr)
#define dc_ipa_hi_adr        0x00318048
#define dc_ipa_hi            (*(volatile unsigned int *)dc_ipa_hi_adr)
#define dc_pg_uap_adr        0x0031804c
#define dc_pg_uap            (*(volatile unsigned int *)dc_pg_uap_adr)
#define dc_fhs_pyld_adr      0x00318050
#define dc_fhs_pyld          (*(volatile unsigned int *)dc_fhs_pyld_adr)
#define dc_n_iq_adr          0x00318054
#define dc_n_iq              (*(volatile unsigned int *)dc_n_iq_adr)
#define dc_iq_to_adr         0x00318058
#define dc_iq_to             (*(volatile unsigned int *)dc_iq_to_adr)
#define dc_iq_respto_adr     0x0031805c
#define dc_iq_respto         (*(volatile unsigned int *)dc_iq_respto_adr)
#define dc_n_pg_adr          0x00318060
#define dc_n_pg              (*(volatile unsigned int *)dc_n_pg_adr)
#define dc_pg_to_adr         0x00318064
#define dc_pg_to             (*(volatile unsigned int *)dc_pg_to_adr)
#define dc_pg_respto_adr     0x00318068
#define dc_pg_respto         (*(volatile unsigned int *)dc_pg_respto_adr)
#define dc_newcon_to_adr     0x0031806c
#define dc_newcon_to         (*(volatile unsigned int *)dc_newcon_to_adr)
#define dc_pg_ckoff_adr      0x00318070
#define dc_pg_ckoff          (*(volatile unsigned int *)dc_pg_ckoff_adr)
#define dc_n_pclk_oft_adr    0x00318074
#define dc_n_pclk_oft        (*(volatile unsigned int *)dc_n_pclk_oft_adr)
#define dc_n_clk_oft_adr     0x00318078
#define dc_n_clk_oft         (*(volatile unsigned int *)dc_n_clk_oft_adr)
#define dc_n_clk_wake_cnt_adr 0x0031807c
#define dc_n_clk_wake_cnt    (*(volatile unsigned int *)dc_n_clk_wake_cnt_adr)
#define dc_nclk_ctl_adr      0x00318080
#define dc_nclk_ctl          (*(volatile unsigned int *)dc_nclk_ctl_adr)
#define dc_nbtc_pclk_adr     0x00318084
#define dc_nbtc_pclk         (*(volatile unsigned int *)dc_nbtc_pclk_adr)
#define dc_nbtc_clk_adr      0x00318088
#define dc_nbtc_clk          (*(volatile unsigned int *)dc_nbtc_clk_adr)
#define dc_acode_dly_adr     0x0031808c
#define dc_acode_dly         (*(volatile unsigned int *)dc_acode_dly_adr)
#define dc_cr_op_st_en_cnt_adr 0x00318090
#define dc_cr_op_st_en_cnt   (*(volatile unsigned int *)dc_cr_op_st_en_cnt_adr)
#define dc_rtx_start_cnt_adr 0x00318094
#define dc_rtx_start_cnt     (*(volatile unsigned int *)dc_rtx_start_cnt_adr)
#define dc_pll_en_cnt_adr    0x00318098
#define dc_pll_en_cnt        (*(volatile unsigned int *)dc_pll_en_cnt_adr)
#define fsm_intrp_enable_adr 0x0031809c
#define fsm_intrp_enable     (*(volatile unsigned int *)fsm_intrp_enable_adr)
#define dc_smpl_ctl_adr      0x003180a0
#define dc_smpl_ctl          (*(volatile unsigned int *)dc_smpl_ctl_adr)
#define dc_slot_offset_adr   0x003180a4
#define dc_slot_offset       (*(volatile unsigned int *)dc_slot_offset_adr)
#define dc_win_pkt0_adr      0x003180a8
#define dc_win_pkt0          (*(volatile unsigned int *)dc_win_pkt0_adr)
#define dc_win_pkt1_adr      0x003180ac
#define dc_win_pkt1          (*(volatile unsigned int *)dc_win_pkt1_adr)
#define dc_win_pkt2_adr      0x003180b0
#define dc_win_pkt2          (*(volatile unsigned int *)dc_win_pkt2_adr)
#define dc_win_txconfx_adr   0x003180b4
#define dc_win_txconfx       (*(volatile unsigned int *)dc_win_txconfx_adr)
#define txconfx_st_adr       0x003180b8
#define txconfx_st           (*(volatile unsigned int *)txconfx_st_adr)
#define dc_icoex_adr         0x003180bc
#define dc_icoex             (*(volatile unsigned int *)dc_icoex_adr)
#define dc_icoex_freq0_adr   0x003180c0
#define dc_icoex_freq0       (*(volatile unsigned int *)dc_icoex_freq0_adr)
#define dc_icoex_freq1_adr   0x003180c4
#define dc_icoex_freq1       (*(volatile unsigned int *)dc_icoex_freq1_adr)
#define dc_icoex_freq2_adr   0x003180c8
#define dc_icoex_freq2       (*(volatile unsigned int *)dc_icoex_freq2_adr)
#define dc_icoex_freq3_adr   0x003180cc
#define dc_icoex_freq3       (*(volatile unsigned int *)dc_icoex_freq3_adr)
#define dc_bt_clk_offset_adr 0x003180d0
#define dc_bt_clk_offset     (*(volatile unsigned int *)dc_bt_clk_offset_adr)
#define dc_diac_lo_adr       0x003180d4
#define dc_diac_lo           (*(volatile unsigned int *)dc_diac_lo_adr)
#define dc_diac_hi_adr       0x003180d8
#define dc_diac_hi           (*(volatile unsigned int *)dc_diac_hi_adr)
#define dc_giac_lo_adr       0x003180dc
#define dc_giac_lo           (*(volatile unsigned int *)dc_giac_lo_adr)
#define dc_giac_hi_adr       0x003180e0
#define dc_giac_hi           (*(volatile unsigned int *)dc_giac_hi_adr)
#define dc_dual_synch_adr    0x003180e4
#define dc_dual_synch        (*(volatile unsigned int *)dc_dual_synch_adr)
#define dc_win_pkt3_adr      0x003180e8
#define dc_win_pkt3          (*(volatile unsigned int *)dc_win_pkt3_adr)
#define dc_2clb_eci_lsw_adr  0x003180ec
#define dc_2clb_eci_lsw      (*(volatile unsigned int *)dc_2clb_eci_lsw_adr)
#define dc_2clb_eci_msw_adr  0x003180f0
#define dc_2clb_eci_msw      (*(volatile unsigned int *)dc_2clb_eci_msw_adr)
#define dc_cap_SyncCnt_adr   0x003180f4
#define dc_cap_SyncCnt       (*(volatile unsigned int *)dc_cap_SyncCnt_adr)
#define dc_scan_pg_cfg_adr   0x003180f8
#define dc_scan_pg_cfg       (*(volatile unsigned int *)dc_scan_pg_cfg_adr)
#define dc_pcxclkdiff_adr    0x003180fc
#define dc_pcxclkdiff        (*(volatile unsigned int *)dc_pcxclkdiff_adr)
#define wib_t_ifs_adr        0x00318100
#define wib_t_ifs            (*(volatile unsigned int *)wib_t_ifs_adr)
#define dcenc_base           0x00318104
#define encryption_key_adr0  0x00318184
#define encryption_key0      (*(volatile unsigned int *)encryption_key_adr0)
#define encryption_key_adr1  0x00318188
#define encryption_key1      (*(volatile unsigned int *)encryption_key_adr1)
#define encryption_key_adr2  0x0031818c
#define encryption_key2      (*(volatile unsigned int *)encryption_key_adr2)
#define encryption_key_adr3  0x00318190
#define encryption_key3      (*(volatile unsigned int *)encryption_key_adr3)
#define pcx_base             0x00318264
#define pc_idx_ptr_adr       0x00318264
#define pc_idx_ptr           (*(volatile unsigned int *)pc_idx_ptr_adr)
#define pcx_ctl_adr          0x00318268
#define pcx_ctl              (*(volatile unsigned int *)pcx_ctl_adr)
#define pcx_pckoff_adr       0x0031826c
#define pcx_pckoff           (*(volatile unsigned int *)pcx_pckoff_adr)
#define pcx_ckoff_adr        0x00318270
#define pcx_ckoff            (*(volatile unsigned int *)pcx_ckoff_adr)
#define pcx_timer_ctl_adr    0x00318274
#define pcx_timer_ctl        (*(volatile unsigned int *)pcx_timer_ctl_adr)
#define pcx_timer_val_adr    0x00318278
#define pcx_timer_val        (*(volatile unsigned int *)pcx_timer_val_adr)
#define pc_acscd_lo_adr      0x0031827c
#define pc_acscd_lo          (*(volatile unsigned int *)pc_acscd_lo_adr)
#define pc_acscd_hi_adr      0x00318280
#define pc_acscd_hi          (*(volatile unsigned int *)pc_acscd_hi_adr)
#define pcx_uap_adr          0x00318284
#define pcx_uap              (*(volatile unsigned int *)pcx_uap_adr)
#define pcx_nap_adr          0x00318288
#define pcx_nap              (*(volatile unsigned int *)pcx_nap_adr)
#define pcx_pbtclk_adr       0x0031828c
#define pcx_pbtclk           (*(volatile unsigned int *)pcx_pbtclk_adr)
#define pcx_btclk_adr        0x00318290
#define pcx_btclk            (*(volatile unsigned int *)pcx_btclk_adr)
#define pcx_last_synctr_adr  0x00318294
#define pcx_last_synctr      (*(volatile unsigned int *)pcx_last_synctr_adr)
#define pcx_nat_ckoff_adr    0x00318298
#define pcx_nat_ckoff        (*(volatile unsigned int *)pcx_nat_ckoff_adr)
#define pcx2_pbtclk_adr      0x0031829c
#define pcx2_pbtclk          (*(volatile unsigned int *)pcx2_pbtclk_adr)
#define pcx2_btclk_adr       0x003182a0
#define pcx2_btclk           (*(volatile unsigned int *)pcx2_btclk_adr)
#define pc_varacscd_lo_adr   0x003182a4
#define pc_varacscd_lo       (*(volatile unsigned int *)pc_varacscd_lo_adr)
#define pc_varacscd_hi_adr   0x003182a8
#define pc_varacscd_hi       (*(volatile unsigned int *)pc_varacscd_hi_adr)
#define btclk_vsync_val1_adr 0x003182ac
#define btclk_vsync_val1     (*(volatile unsigned int *)btclk_vsync_val1_adr)
#define btclk_vsync_val2_adr 0x003182b0
#define btclk_vsync_val2     (*(volatile unsigned int *)btclk_vsync_val2_adr)
#define amodb1_adr           0x003182b8
#define amodb1               (*(volatile unsigned int *)amodb1_adr)
#define amodb2_adr           0x003182bc
#define amodb2               (*(volatile unsigned int *)amodb2_adr)
#define mod_calc_ctl_adr     0x003182c0
#define mod_calc_ctl         (*(volatile unsigned int *)mod_calc_ctl_adr)
#define a_in_amodb1_adr      0x003182c4
#define a_in_amodb1          (*(volatile unsigned int *)a_in_amodb1_adr)
#define b_in_amodb1_adr      0x003182c8
#define b_in_amodb1          (*(volatile unsigned int *)b_in_amodb1_adr)
#define a_in_amodb2_adr      0x003182cc
#define a_in_amodb2          (*(volatile unsigned int *)a_in_amodb2_adr)
#define b_in_amodb2_adr      0x003182d0
#define b_in_amodb2          (*(volatile unsigned int *)b_in_amodb2_adr)
#define pcx_enc_bc_adr       0x00318300
#define pcx_enc_bc           (*(volatile unsigned int *)pcx_enc_bc_adr)
#define pcx_mc_cfg_adr       0x00318304
#define pcx_mc_cfg           (*(volatile unsigned int *)pcx_mc_cfg_adr)
#define pcx_btclk_no_offset_adr 0x00318308
#define pcx_btclk_no_offset  (*(volatile unsigned int *)pcx_btclk_no_offset_adr)
#define cdx_base             0x00318330
#define dc_mss_pc_indx_adr   0x00318330
#define dc_mss_pc_indx       (*(volatile unsigned int *)dc_mss_pc_indx_adr)
#define tx_power_adr         0x00318334
#define tx_power             (*(volatile unsigned int *)tx_power_adr)
#define xcdx_hold_ctl_adr    0x00318338
#define xcdx_hold_ctl        (*(volatile unsigned int *)xcdx_hold_ctl_adr)
#define modemFifoPtr_adr     0x00318364
#define modemFifoPtr         (*(volatile unsigned int *)modemFifoPtr_adr)
#define enh_base             0x00318370
#define task_ind_adr         0x00318370
#define task_ind             (*(volatile unsigned int *)task_ind_adr)
#define btsch_ctl_adr        0x00318374
#define btsch_ctl            (*(volatile unsigned int *)btsch_ctl_adr)
#define wimax_cap_clk_adr    0x00318378
#define wimax_cap_clk        (*(volatile unsigned int *)wimax_cap_clk_adr)
#define wimax_cap_pclk_adr   0x0031837c
#define wimax_cap_pclk       (*(volatile unsigned int *)wimax_cap_pclk_adr)
#define btsch_ctl3_adr       0x00318380
#define btsch_ctl3           (*(volatile unsigned int *)btsch_ctl3_adr)
#define wib_ctr_iv0_adr      0x00318384
#define wib_ctr_iv0          (*(volatile unsigned int *)wib_ctr_iv0_adr)
#define wib_ctr_iv1_adr      0x00318388
#define wib_ctr_iv1          (*(volatile unsigned int *)wib_ctr_iv1_adr)
#define wib_ctr_iv2_adr      0x0031838c
#define wib_ctr_iv2          (*(volatile unsigned int *)wib_ctr_iv2_adr)
#define wib_ctr_iv3_adr      0x00318390
#define wib_ctr_iv3          (*(volatile unsigned int *)wib_ctr_iv3_adr)
#define wib_rtx_ctrl_adr     0x00318394
#define wib_rtx_ctrl         (*(volatile unsigned int *)wib_rtx_ctrl_adr)
#define pc0_hs_cap_clk_adr   0x00318398
#define pc0_hs_cap_clk       (*(volatile unsigned int *)pc0_hs_cap_clk_adr)
#define pc1_hs_cap_clk_adr   0x0031839c
#define pc1_hs_cap_clk       (*(volatile unsigned int *)pc1_hs_cap_clk_adr)
#define pc2_hs_cap_clk_adr   0x003183a0
#define pc2_hs_cap_clk       (*(volatile unsigned int *)pc2_hs_cap_clk_adr)
#define pc3_hs_cap_clk_adr   0x003183a4
#define pc3_hs_cap_clk       (*(volatile unsigned int *)pc3_hs_cap_clk_adr)
#define pc4_hs_cap_clk_adr   0x003183a8
#define pc4_hs_cap_clk       (*(volatile unsigned int *)pc4_hs_cap_clk_adr)
#define enh_ctl_adr          0x003183ac
#define enh_ctl              (*(volatile unsigned int *)enh_ctl_adr)
#define rtx_fifo_ctl_adr     0x003183b0
#define rtx_fifo_ctl         (*(volatile unsigned int *)rtx_fifo_ctl_adr)
#define wib_txpktctr_lo_adr  0x003183b4
#define wib_txpktctr_lo      (*(volatile unsigned int *)wib_txpktctr_lo_adr)
#define wib_txpktctr_hi_adr  0x003183b8
#define wib_txpktctr_hi      (*(volatile unsigned int *)wib_txpktctr_hi_adr)
#define wib_rxpktctr_lo_adr  0x003183bc
#define wib_rxpktctr_lo      (*(volatile unsigned int *)wib_rxpktctr_lo_adr)
#define wib_rxpktctr_hi_adr  0x003183c0
#define wib_rxpktctr_hi      (*(volatile unsigned int *)wib_rxpktctr_hi_adr)
#define tbfc_ctrl_adr        0x003183c4
#define tbfc_ctrl            (*(volatile unsigned int *)tbfc_ctrl_adr)
#define tbfc_trig_pkt_num_adr 0x003183c8
#define tbfc_trig_pkt_num    (*(volatile unsigned int *)tbfc_trig_pkt_num_adr)
#define diag_base            0x00318430
#define di_test_ctl_adr      0x00318430
#define di_test_ctl          (*(volatile unsigned int *)di_test_ctl_adr)
#define di_mod_index_ctl_adr 0x00318434
#define di_mod_index_ctl     (*(volatile unsigned int *)di_mod_index_ctl_adr)
#define di_rf_ctl_adr0       0x00318438
#define di_rf_ctl0           (*(volatile unsigned int *)di_rf_ctl_adr0)
#define di_rf_ctl_adr1       0x0031843c
#define di_rf_ctl1           (*(volatile unsigned int *)di_rf_ctl_adr1)
#define di_tmode_ctl_adr     0x00318440
#define di_tmode_ctl         (*(volatile unsigned int *)di_tmode_ctl_adr)
#define di_tmode_rx_hf_adr   0x00318444
#define di_tmode_rx_hf       (*(volatile unsigned int *)di_tmode_rx_hf_adr)
#define di_tmode_tx_hf_adr   0x00318448
#define di_tmode_tx_hf       (*(volatile unsigned int *)di_tmode_tx_hf_adr)
#define di_lcu_substate_adr0 0x0031844c
#define di_lcu_substate0     (*(volatile unsigned int *)di_lcu_substate_adr0)
#define di_lcu_substate_adr1 0x00318450
#define di_lcu_substate1     (*(volatile unsigned int *)di_lcu_substate_adr1)
#define di_tx_pattern_adr    0x00318470
#define di_tx_pattern        (*(volatile unsigned int *)di_tx_pattern_adr)
#define di_tx_pattern_sel_adr 0x00318474
#define di_tx_pattern_sel    (*(volatile unsigned int *)di_tx_pattern_sel_adr)
#define sco_pc_context_adr   0x00318478
#define sco_pc_context       (*(volatile unsigned int *)sco_pc_context_adr)
#define di_phy_fpga_vid_adr  0x0031847c
#define di_phy_fpga_vid      (*(volatile unsigned int *)di_phy_fpga_vid_adr)
#define di_phy_fpga_48m_dcm_adr 0x00318480
#define di_phy_fpga_48m_dcm  (*(volatile unsigned int *)di_phy_fpga_48m_dcm_adr)
#define fpga_txconfx_gen_ctl_adr 0x00318488
#define fpga_txconfx_gen_ctl (*(volatile unsigned int *)fpga_txconfx_gen_ctl_adr)
#define park_base            0x003184ac
#define scox_d_sco_adr       0x003184b0
#define scox_d_sco           (*(volatile unsigned int *)scox_d_sco_adr)
#define scox_t_sco_adr       0x003184b4
#define scox_t_sco           (*(volatile unsigned int *)scox_t_sco_adr)
#define pcx_n_poll_adr       0x003184b8
#define pcx_n_poll           (*(volatile unsigned int *)pcx_n_poll_adr)
#define park_available01_adr 0x003184bc
#define park_available01     (*(volatile unsigned int *)park_available01_adr)
#define park_ctl_adr         0x003184c0
#define park_ctl             (*(volatile unsigned int *)park_ctl_adr)
#define park_status_adr      0x003184c4
#define park_status          (*(volatile unsigned int *)park_status_adr)
#define sri_base             0x00318510
#define dc_sri_ist_bit_adr   0x00318510
#define dc_sri_ist_bit       (*(volatile unsigned int *)dc_sri_ist_bit_adr)
#define dc_sri_ds_bit_adr    0x00318514
#define dc_sri_ds_bit        (*(volatile unsigned int *)dc_sri_ds_bit_adr)
#define dc_sri_ist_adr0      0x00318518
#define dc_sri_ist0          (*(volatile unsigned int *)dc_sri_ist_adr0)
#define dc_sri_ist_adr1      0x0031851c
#define dc_sri_ist1          (*(volatile unsigned int *)dc_sri_ist_adr1)
#define dc_sri_ds_adr0       0x00318520
#define dc_sri_ds0           (*(volatile unsigned int *)dc_sri_ds_adr0)
#define dc_sri_ds_adr1       0x00318524
#define dc_sri_ds1           (*(volatile unsigned int *)dc_sri_ds_adr1)
#define dc_sri_ds_adr2       0x00318528
#define dc_sri_ds2           (*(volatile unsigned int *)dc_sri_ds_adr2)
#define dc_sri_ds_adr3       0x0031852c
#define dc_sri_ds3           (*(volatile unsigned int *)dc_sri_ds_adr3)
#define dc_sri_jtag_access_adr 0x00318530
#define dc_sri_jtag_access   (*(volatile unsigned int *)dc_sri_jtag_access_adr)
#define dc_sri_rf_ctl_adr0   0x00318534
#define dc_sri_rf_ctl0       (*(volatile unsigned int *)dc_sri_rf_ctl_adr0)
#define dc_sri_rf_ctl_adr1   0x00318538
#define dc_sri_rf_ctl1       (*(volatile unsigned int *)dc_sri_rf_ctl_adr1)
#define dc_sri_rssi_val_adr  0x0031853c
#define dc_sri_rssi_val      (*(volatile unsigned int *)dc_sri_rssi_val_adr)
#define dc_sri_pwr_ctl_adr   0x00318540
#define dc_sri_pwr_ctl       (*(volatile unsigned int *)dc_sri_pwr_ctl_adr)
#define dc_t_synth_pu_adr    0x00318544
#define dc_t_synth_pu        (*(volatile unsigned int *)dc_t_synth_pu_adr)
#define dc_t_sl_ctl_adr      0x00318548
#define dc_t_sl_ctl          (*(volatile unsigned int *)dc_t_sl_ctl_adr)
#define dc_t_pa_ramp_adr     0x0031854c
#define dc_t_pa_ramp         (*(volatile unsigned int *)dc_t_pa_ramp_adr)
#define dc_t_pa_down_adr     0x00318550
#define dc_t_pa_down         (*(volatile unsigned int *)dc_t_pa_down_adr)
#define dc_t_tx_pu_adr       0x00318554
#define dc_t_tx_pu           (*(volatile unsigned int *)dc_t_tx_pu_adr)
#define dc_t_rx_pu_adr       0x00318558
#define dc_t_rx_pu           (*(volatile unsigned int *)dc_t_rx_pu_adr)
#define dc_available01_adr   0x0031855c
#define dc_available01       (*(volatile unsigned int *)dc_available01_adr)
#define dc_sri_schp0_sel_adr 0x00318560
#define dc_sri_schp0_sel     (*(volatile unsigned int *)dc_sri_schp0_sel_adr)
#define dc_sri_schp1_sel_adr 0x00318564
#define dc_sri_schp1_sel     (*(volatile unsigned int *)dc_sri_schp1_sel_adr)
#define dc_sri_schp2_sel_adr 0x00318568
#define dc_sri_schp2_sel     (*(volatile unsigned int *)dc_sri_schp2_sel_adr)
#define dc_sri_schp3_sel_adr 0x0031856c
#define dc_sri_schp3_sel     (*(volatile unsigned int *)dc_sri_schp3_sel_adr)
#define afh_base             0x00318590
#define dc_afh_ctl_adr       0x003185b0
#define dc_afh_ctl           (*(volatile unsigned int *)dc_afh_ctl_adr)
#define dc_ind_d_ptr_adr     0x003185b4
#define dc_ind_d_ptr         (*(volatile unsigned int *)dc_ind_d_ptr_adr)
#define dc_hd_acc_cfg_adr    0x003185b8
#define dc_hd_acc_cfg        (*(volatile unsigned int *)dc_hd_acc_cfg_adr)
#define dc_up_opcode_adr     0x003185bc
#define dc_up_opcode         (*(volatile unsigned int *)dc_up_opcode_adr)
#define up_reg_b_adr         0x003185c0
#define up_reg_b             (*(volatile unsigned int *)up_reg_b_adr)
#define up_reg_c_adr         0x003185c4
#define up_reg_c             (*(volatile unsigned int *)up_reg_c_adr)
#define up_reg_d_adr         0x003185c8
#define up_reg_d             (*(volatile unsigned int *)up_reg_d_adr)
#define dc_reg_a_adr         0x003185cc
#define dc_reg_a             (*(volatile unsigned int *)dc_reg_a_adr)
#define dc_reg_b_adr         0x003185d0
#define dc_reg_b             (*(volatile unsigned int *)dc_reg_b_adr)
#define dc_reg_c_adr         0x003185d4
#define dc_reg_c             (*(volatile unsigned int *)dc_reg_c_adr)
#define dc_reg_d_adr         0x003185d8
#define dc_reg_d             (*(volatile unsigned int *)dc_reg_d_adr)
#define dc_opcode_adr0       0x003185dc
#define dc_opcode0           (*(volatile unsigned int *)dc_opcode_adr0)
#define dc_opcode_adr1       0x003185e0
#define dc_opcode1           (*(volatile unsigned int *)dc_opcode_adr1)
#define dc_opcode_adr2       0x003185e4
#define dc_opcode2           (*(volatile unsigned int *)dc_opcode_adr2)
#define dc_opcode_adr3       0x003185e8
#define dc_opcode3           (*(volatile unsigned int *)dc_opcode_adr3)
#define dc_opcode_adr4       0x003185ec
#define dc_opcode4           (*(volatile unsigned int *)dc_opcode_adr4)
#define dc_opcode_adr5       0x003185f0
#define dc_opcode5           (*(volatile unsigned int *)dc_opcode_adr5)
#define dc_opcode_adr6       0x003185f4
#define dc_opcode6           (*(volatile unsigned int *)dc_opcode_adr6)
#define dc_opcode_adr7       0x003185f8
#define dc_opcode7           (*(volatile unsigned int *)dc_opcode_adr7)
#define dc_opcode_adr8       0x003185fc
#define dc_opcode8           (*(volatile unsigned int *)dc_opcode_adr8)
#define dc_opcode_adr9       0x00318600
#define dc_opcode9           (*(volatile unsigned int *)dc_opcode_adr9)
#define dc_opcode_adr10      0x00318604
#define dc_opcode10          (*(volatile unsigned int *)dc_opcode_adr10)
#define dc_opcode_adr11      0x00318608
#define dc_opcode11          (*(volatile unsigned int *)dc_opcode_adr11)
#define dc_opcode_adr12      0x0031860c
#define dc_opcode12          (*(volatile unsigned int *)dc_opcode_adr12)
#define dc_opcode_adr13      0x00318610
#define dc_opcode13          (*(volatile unsigned int *)dc_opcode_adr13)
#define dc_opcode_adr14      0x00318614
#define dc_opcode14          (*(volatile unsigned int *)dc_opcode_adr14)
#define dc_opcode_adr15      0x00318618
#define dc_opcode15          (*(volatile unsigned int *)dc_opcode_adr15)
#define dc_log_data_adr0     0x0031861c
#define dc_log_data0         (*(volatile unsigned int *)dc_log_data_adr0)
#define dc_log_data_adr1     0x00318620
#define dc_log_data1         (*(volatile unsigned int *)dc_log_data_adr1)
#define dc_log_data_adr2     0x00318624
#define dc_log_data2         (*(volatile unsigned int *)dc_log_data_adr2)
#define dc_log_data_adr3     0x00318628
#define dc_log_data3         (*(volatile unsigned int *)dc_log_data_adr3)
#define dc_log_data_adr4     0x0031862c
#define dc_log_data4         (*(volatile unsigned int *)dc_log_data_adr4)
#define dc_fhout_adr         0x00318630
#define dc_fhout             (*(volatile unsigned int *)dc_fhout_adr)
#define dc_fk_remap_adr      0x00318634
#define dc_fk_remap          (*(volatile unsigned int *)dc_fk_remap_adr)
#define dc_misc_ctl_adr      0x00318638
#define dc_misc_ctl          (*(volatile unsigned int *)dc_misc_ctl_adr)
#define dc_x_clk_adr         0x0031863c
#define dc_x_clk             (*(volatile unsigned int *)dc_x_clk_adr)
#define used_carriers_32t0_adr 0x00318640
#define used_carriers_32t0   (*(volatile unsigned int *)used_carriers_32t0_adr)
#define used_carriers_64t0_adr 0x00318644
#define used_carriers_64t0   (*(volatile unsigned int *)used_carriers_64t0_adr)
#define used_carriers_79t0_adr 0x00318648
#define used_carriers_79t0   (*(volatile unsigned int *)used_carriers_79t0_adr)
#define used_carriers_32t1_adr 0x0031864c
#define used_carriers_32t1   (*(volatile unsigned int *)used_carriers_32t1_adr)
#define used_carriers_64t1_adr 0x00318650
#define used_carriers_64t1   (*(volatile unsigned int *)used_carriers_64t1_adr)
#define used_carriers_79t1_adr 0x00318654
#define used_carriers_79t1   (*(volatile unsigned int *)used_carriers_79t1_adr)
#define afh_mem_start_adr    0x00318800
#define afh_mem_start        (*(volatile unsigned int *)afh_mem_start_adr)
#define afh_mem_end_adr      0x00318940
#define afh_mem_end          (*(volatile unsigned int *)afh_mem_end_adr)
#define dc_saved_fhout_adr   0x00318944
#define dc_saved_fhout       (*(volatile unsigned int *)dc_saved_fhout_adr)
#define dc_llr_fhs_pyld0_adr 0x00318948
#define dc_llr_fhs_pyld0     (*(volatile unsigned int *)dc_llr_fhs_pyld0_adr)
#define dc_llr_fhs_pyld1_adr 0x0031894c
#define dc_llr_fhs_pyld1     (*(volatile unsigned int *)dc_llr_fhs_pyld1_adr)
#define dc_llr_fhs_pyld2_adr 0x00318950
#define dc_llr_fhs_pyld2     (*(volatile unsigned int *)dc_llr_fhs_pyld2_adr)
#define dc_llr_fhs_pyld3_adr 0x00318954
#define dc_llr_fhs_pyld3     (*(volatile unsigned int *)dc_llr_fhs_pyld3_adr)
#define dc_llr_fhs_pyld4_adr 0x00318958
#define dc_llr_fhs_pyld4     (*(volatile unsigned int *)dc_llr_fhs_pyld4_adr)
#define dc_llr_bd_adr0       0x0031895c
#define dc_llr_bd0           (*(volatile unsigned int *)dc_llr_bd_adr0)
#define dc_llr_bd_adr1       0x00318960
#define dc_llr_bd1           (*(volatile unsigned int *)dc_llr_bd_adr1)
#define rtx_prog_rxdcnt_adr  0x00318964
#define rtx_prog_rxdcnt      (*(volatile unsigned int *)rtx_prog_rxdcnt_adr)
#define rtx_base             0x00318980
#define null_def_adr         0x00318988
#define null_def             (*(volatile unsigned int *)null_def_adr)
#define poll_def_adr         0x0031898c
#define poll_def             (*(volatile unsigned int *)poll_def_adr)
#define fhs_def_adr          0x00318990
#define fhs_def              (*(volatile unsigned int *)fhs_def_adr)
#define dm1_def_adr          0x00318994
#define dm1_def              (*(volatile unsigned int *)dm1_def_adr)
#define dh1_def_adr          0x00318998
#define dh1_def              (*(volatile unsigned int *)dh1_def_adr)
#define dm3_def_adr          0x0031899c
#define dm3_def              (*(volatile unsigned int *)dm3_def_adr)
#define dh3_def_adr          0x003189a0
#define dh3_def              (*(volatile unsigned int *)dh3_def_adr)
#define dm5_def_adr          0x003189a4
#define dm5_def              (*(volatile unsigned int *)dm5_def_adr)
#define dh5_def_adr          0x003189a8
#define dh5_def              (*(volatile unsigned int *)dh5_def_adr)
#define dh1_2_def_adr        0x003189ac
#define dh1_2_def            (*(volatile unsigned int *)dh1_2_def_adr)
#define dh3_2_def_adr        0x003189b0
#define dh3_2_def            (*(volatile unsigned int *)dh3_2_def_adr)
#define dh5_2_def_adr        0x003189b4
#define dh5_2_def            (*(volatile unsigned int *)dh5_2_def_adr)
#define dh1_3_def_adr        0x003189b8
#define dh1_3_def            (*(volatile unsigned int *)dh1_3_def_adr)
#define dh3_3_def_adr        0x003189bc
#define dh3_3_def            (*(volatile unsigned int *)dh3_3_def_adr)
#define dh5_3_def_adr        0x003189c0
#define dh5_3_def            (*(volatile unsigned int *)dh5_3_def_adr)
#define aux1_def_adr         0x003189c4
#define aux1_def             (*(volatile unsigned int *)aux1_def_adr)
#define hv1_def_adr          0x003189c8
#define hv1_def              (*(volatile unsigned int *)hv1_def_adr)
#define hv2_def_adr          0x003189cc
#define hv2_def              (*(volatile unsigned int *)hv2_def_adr)
#define hv3_def_adr          0x003189d0
#define hv3_def              (*(volatile unsigned int *)hv3_def_adr)
#define ev3_def_adr          0x003189d4
#define ev3_def              (*(volatile unsigned int *)ev3_def_adr)
#define ev4_def_adr          0x003189d8
#define ev4_def              (*(volatile unsigned int *)ev4_def_adr)
#define ev5_def_adr          0x003189dc
#define ev5_def              (*(volatile unsigned int *)ev5_def_adr)
#define dv_def_adr           0x003189e0
#define dv_def               (*(volatile unsigned int *)dv_def_adr)
#define ev3_2_def_adr        0x003189e4
#define ev3_2_def            (*(volatile unsigned int *)ev3_2_def_adr)
#define ev5_2_def_adr        0x003189e8
#define ev5_2_def            (*(volatile unsigned int *)ev5_2_def_adr)
#define ev3_3_def_adr        0x003189ec
#define ev3_3_def            (*(volatile unsigned int *)ev3_3_def_adr)
#define ev5_3_def_adr        0x003189f0
#define ev5_3_def            (*(volatile unsigned int *)ev5_3_def_adr)
#define ps_def_adr           0x003189f4
#define ps_def               (*(volatile unsigned int *)ps_def_adr)
#define rsv0_def_adr         0x003189f8
#define rsv0_def             (*(volatile unsigned int *)rsv0_def_adr)
#define rsv1_def_adr         0x003189fc
#define rsv1_def             (*(volatile unsigned int *)rsv1_def_adr)
#define pkt_rpl_def_2_adr    0x00318a00
#define pkt_rpl_def_2        (*(volatile unsigned int *)pkt_rpl_def_2_adr)
#define pkt_rpl_def_3_adr    0x00318a04
#define pkt_rpl_def_3        (*(volatile unsigned int *)pkt_rpl_def_3_adr)
#define pkt_rpl_def_4_adr    0x00318a08
#define pkt_rpl_def_4        (*(volatile unsigned int *)pkt_rpl_def_4_adr)
#define pkt_rpl_def_5_adr    0x00318a0c
#define pkt_rpl_def_5        (*(volatile unsigned int *)pkt_rpl_def_5_adr)
#define pkt_rpl_def_6_adr    0x00318a10
#define pkt_rpl_def_6        (*(volatile unsigned int *)pkt_rpl_def_6_adr)
#define pkt_rpl_def_7_adr    0x00318a14
#define pkt_rpl_def_7        (*(volatile unsigned int *)pkt_rpl_def_7_adr)
#define pkt_rpl_def_8_adr    0x00318a18
#define pkt_rpl_def_8        (*(volatile unsigned int *)pkt_rpl_def_8_adr)
#define pkt_rpl_def_9_adr    0x00318a1c
#define pkt_rpl_def_9        (*(volatile unsigned int *)pkt_rpl_def_9_adr)
#define pkt_rpl_def_10_adr   0x00318a20
#define pkt_rpl_def_10       (*(volatile unsigned int *)pkt_rpl_def_10_adr)
#define pkt_rpl_def_11_adr   0x00318a24
#define pkt_rpl_def_11       (*(volatile unsigned int *)pkt_rpl_def_11_adr)
#define pkt_rpl_def_12_adr   0x00318a28
#define pkt_rpl_def_12       (*(volatile unsigned int *)pkt_rpl_def_12_adr)
#define pkt_rpl_def_13_adr   0x00318a2c
#define pkt_rpl_def_13       (*(volatile unsigned int *)pkt_rpl_def_13_adr)
#define pkt_rpl_def_14_adr   0x00318a30
#define pkt_rpl_def_14       (*(volatile unsigned int *)pkt_rpl_def_14_adr)
#define pkt_rpl_def_15_adr   0x00318a34
#define pkt_rpl_def_15       (*(volatile unsigned int *)pkt_rpl_def_15_adr)
#define prog_bt_trigger_cfg_adr 0x00318a38
#define prog_bt_trigger_cfg  (*(volatile unsigned int *)prog_bt_trigger_cfg_adr)
#define prog_bt_int0_cfg_adr 0x00318a3c
#define prog_bt_int0_cfg     (*(volatile unsigned int *)prog_bt_int0_cfg_adr)
#define prog_bt_int1_cfg_adr 0x00318a40
#define prog_bt_int1_cfg     (*(volatile unsigned int *)prog_bt_int1_cfg_adr)
#define prog_bt_int2_cfg_adr 0x00318a44
#define prog_bt_int2_cfg     (*(volatile unsigned int *)prog_bt_int2_cfg_adr)
#define prog_bt_int3_cfg_adr 0x00318a48
#define prog_bt_int3_cfg     (*(volatile unsigned int *)prog_bt_int3_cfg_adr)
#define tx_pkt_info_adr      0x00318a4c
#define tx_pkt_info          (*(volatile unsigned int *)tx_pkt_info_adr)
#define tx_pkt_pyld_hdr_adr  0x00318a50
#define tx_pkt_pyld_hdr      (*(volatile unsigned int *)tx_pkt_pyld_hdr_adr)
#define tx_fhs_pkt_hdr_adr   0x00318a54
#define tx_fhs_pkt_hdr       (*(volatile unsigned int *)tx_fhs_pkt_hdr_adr)
#define tx_dft_pkt_hdr_adr   0x00318a58
#define tx_dft_pkt_hdr       (*(volatile unsigned int *)tx_dft_pkt_hdr_adr)
#define tx_corrupt_cfg_adr   0x00318a5c
#define tx_corrupt_cfg       (*(volatile unsigned int *)tx_corrupt_cfg_adr)
#define logical_conn_info_adr 0x00318a60
#define logical_conn_info    (*(volatile unsigned int *)logical_conn_info_adr)
#define txd_rxd_sco_length_adr 0x00318a64
#define txd_rxd_sco_length   (*(volatile unsigned int *)txd_rxd_sco_length_adr)
#define txd_rxd_ctl_cfg_adr  0x00318a68
#define txd_rxd_ctl_cfg      (*(volatile unsigned int *)txd_rxd_ctl_cfg_adr)
#define rxd_chk_notx_fhs_cfg_adr 0x00318a6c
#define rxd_chk_notx_fhs_cfg (*(volatile unsigned int *)rxd_chk_notx_fhs_cfg_adr)
#define rxd_chk_notx_acl_cfg_adr 0x00318a70
#define rxd_chk_notx_acl_cfg (*(volatile unsigned int *)rxd_chk_notx_acl_cfg_adr)
#define rxd_chk_notx_sco_cfg_adr 0x00318a74
#define rxd_chk_notx_sco_cfg (*(volatile unsigned int *)rxd_chk_notx_sco_cfg_adr)
#define rxd_hdr_ok_cfg_adr   0x00318a78
#define rxd_hdr_ok_cfg       (*(volatile unsigned int *)rxd_hdr_ok_cfg_adr)
#define pkt_rx_1slot_max_cnt_adr 0x00318a7c
#define pkt_rx_1slot_max_cnt (*(volatile unsigned int *)pkt_rx_1slot_max_cnt_adr)
#define pkt_rx_3slot_max_cnt_adr 0x00318a80
#define pkt_rx_3slot_max_cnt (*(volatile unsigned int *)pkt_rx_3slot_max_cnt_adr)
#define pkt_rx_5slot_max_cnt_adr 0x00318a84
#define pkt_rx_5slot_max_cnt (*(volatile unsigned int *)pkt_rx_5slot_max_cnt_adr)
#define multi_s_term_cnt_adr 0x00318a88
#define multi_s_term_cnt     (*(volatile unsigned int *)multi_s_term_cnt_adr)
#define ips_synch_win_adr    0x00318a8c
#define ips_synch_win        (*(volatile unsigned int *)ips_synch_win_adr)
#define synch_win_adr        0x00318a90
#define synch_win            (*(volatile unsigned int *)synch_win_adr)
#define synch_trigger_offset_adr 0x00318a94
#define synch_trigger_offset (*(volatile unsigned int *)synch_trigger_offset_adr)
#define rtx_dma_ctl_adr      0x00318a98
#define rtx_dma_ctl          (*(volatile unsigned int *)rtx_dma_ctl_adr)
#define psk_synch_word_lo_adr 0x00318a9c
#define psk_synch_word_lo    (*(volatile unsigned int *)psk_synch_word_lo_adr)
#define psk_synch_word_hi_adr 0x00318aa0
#define psk_synch_word_hi    (*(volatile unsigned int *)psk_synch_word_hi_adr)
#define psk_pyld_mod_cfg_adr 0x00318aa4
#define psk_pyld_mod_cfg     (*(volatile unsigned int *)psk_pyld_mod_cfg_adr)
#define pkt_hdr_status_adr   0x00318aa8
#define pkt_hdr_status       (*(volatile unsigned int *)pkt_hdr_status_adr)
#define pkt_log_adr          0x00318aac
#define pkt_log              (*(volatile unsigned int *)pkt_log_adr)
#define pkt_picky_cnt_th_adr 0x00318ab0
#define pkt_picky_cnt_th     (*(volatile unsigned int *)pkt_picky_cnt_th_adr)
#define pyld_picky_err_cnt_adr 0x00318ab4
#define pyld_picky_err_cnt   (*(volatile unsigned int *)pyld_picky_err_cnt_adr)
#define wib_wh_list2_lo_adr  0x00318ab8
#define wib_wh_list2_lo      (*(volatile unsigned int *)wib_wh_list2_lo_adr)
#define wib_wh_list2_hi_adr  0x00318abc
#define wib_wh_list2_hi      (*(volatile unsigned int *)wib_wh_list2_hi_adr)
#define wib_wh_list3_lo_adr  0x00318ac0
#define wib_wh_list3_lo      (*(volatile unsigned int *)wib_wh_list3_lo_adr)
#define wib_wh_list3_hi_adr  0x00318ac4
#define wib_wh_list3_hi      (*(volatile unsigned int *)wib_wh_list3_hi_adr)
#define wib_wh_list4_lo_adr  0x00318ac8
#define wib_wh_list4_lo      (*(volatile unsigned int *)wib_wh_list4_lo_adr)
#define wib_wh_list4_hi_adr  0x00318acc
#define wib_wh_list4_hi      (*(volatile unsigned int *)wib_wh_list4_hi_adr)
#define wib_wh_list5_lo_adr  0x00318ad0
#define wib_wh_list5_lo      (*(volatile unsigned int *)wib_wh_list5_lo_adr)
#define wib_wh_list5_hi_adr  0x00318ad4
#define wib_wh_list5_hi      (*(volatile unsigned int *)wib_wh_list5_hi_adr)
#define wib_wh_list6_lo_adr  0x00318ad8
#define wib_wh_list6_lo      (*(volatile unsigned int *)wib_wh_list6_lo_adr)
#define wib_wh_list6_hi_adr  0x00318adc
#define wib_wh_list6_hi      (*(volatile unsigned int *)wib_wh_list6_hi_adr)
#define wib_rtx_ctrl2_adr    0x00318ae0
#define wib_rtx_ctrl2        (*(volatile unsigned int *)wib_rtx_ctrl2_adr)
#define rtx_mem_start_adr    0x00370000
#define rtx_mem_start        (*(volatile unsigned int *)rtx_mem_start_adr)
#define rtx_ram_address_adr  0x00370040
#define rtx_ram_address      (*(volatile unsigned int *)rtx_ram_address_adr)
#define rtx_mem_start1_adr   0x00370400
#define rtx_mem_start1       (*(volatile unsigned int *)rtx_mem_start1_adr)
#define rtx_mem_start2_adr   0x00370800
#define rtx_mem_start2       (*(volatile unsigned int *)rtx_mem_start2_adr)
#define rtx_mem_end_adr      0x00370900
#define rtx_mem_end          (*(volatile unsigned int *)rtx_mem_end_adr)
#define prog_wht_seed_eir_adr 0x00319304
#define prog_wht_seed_eir    (*(volatile unsigned int *)prog_wht_seed_eir_adr)
#define eci_status_adr       0x00319308
#define eci_status           (*(volatile unsigned int *)eci_status_adr)
#define eci_status_int_en_adr 0x0031930c
#define eci_status_int_en    (*(volatile unsigned int *)eci_status_int_en_adr)
#define eci_edgtr_mode_adr   0x00319310
#define eci_edgtr_mode       (*(volatile unsigned int *)eci_edgtr_mode_adr)
#define eci_dual_edge_adr    0x00319314
#define eci_dual_edge        (*(volatile unsigned int *)eci_dual_edge_adr)
#define eci_int_mask_adr     0x00319318
#define eci_int_mask         (*(volatile unsigned int *)eci_int_mask_adr)
#define wib_tx_pyld_info_adr 0x0031931c
#define wib_tx_pyld_info     (*(volatile unsigned int *)wib_tx_pyld_info_adr)
#define wib_ctrl_reg_adr     0x00319320
#define wib_ctrl_reg         (*(volatile unsigned int *)wib_ctrl_reg_adr)
#define wib_access_adr       0x00319324
#define wib_access           (*(volatile unsigned int *)wib_access_adr)
#define wib_advA_lo_adr      0x00319328
#define wib_advA_lo          (*(volatile unsigned int *)wib_advA_lo_adr)
#define wib_advA_hi_adr      0x0031932c
#define wib_advA_hi          (*(volatile unsigned int *)wib_advA_hi_adr)
#define wib_TxA_lo_adr       0x00319330
#define wib_TxA_lo           (*(volatile unsigned int *)wib_TxA_lo_adr)
#define wib_TxA_hi_adr       0x00319334
#define wib_TxA_hi           (*(volatile unsigned int *)wib_TxA_hi_adr)
#define wib_InterLat_timer_adr 0x00319338
#define wib_InterLat_timer   (*(volatile unsigned int *)wib_InterLat_timer_adr)
#define wib_T0_Cnt_adr       0x0031933c
#define wib_T0_Cnt           (*(volatile unsigned int *)wib_T0_Cnt_adr)
#define wib_coex_win0_adr    0x00319340
#define wib_coex_win0        (*(volatile unsigned int *)wib_coex_win0_adr)
#define wib_coex_win1_adr    0x00319344
#define wib_coex_win1        (*(volatile unsigned int *)wib_coex_win1_adr)
#define wib_rx_status_adr    0x00319348
#define wib_rx_status        (*(volatile unsigned int *)wib_rx_status_adr)
#define wib_ScanA_lo_adr     0x0031934c
#define wib_ScanA_lo         (*(volatile unsigned int *)wib_ScanA_lo_adr)
#define wib_ScanA_hi_adr     0x00319350
#define wib_ScanA_hi         (*(volatile unsigned int *)wib_ScanA_hi_adr)
#define wib_conn_lfsr_adr    0x00319354
#define wib_conn_lfsr        (*(volatile unsigned int *)wib_conn_lfsr_adr)
#define wib_adv_acs_adr      0x00319358
#define wib_adv_acs          (*(volatile unsigned int *)wib_adv_acs_adr)
#define wib_pkt_log_adr      0x0031935c
#define wib_pkt_log          (*(volatile unsigned int *)wib_pkt_log_adr)
#define wib_InitA_lo_adr     0x00319360
#define wib_InitA_lo         (*(volatile unsigned int *)wib_InitA_lo_adr)
#define wib_InitA_hi_adr     0x00319364
#define wib_InitA_hi         (*(volatile unsigned int *)wib_InitA_hi_adr)
#define wib_AA_adr           0x00319368
#define wib_AA               (*(volatile unsigned int *)wib_AA_adr)
#define wib_pkt_end_max_cnt_adr 0x0031936c
#define wib_pkt_end_max_cnt  (*(volatile unsigned int *)wib_pkt_end_max_cnt_adr)
#define wib_rtx_start_adr    0x00319370
#define wib_rtx_start        (*(volatile unsigned int *)wib_rtx_start_adr)
#define wib_repl_pkt_adr     0x00319374
#define wib_repl_pkt         (*(volatile unsigned int *)wib_repl_pkt_adr)
#define wib_wh_list1_lo_adr  0x00319378
#define wib_wh_list1_lo      (*(volatile unsigned int *)wib_wh_list1_lo_adr)
#define wib_wh_list1_hi_adr  0x0031937c
#define wib_wh_list1_hi      (*(volatile unsigned int *)wib_wh_list1_hi_adr)
#define pkt_rx_max_slot_cnt_1_adr 0x00319380
#define pkt_rx_max_slot_cnt_1 (*(volatile unsigned int *)pkt_rx_max_slot_cnt_1_adr)
#define pkt_rx_max_slot_cnt_2_adr 0x00319384
#define pkt_rx_max_slot_cnt_2 (*(volatile unsigned int *)pkt_rx_max_slot_cnt_2_adr)
#define pkt_rx_max_slot_cnt_3_adr 0x00319388
#define pkt_rx_max_slot_cnt_3 (*(volatile unsigned int *)pkt_rx_max_slot_cnt_3_adr)
#define pkt_rx_max_slot_cnt_4_adr 0x0031938c
#define pkt_rx_max_slot_cnt_4 (*(volatile unsigned int *)pkt_rx_max_slot_cnt_4_adr)
#define pkt_rx_max_slot_cnt_5_adr 0x00319390
#define pkt_rx_max_slot_cnt_5 (*(volatile unsigned int *)pkt_rx_max_slot_cnt_5_adr)
#define pkt_rx_max_slot_cnt_6_adr 0x00319394
#define pkt_rx_max_slot_cnt_6 (*(volatile unsigned int *)pkt_rx_max_slot_cnt_6_adr)
#define pkt_rx_max_slot_cnt_7_adr 0x00319398
#define pkt_rx_max_slot_cnt_7 (*(volatile unsigned int *)pkt_rx_max_slot_cnt_7_adr)
#define pkt_rx_max_slot_ctrl_adr 0x0031939c
#define pkt_rx_max_slot_ctrl (*(volatile unsigned int *)pkt_rx_max_slot_ctrl_adr)
#define wib_chan_adr         0x003193a0
#define wib_chan             (*(volatile unsigned int *)wib_chan_adr)
#define wib_seed_adr         0x003193a4
#define wib_seed             (*(volatile unsigned int *)wib_seed_adr)
#define wib_ses_key0_adr     0x003193a8
#define wib_ses_key0         (*(volatile unsigned int *)wib_ses_key0_adr)
#define wib_ses_key1_adr     0x003193ac
#define wib_ses_key1         (*(volatile unsigned int *)wib_ses_key1_adr)
#define wib_ses_key2_adr     0x003193b0
#define wib_ses_key2         (*(volatile unsigned int *)wib_ses_key2_adr)
#define wib_ses_key3_adr     0x003193b4
#define wib_ses_key3         (*(volatile unsigned int *)wib_ses_key3_adr)
#define wib_aes_keyvalid_adr 0x003193b8
#define wib_aes_keyvalid     (*(volatile unsigned int *)wib_aes_keyvalid_adr)
#define wib_ctr0_iv0_adr     0x003193bc
#define wib_ctr0_iv0         (*(volatile unsigned int *)wib_ctr0_iv0_adr)
#define wib_ctr0_iv1_adr     0x003193c0
#define wib_ctr0_iv1         (*(volatile unsigned int *)wib_ctr0_iv1_adr)
#define wib_ctr0_iv2_adr     0x003193c4
#define wib_ctr0_iv2         (*(volatile unsigned int *)wib_ctr0_iv2_adr)
#define wib_ctr0_iv3_adr     0x003193c8
#define wib_ctr0_iv3         (*(volatile unsigned int *)wib_ctr0_iv3_adr)
#define wib_ctr2_iv0_adr     0x003193cc
#define wib_ctr2_iv0         (*(volatile unsigned int *)wib_ctr2_iv0_adr)
#define wib_ctr2_iv1_adr     0x003193d0
#define wib_ctr2_iv1         (*(volatile unsigned int *)wib_ctr2_iv1_adr)
#define wib_ctr2_iv2_adr     0x003193d4
#define wib_ctr2_iv2         (*(volatile unsigned int *)wib_ctr2_iv2_adr)
#define wib_ctr2_iv3_adr     0x003193d8
#define wib_ctr2_iv3         (*(volatile unsigned int *)wib_ctr2_iv3_adr)
#define wib_rdctr0_iv0_adr   0x003193dc
#define wib_rdctr0_iv0       (*(volatile unsigned int *)wib_rdctr0_iv0_adr)
#define wib_rdctr0_iv1_adr   0x003193e0
#define wib_rdctr0_iv1       (*(volatile unsigned int *)wib_rdctr0_iv1_adr)
#define wib_rdctr0_iv2_adr   0x003193e4
#define wib_rdctr0_iv2       (*(volatile unsigned int *)wib_rdctr0_iv2_adr)
#define wib_rdctr0_iv3_adr   0x003193e8
#define wib_rdctr0_iv3       (*(volatile unsigned int *)wib_rdctr0_iv3_adr)
#define wib_aes_status_adr   0x003193ec
#define wib_aes_status       (*(volatile unsigned int *)wib_aes_status_adr)
#define wib_aes2_status_adr  0x003193f0
#define wib_aes2_status      (*(volatile unsigned int *)wib_aes2_status_adr)
#define wib_mic_b0_adr       0x003193f4
#define wib_mic_b0           (*(volatile unsigned int *)wib_mic_b0_adr)
#define wib_error_inj_adr    0x003193f8
#define wib_error_inj        (*(volatile unsigned int *)wib_error_inj_adr)
#define apu_base             0x00319400
#define apu_control_adr      0x00319400
#define apu_control          (*(volatile unsigned int *)apu_control_adr)
#define apu_mode_adr         0x00319404
#define apu_mode             (*(volatile unsigned int *)apu_mode_adr)
#define apu_stuff_length_adr 0x00319408
#define apu_stuff_length     (*(volatile unsigned int *)apu_stuff_length_adr)
#define sco_idx_ptr_adr      0x0031940c
#define sco_idx_ptr          (*(volatile unsigned int *)sco_idx_ptr_adr)
#define sco0_rx_decim_adr    0x00319410
#define sco0_rx_decim        (*(volatile unsigned int *)sco0_rx_decim_adr)
#define sco0_rx_cvsd_adr0    0x00319414
#define sco0_rx_cvsd0        (*(volatile unsigned int *)sco0_rx_cvsd_adr0)
#define sco0_rx_cvsd_adr1    0x00319418
#define sco0_rx_cvsd1        (*(volatile unsigned int *)sco0_rx_cvsd_adr1)
#define sco0_tx_intrp_adr    0x0031941c
#define sco0_tx_intrp        (*(volatile unsigned int *)sco0_tx_intrp_adr)
#define sco0_tx_cvsd_adr0    0x00319420
#define sco0_tx_cvsd0        (*(volatile unsigned int *)sco0_tx_cvsd_adr0)
#define sco0_tx_cvsd_adr1    0x00319424
#define sco0_tx_cvsd1        (*(volatile unsigned int *)sco0_tx_cvsd_adr1)
#define sco1_rx_decim_adr    0x00319428
#define sco1_rx_decim        (*(volatile unsigned int *)sco1_rx_decim_adr)
#define sco1_rx_cvsd_adr0    0x0031942c
#define sco1_rx_cvsd0        (*(volatile unsigned int *)sco1_rx_cvsd_adr0)
#define sco1_rx_cvsd_adr1    0x00319430
#define sco1_rx_cvsd1        (*(volatile unsigned int *)sco1_rx_cvsd_adr1)
#define sco1_tx_intrp_adr    0x00319434
#define sco1_tx_intrp        (*(volatile unsigned int *)sco1_tx_intrp_adr)
#define sco1_tx_cvsd_adr0    0x00319438
#define sco1_tx_cvsd0        (*(volatile unsigned int *)sco1_tx_cvsd_adr0)
#define sco1_tx_cvsd_adr1    0x0031943c
#define sco1_tx_cvsd1        (*(volatile unsigned int *)sco1_tx_cvsd_adr1)
#define sco2_rx_decim_adr    0x00319440
#define sco2_rx_decim        (*(volatile unsigned int *)sco2_rx_decim_adr)
#define sco2_rx_cvsd_adr0    0x00319444
#define sco2_rx_cvsd0        (*(volatile unsigned int *)sco2_rx_cvsd_adr0)
#define sco2_rx_cvsd_adr1    0x00319448
#define sco2_rx_cvsd1        (*(volatile unsigned int *)sco2_rx_cvsd_adr1)
#define sco2_tx_intrp_adr    0x0031944c
#define sco2_tx_intrp        (*(volatile unsigned int *)sco2_tx_intrp_adr)
#define sco2_tx_cvsd_adr0    0x00319450
#define sco2_tx_cvsd0        (*(volatile unsigned int *)sco2_tx_cvsd_adr0)
#define sco2_tx_cvsd_adr1    0x00319454
#define sco2_tx_cvsd1        (*(volatile unsigned int *)sco2_tx_cvsd_adr1)
#define apu_ram_base_adr     0x00319458
#define apu_ram_base         (*(volatile unsigned int *)apu_ram_base_adr)
#define apu_fifo_base_adr    0x00319578
#define apu_fifo_base        (*(volatile unsigned int *)apu_fifo_base_adr)
#define apu_fifo_rst_adr     0x003199b0
#define apu_fifo_rst         (*(volatile unsigned int *)apu_fifo_rst_adr)
#define apu_wlin_stat_adr    0x003199b4
#define apu_wlin_stat        (*(volatile unsigned int *)apu_wlin_stat_adr)
#define apu_wlin_trig_adr    0x003199b8
#define apu_wlin_trig        (*(volatile unsigned int *)apu_wlin_trig_adr)
#define plc_base             0x00319a00
#define plc_enable_adr       0x00319a00
#define plc_enable           (*(volatile unsigned int *)plc_enable_adr)
#define plc_mode_adr         0x00319a04
#define plc_mode             (*(volatile unsigned int *)plc_mode_adr)
#define plc_config_adr       0x00319a08
#define plc_config           (*(volatile unsigned int *)plc_config_adr)
#define plc_data_adr         0x00319a0c
#define plc_data             (*(volatile unsigned int *)plc_data_adr)
#define plc_status_adr       0x00319a10
#define plc_status           (*(volatile unsigned int *)plc_status_adr)
#define plc_ch_a_base_addr_adr 0x00319a00
#define plc_ch_a_base_addr   (*(volatile unsigned int *)plc_ch_a_base_addr_adr)
#define plc_ch_b_base_addr_adr 0x00319a40
#define plc_ch_b_base_addr   (*(volatile unsigned int *)plc_ch_b_base_addr_adr)
#define rbg_base             0x00319b04
#define rbg_control_adr      0x00319b04
#define rbg_control          (*(volatile unsigned int *)rbg_control_adr)
#define rbg_status_adr       0x00319b08
#define rbg_status           (*(volatile unsigned int *)rbg_status_adr)
#define rbg_random_num_adr   0x00319b0c
#define rbg_random_num       (*(volatile unsigned int *)rbg_random_num_adr)
#define aoa_snapShotMem_base 0x0031a000
#define aoa_antMapMem_base   0x0031a400
#define aoa_iqBufferMem_base 0x0031a800
#define aoa_reg0_adr         0x0031ac00
#define aoa_reg0             (*(volatile unsigned int *)aoa_reg0_adr)
#define aoa_reg1_adr         0x0031ac04
#define aoa_reg1             (*(volatile unsigned int *)aoa_reg1_adr)
#define aoa_reg2_adr         0x0031ac08
#define aoa_reg2             (*(volatile unsigned int *)aoa_reg2_adr)
#define aoa_reg3_adr         0x0031ac0c
#define aoa_reg3             (*(volatile unsigned int *)aoa_reg3_adr)
#define aoa_x2x0_status_adr  0x0031ac10
#define aoa_x2x0_status      (*(volatile unsigned int *)aoa_x2x0_status_adr)
#define btrf_idcode_adr      0x00600004
#define btrf_idcode          (*(volatile unsigned int *)btrf_idcode_adr)
#define btrf_reset_adr       0x00600008
#define btrf_reset           (*(volatile unsigned int *)btrf_reset_adr)
#define btrf_reg3_adr        0x0060000c
#define btrf_reg3            (*(volatile unsigned int *)btrf_reg3_adr)
#define btrf_reg4_adr        0x00600010
#define btrf_reg4            (*(volatile unsigned int *)btrf_reg4_adr)
#define btrf_reg5_adr        0x00600014
#define btrf_reg5            (*(volatile unsigned int *)btrf_reg5_adr)
#define btrf_reg6_adr        0x00600018
#define btrf_reg6            (*(volatile unsigned int *)btrf_reg6_adr)
#define btrf_reg7_adr        0x0060001c
#define btrf_reg7            (*(volatile unsigned int *)btrf_reg7_adr)
#define btrf_reg8_adr        0x00600020
#define btrf_reg8            (*(volatile unsigned int *)btrf_reg8_adr)
#define btrf_reg9_adr        0x00600024
#define btrf_reg9            (*(volatile unsigned int *)btrf_reg9_adr)
#define btrf_rega_adr        0x00600028
#define btrf_rega            (*(volatile unsigned int *)btrf_rega_adr)
#define btrf_regb_adr        0x0060002c
#define btrf_regb            (*(volatile unsigned int *)btrf_regb_adr)
#define btrf_regc_adr        0x00600030
#define btrf_regc            (*(volatile unsigned int *)btrf_regc_adr)
#define btrf_regd_adr        0x00600034
#define btrf_regd            (*(volatile unsigned int *)btrf_regd_adr)
#define btrf_rege_adr        0x00600038
#define btrf_rege            (*(volatile unsigned int *)btrf_rege_adr)
#define btrf_regf_adr        0x0060003c
#define btrf_regf            (*(volatile unsigned int *)btrf_regf_adr)
#define btrf_reg10_adr       0x00600040
#define btrf_reg10           (*(volatile unsigned int *)btrf_reg10_adr)
#define btrf_reg11_adr       0x00600044
#define btrf_reg11           (*(volatile unsigned int *)btrf_reg11_adr)
#define btrf_reg12_adr       0x00600048
#define btrf_reg12           (*(volatile unsigned int *)btrf_reg12_adr)
#define btrf_reg13_adr       0x0060004c
#define btrf_reg13           (*(volatile unsigned int *)btrf_reg13_adr)
#define btrf_reg14_adr       0x00600050
#define btrf_reg14           (*(volatile unsigned int *)btrf_reg14_adr)
#define btrf_reg15_adr       0x00600054
#define btrf_reg15           (*(volatile unsigned int *)btrf_reg15_adr)
#define btrf_reg16_adr       0x00600058
#define btrf_reg16           (*(volatile unsigned int *)btrf_reg16_adr)
#define btrf_reg17_adr       0x0060005c
#define btrf_reg17           (*(volatile unsigned int *)btrf_reg17_adr)
#define btrf_reg18_adr       0x00600060
#define btrf_reg18           (*(volatile unsigned int *)btrf_reg18_adr)
#define btrf_reg19_adr       0x00600064
#define btrf_reg19           (*(volatile unsigned int *)btrf_reg19_adr)
#define btrf_reg1a_adr       0x00600068
#define btrf_reg1a           (*(volatile unsigned int *)btrf_reg1a_adr)
#define btrf_reg1b_adr       0x0060006c
#define btrf_reg1b           (*(volatile unsigned int *)btrf_reg1b_adr)
#define btrf_reg1c_adr       0x00600070
#define btrf_reg1c           (*(volatile unsigned int *)btrf_reg1c_adr)
#define btrf_reg1d_adr       0x00600074
#define btrf_reg1d           (*(volatile unsigned int *)btrf_reg1d_adr)
#define btrf_reg1e_adr       0x00600078
#define btrf_reg1e           (*(volatile unsigned int *)btrf_reg1e_adr)
#define btrf_reg1f_adr       0x0060007c
#define btrf_reg1f           (*(volatile unsigned int *)btrf_reg1f_adr)
#define btrf_reg20_adr       0x00600080
#define btrf_reg20           (*(volatile unsigned int *)btrf_reg20_adr)
#define btrf_reg21_adr       0x00600084
#define btrf_reg21           (*(volatile unsigned int *)btrf_reg21_adr)
#define btrf_reg22_adr       0x00600088
#define btrf_reg22           (*(volatile unsigned int *)btrf_reg22_adr)
#define btrf_reg23_adr       0x0060008c
#define btrf_reg23           (*(volatile unsigned int *)btrf_reg23_adr)
#define btrf_reg24_adr       0x00600090
#define btrf_reg24           (*(volatile unsigned int *)btrf_reg24_adr)
#define btrf_reg25_adr       0x00600094
#define btrf_reg25           (*(volatile unsigned int *)btrf_reg25_adr)
#define btrf_reg26_adr       0x00600098
#define btrf_reg26           (*(volatile unsigned int *)btrf_reg26_adr)
#define btrf_reg27_adr       0x0060009c
#define btrf_reg27           (*(volatile unsigned int *)btrf_reg27_adr)
#define btrf_reg28_adr       0x006000a0
#define btrf_reg28           (*(volatile unsigned int *)btrf_reg28_adr)
#define btrf_reg29_adr       0x006000a4
#define btrf_reg29           (*(volatile unsigned int *)btrf_reg29_adr)
#define btrf_reg2a_adr       0x006000a8
#define btrf_reg2a           (*(volatile unsigned int *)btrf_reg2a_adr)
#define btrf_reg2b_adr       0x006000ac
#define btrf_reg2b           (*(volatile unsigned int *)btrf_reg2b_adr)
#define btrf_reg2c_adr       0x006000b0
#define btrf_reg2c           (*(volatile unsigned int *)btrf_reg2c_adr)
#define btrf_reg2d_adr       0x006000b4
#define btrf_reg2d           (*(volatile unsigned int *)btrf_reg2d_adr)
#define btrf_reg2e_adr       0x006000b8
#define btrf_reg2e           (*(volatile unsigned int *)btrf_reg2e_adr)
#define btrf_reg2f_adr       0x006000bc
#define btrf_reg2f           (*(volatile unsigned int *)btrf_reg2f_adr)
#define btrf_reg30_adr       0x006000c0
#define btrf_reg30           (*(volatile unsigned int *)btrf_reg30_adr)
#define btrf_reg31_adr       0x006000c4
#define btrf_reg31           (*(volatile unsigned int *)btrf_reg31_adr)
#define btrf_reg32_adr       0x006000c8
#define btrf_reg32           (*(volatile unsigned int *)btrf_reg32_adr)
#define btrf_reg33_adr       0x006000cc
#define btrf_reg33           (*(volatile unsigned int *)btrf_reg33_adr)
#define btrf_reg34_adr       0x006000d0
#define btrf_reg34           (*(volatile unsigned int *)btrf_reg34_adr)
#define btrf_reg35_adr       0x006000d4
#define btrf_reg35           (*(volatile unsigned int *)btrf_reg35_adr)
#define btrf_reg36_adr       0x006000d8
#define btrf_reg36           (*(volatile unsigned int *)btrf_reg36_adr)
#define btrf_reg37_adr       0x006000dc
#define btrf_reg37           (*(volatile unsigned int *)btrf_reg37_adr)
#define btrf_reg38_adr       0x006000e0
#define btrf_reg38           (*(volatile unsigned int *)btrf_reg38_adr)
#define btrf_reg39_adr       0x006000e4
#define btrf_reg39           (*(volatile unsigned int *)btrf_reg39_adr)
#define btrf_reg3a_adr       0x006000e8
#define btrf_reg3a           (*(volatile unsigned int *)btrf_reg3a_adr)
#define btrf_reg3b_adr       0x006000ec
#define btrf_reg3b           (*(volatile unsigned int *)btrf_reg3b_adr)
#define btrf_reg3c_adr       0x006000f0
#define btrf_reg3c           (*(volatile unsigned int *)btrf_reg3c_adr)
#define btrf_reg3d_adr       0x006000f4
#define btrf_reg3d           (*(volatile unsigned int *)btrf_reg3d_adr)
#define btrf_reg3e_adr       0x006000f8
#define btrf_reg3e           (*(volatile unsigned int *)btrf_reg3e_adr)
#define btrf_reg3f_adr       0x006000fc
#define btrf_reg3f           (*(volatile unsigned int *)btrf_reg3f_adr)
#define btrf_rssi_adr        0x00600100
#define btrf_rssi            (*(volatile unsigned int *)btrf_rssi_adr)
#define btrf_pll_val_adr     0x00600104
#define btrf_pll_val         (*(volatile unsigned int *)btrf_pll_val_adr)
#define btrf_clk_ctl_adr     0x00600108
#define btrf_clk_ctl         (*(volatile unsigned int *)btrf_clk_ctl_adr)
#define btrf_pwr_cntl_adr    0x0060010c
#define btrf_pwr_cntl        (*(volatile unsigned int *)btrf_pwr_cntl_adr)
#define btrf_pu_ovr_0_adr    0x00600110
#define btrf_pu_ovr_0        (*(volatile unsigned int *)btrf_pu_ovr_0_adr)
#define btrf_pu_ovr_val_0_adr 0x00600114
#define btrf_pu_ovr_val_0    (*(volatile unsigned int *)btrf_pu_ovr_val_0_adr)
#define btrf_pu_ovr_1_adr    0x00600118
#define btrf_pu_ovr_1        (*(volatile unsigned int *)btrf_pu_ovr_1_adr)
#define btrf_pu_ovr_val_1_adr 0x0060011c
#define btrf_pu_ovr_val_1    (*(volatile unsigned int *)btrf_pu_ovr_val_1_adr)
#define btrf_rx_gain_ovr_adr 0x00600120
#define btrf_rx_gain_ovr     (*(volatile unsigned int *)btrf_rx_gain_ovr_adr)
#define btrf_rf_pll_tx_base_0_adr 0x00600124
#define btrf_rf_pll_tx_base_0 (*(volatile unsigned int *)btrf_rf_pll_tx_base_0_adr)
#define btrf_rf_pll_tx_base_1_adr 0x00600128
#define btrf_rf_pll_tx_base_1 (*(volatile unsigned int *)btrf_rf_pll_tx_base_1_adr)
#define btrf_rf_pll_tx_base_2_adr 0x0060012c
#define btrf_rf_pll_tx_base_2 (*(volatile unsigned int *)btrf_rf_pll_tx_base_2_adr)
#define btrf_rf_pll_tx_delta_0_adr 0x00600130
#define btrf_rf_pll_tx_delta_0 (*(volatile unsigned int *)btrf_rf_pll_tx_delta_0_adr)
#define btrf_rf_pll_tx_delta_1_adr 0x00600134
#define btrf_rf_pll_tx_delta_1 (*(volatile unsigned int *)btrf_rf_pll_tx_delta_1_adr)
#define btrf_rf_pll_rx_base_0_adr 0x00600138
#define btrf_rf_pll_rx_base_0 (*(volatile unsigned int *)btrf_rf_pll_rx_base_0_adr)
#define btrf_rf_pll_rx_base_1_adr 0x0060013c
#define btrf_rf_pll_rx_base_1 (*(volatile unsigned int *)btrf_rf_pll_rx_base_1_adr)
#define btrf_rf_pll_rx_base_2_adr 0x00600140
#define btrf_rf_pll_rx_base_2 (*(volatile unsigned int *)btrf_rf_pll_rx_base_2_adr)
#define btrf_rf_pll_rx_delta_0_adr 0x00600144
#define btrf_rf_pll_rx_delta_0 (*(volatile unsigned int *)btrf_rf_pll_rx_delta_0_adr)
#define btrf_rf_pll_rx_delta_1_adr 0x00600148
#define btrf_rf_pll_rx_delta_1 (*(volatile unsigned int *)btrf_rf_pll_rx_delta_1_adr)
#define btrf_vco_lut_addr_adr 0x0060014c
#define btrf_vco_lut_addr    (*(volatile unsigned int *)btrf_vco_lut_addr_adr)
#define btrf_vco_lut_data_in_0_adr 0x00600150
#define btrf_vco_lut_data_in_0 (*(volatile unsigned int *)btrf_vco_lut_data_in_0_adr)
#define btrf_vco_lut_data_in_1_adr 0x00600154
#define btrf_vco_lut_data_in_1 (*(volatile unsigned int *)btrf_vco_lut_data_in_1_adr)
#define btrf_vco_lut_data_out_0_adr 0x00600158
#define btrf_vco_lut_data_out_0 (*(volatile unsigned int *)btrf_vco_lut_data_out_0_adr)
#define btrf_vco_lut_data_out_1_adr 0x0060015c
#define btrf_vco_lut_data_out_1 (*(volatile unsigned int *)btrf_vco_lut_data_out_1_adr)
#define btrf_pll_offset_adr  0x00600160
#define btrf_pll_offset      (*(volatile unsigned int *)btrf_pll_offset_adr)
#define btrf_ifp_int_adr     0x00600164
#define btrf_ifp_int         (*(volatile unsigned int *)btrf_ifp_int_adr)
#define btrf_ifp_fract_0_adr 0x00600168
#define btrf_ifp_fract_0     (*(volatile unsigned int *)btrf_ifp_fract_0_adr)
#define btrf_ifp_fract_1_adr 0x0060016c
#define btrf_ifp_fract_1     (*(volatile unsigned int *)btrf_ifp_fract_1_adr)
#define btrf_tr_ctrl_0_adr   0x00600170
#define btrf_tr_ctrl_0       (*(volatile unsigned int *)btrf_tr_ctrl_0_adr)
#define btrf_tr_ctrl_1_adr   0x00600174
#define btrf_tr_ctrl_1       (*(volatile unsigned int *)btrf_tr_ctrl_1_adr)
#define btrf_reg5e_adr       0x00600178
#define btrf_reg5e           (*(volatile unsigned int *)btrf_reg5e_adr)
#define btrf_reg5f_adr       0x0060017c
#define btrf_reg5f           (*(volatile unsigned int *)btrf_reg5f_adr)
#define btrf_reg60_adr       0x00600180
#define btrf_reg60           (*(volatile unsigned int *)btrf_reg60_adr)
#define btrf_reg61_adr       0x00600184
#define btrf_reg61           (*(volatile unsigned int *)btrf_reg61_adr)
#define btrf_reg62_adr       0x00600188
#define btrf_reg62           (*(volatile unsigned int *)btrf_reg62_adr)
#define btrf_reg63_adr       0x0060018c
#define btrf_reg63           (*(volatile unsigned int *)btrf_reg63_adr)
#define btrf_reg64_adr       0x00600190
#define btrf_reg64           (*(volatile unsigned int *)btrf_reg64_adr)
#define btrf_vcocal_base_adr 0x00600194
#define btrf_vcocal_base     (*(volatile unsigned int *)btrf_vcocal_base_adr)
#define btrf_reg66_adr       0x00600198
#define btrf_reg66           (*(volatile unsigned int *)btrf_reg66_adr)
#define btrf_reg67_adr       0x0060019c
#define btrf_reg67           (*(volatile unsigned int *)btrf_reg67_adr)
#define btrf_reg68_adr       0x006001a0
#define btrf_reg68           (*(volatile unsigned int *)btrf_reg68_adr)
#define btrf_reg69_adr       0x006001a4
#define btrf_reg69           (*(volatile unsigned int *)btrf_reg69_adr)
#define btrf_reg6a_adr       0x006001a8
#define btrf_reg6a           (*(volatile unsigned int *)btrf_reg6a_adr)
#define btrf_reg6b_adr       0x006001ac
#define btrf_reg6b           (*(volatile unsigned int *)btrf_reg6b_adr)
#define btrf_reg6c_adr       0x006001b0
#define btrf_reg6c           (*(volatile unsigned int *)btrf_reg6c_adr)
#define btrf_reg6d_adr       0x006001b4
#define btrf_reg6d           (*(volatile unsigned int *)btrf_reg6d_adr)
#define btrf_reg6e_adr       0x006001b8
#define btrf_reg6e           (*(volatile unsigned int *)btrf_reg6e_adr)
#define btrf_reg6f_adr       0x006001bc
#define btrf_reg6f           (*(volatile unsigned int *)btrf_reg6f_adr)
#define btrf_reg70_adr       0x006001c0
#define btrf_reg70           (*(volatile unsigned int *)btrf_reg70_adr)
#define btrf_reg71_adr       0x006001c4
#define btrf_reg71           (*(volatile unsigned int *)btrf_reg71_adr)
#define btrf_reg72_adr       0x006001c8
#define btrf_reg72           (*(volatile unsigned int *)btrf_reg72_adr)
#define btrf_reg73_adr       0x006001cc
#define btrf_reg73           (*(volatile unsigned int *)btrf_reg73_adr)
#define btrf_reg74_adr       0x006001d0
#define btrf_reg74           (*(volatile unsigned int *)btrf_reg74_adr)
#define btrf_reg75_adr       0x006001d4
#define btrf_reg75           (*(volatile unsigned int *)btrf_reg75_adr)
#define btrf_reg76_adr       0x006001d8
#define btrf_reg76           (*(volatile unsigned int *)btrf_reg76_adr)
#define btrf_reg77_adr       0x006001dc
#define btrf_reg77           (*(volatile unsigned int *)btrf_reg77_adr)
#define btrf_reg78_adr       0x006001e0
#define btrf_reg78           (*(volatile unsigned int *)btrf_reg78_adr)
#define btrf_reg79_adr       0x006001e4
#define btrf_reg79           (*(volatile unsigned int *)btrf_reg79_adr)
#define btrf_reg7a_adr       0x006001e8
#define btrf_reg7a           (*(volatile unsigned int *)btrf_reg7a_adr)
#define btrf_reg7b_adr       0x006001ec
#define btrf_reg7b           (*(volatile unsigned int *)btrf_reg7b_adr)
#define btrf_reg7c_adr       0x006001f0
#define btrf_reg7c           (*(volatile unsigned int *)btrf_reg7c_adr)
#define btrf_reg7d_adr       0x006001f4
#define btrf_reg7d           (*(volatile unsigned int *)btrf_reg7d_adr)
#define btrf_reg7e_adr       0x006001f8
#define btrf_reg7e           (*(volatile unsigned int *)btrf_reg7e_adr)
#define btrf_reg83_adr       0x0060020c
#define btrf_reg83           (*(volatile unsigned int *)btrf_reg83_adr)
#define btrf_reg84_adr       0x00600210
#define btrf_reg84           (*(volatile unsigned int *)btrf_reg84_adr)
#define btrf_reg85_adr       0x00600214
#define btrf_reg85           (*(volatile unsigned int *)btrf_reg85_adr)
#define btrf_reg86_adr       0x00600218
#define btrf_reg86           (*(volatile unsigned int *)btrf_reg86_adr)
#define btrf_reg87_adr       0x0060021c
#define btrf_reg87           (*(volatile unsigned int *)btrf_reg87_adr)
#define btrf_reg88_adr       0x00600220
#define btrf_reg88           (*(volatile unsigned int *)btrf_reg88_adr)
#define btrf_reg89_adr       0x00600224
#define btrf_reg89           (*(volatile unsigned int *)btrf_reg89_adr)
#define btrf_reg8a_adr       0x00600228
#define btrf_reg8a           (*(volatile unsigned int *)btrf_reg8a_adr)
#define btrf_reg8b_adr       0x0060022c
#define btrf_reg8b           (*(volatile unsigned int *)btrf_reg8b_adr)
#define btrf_reg8c_adr       0x00600230
#define btrf_reg8c           (*(volatile unsigned int *)btrf_reg8c_adr)
#define btrf_reg8d_adr       0x00600234
#define btrf_reg8d           (*(volatile unsigned int *)btrf_reg8d_adr)
#define btrf_reg8e_adr       0x00600238
#define btrf_reg8e           (*(volatile unsigned int *)btrf_reg8e_adr)
#define btrf_reg8f_adr       0x0060023c
#define btrf_reg8f           (*(volatile unsigned int *)btrf_reg8f_adr)
#define btrf_rf_sri_slave_sel_adr 0x006001fc
#define btrf_rf_sri_slave_sel (*(volatile unsigned int *)btrf_rf_sri_slave_sel_adr)
#define btrf_reg90_adr       0x00600240
#define btrf_reg90           (*(volatile unsigned int *)btrf_reg90_adr)
#define btrf_reg91_adr       0x00600244
#define btrf_reg91           (*(volatile unsigned int *)btrf_reg91_adr)
#define btrf_reg92_adr       0x00600248
#define btrf_reg92           (*(volatile unsigned int *)btrf_reg92_adr)
#define btrf_reg93_adr       0x0060024c
#define btrf_reg93           (*(volatile unsigned int *)btrf_reg93_adr)
#define btrf_reg94_adr       0x00600250
#define btrf_reg94           (*(volatile unsigned int *)btrf_reg94_adr)
#define btrf_reg95_adr       0x00600254
#define btrf_reg95           (*(volatile unsigned int *)btrf_reg95_adr)
#define btrf_reg96_adr       0x00600258
#define btrf_reg96           (*(volatile unsigned int *)btrf_reg96_adr)
#define btrf_reg97_adr       0x0060025c
#define btrf_reg97           (*(volatile unsigned int *)btrf_reg97_adr)
#define btrf_reg98_adr       0x00600260
#define btrf_reg98           (*(volatile unsigned int *)btrf_reg98_adr)
#define btrf_reg99_adr       0x00600264
#define btrf_reg99           (*(volatile unsigned int *)btrf_reg99_adr)
#define btrf_reg9a_adr       0x00600268
#define btrf_reg9a           (*(volatile unsigned int *)btrf_reg9a_adr)
#define btrf_reg9b_adr       0x0060026c
#define btrf_reg9b           (*(volatile unsigned int *)btrf_reg9b_adr)
#define btrf_reg9c_adr       0x00600270
#define btrf_reg9c           (*(volatile unsigned int *)btrf_reg9c_adr)
#define btrf_reg9d_adr       0x00600274
#define btrf_reg9d           (*(volatile unsigned int *)btrf_reg9d_adr)
#define btrf_reg9e_adr       0x00600278
#define btrf_reg9e           (*(volatile unsigned int *)btrf_reg9e_adr)
#define btrf_reg9f_adr       0x0060027c
#define btrf_reg9f           (*(volatile unsigned int *)btrf_reg9f_adr)
#define btrf_rega0_adr       0x00600280
#define btrf_rega0           (*(volatile unsigned int *)btrf_rega0_adr)
#define btrf_rega1_adr       0x00600284
#define btrf_rega1           (*(volatile unsigned int *)btrf_rega1_adr)
#define btrf_rega2_adr       0x00600288
#define btrf_rega2           (*(volatile unsigned int *)btrf_rega2_adr)
#define btrf_rega3_adr       0x0060028c
#define btrf_rega3           (*(volatile unsigned int *)btrf_rega3_adr)
#define btrf_rega4_adr       0x00600290
#define btrf_rega4           (*(volatile unsigned int *)btrf_rega4_adr)
#define btrf_rega5_adr       0x00600294
#define btrf_rega5           (*(volatile unsigned int *)btrf_rega5_adr)
#define btrf_rega6_adr       0x00600298
#define btrf_rega6           (*(volatile unsigned int *)btrf_rega6_adr)
#define btrf_rega7_adr       0x0060029c
#define btrf_rega7           (*(volatile unsigned int *)btrf_rega7_adr)
#define btrf_rega8_adr       0x006002a0
#define btrf_rega8           (*(volatile unsigned int *)btrf_rega8_adr)
#define btrf_rega9_adr       0x006002a4
#define btrf_rega9           (*(volatile unsigned int *)btrf_rega9_adr)
#define btrf_regaa_adr       0x006002a8
#define btrf_regaa           (*(volatile unsigned int *)btrf_regaa_adr)
#define btrf_regab_adr       0x006002ac
#define btrf_regab           (*(volatile unsigned int *)btrf_regab_adr)
#define btrf_regac_adr       0x006002b0
#define btrf_regac           (*(volatile unsigned int *)btrf_regac_adr)
#define btrf_regad_adr       0x006002b4
#define btrf_regad           (*(volatile unsigned int *)btrf_regad_adr)
#define btrf_regae_adr       0x006002b8
#define btrf_regae           (*(volatile unsigned int *)btrf_regae_adr)
#define btrf_regaf_adr       0x006002bc
#define btrf_regaf           (*(volatile unsigned int *)btrf_regaf_adr)
#define btrf_regb0_adr       0x006002c0
#define btrf_regb0           (*(volatile unsigned int *)btrf_regb0_adr)
#define btrf_regb1_adr       0x006002c4
#define btrf_regb1           (*(volatile unsigned int *)btrf_regb1_adr)
#define btrf_regb2_adr       0x006002c8
#define btrf_regb2           (*(volatile unsigned int *)btrf_regb2_adr)
#define btrf_regb3_adr       0x006002cc
#define btrf_regb3           (*(volatile unsigned int *)btrf_regb3_adr)
#define btrf_regb4_adr       0x006002d0
#define btrf_regb4           (*(volatile unsigned int *)btrf_regb4_adr)
#define btrf_regb5_adr       0x006002d4
#define btrf_regb5           (*(volatile unsigned int *)btrf_regb5_adr)
#define btrf_regb6_adr       0x006002d8
#define btrf_regb6           (*(volatile unsigned int *)btrf_regb6_adr)
#define btrf_regb7_adr       0x006002dc
#define btrf_regb7           (*(volatile unsigned int *)btrf_regb7_adr)
#define btrf_regb8_adr       0x006002e0
#define btrf_regb8           (*(volatile unsigned int *)btrf_regb8_adr)
#define btrf_regb9_adr       0x006002e4
#define btrf_regb9           (*(volatile unsigned int *)btrf_regb9_adr)
#define btrf_regba_adr       0x006002e8
#define btrf_regba           (*(volatile unsigned int *)btrf_regba_adr)
#define btrf_regbb_adr       0x006002ec
#define btrf_regbb           (*(volatile unsigned int *)btrf_regbb_adr)
#define btrf_regbc_adr       0x006002f0
#define btrf_regbc           (*(volatile unsigned int *)btrf_regbc_adr)
#define btrf_regbd_adr       0x006002f4
#define btrf_regbd           (*(volatile unsigned int *)btrf_regbd_adr)
#define btrf_regbe_adr       0x006002f8
#define btrf_regbe           (*(volatile unsigned int *)btrf_regbe_adr)
#define btrf_regbf_adr       0x006002fc
#define btrf_regbf           (*(volatile unsigned int *)btrf_regbf_adr)
#define btrf_regc0_adr       0x00600300
#define btrf_regc0           (*(volatile unsigned int *)btrf_regc0_adr)
#define btrf_regc1_adr       0x00600304
#define btrf_regc1           (*(volatile unsigned int *)btrf_regc1_adr)
#define btrf_regc2_adr       0x00600308
#define btrf_regc2           (*(volatile unsigned int *)btrf_regc2_adr)
#define btrf_regc3_adr       0x0060030c
#define btrf_regc3           (*(volatile unsigned int *)btrf_regc3_adr)
#define btrf_regc4_adr       0x00600310
#define btrf_regc4           (*(volatile unsigned int *)btrf_regc4_adr)
#define btrf_regc5_adr       0x00600314
#define btrf_regc5           (*(volatile unsigned int *)btrf_regc5_adr)
#define btrf_regc6_adr       0x00600318
#define btrf_regc6           (*(volatile unsigned int *)btrf_regc6_adr)
#define btrf_regc7_adr       0x0060031c
#define btrf_regc7           (*(volatile unsigned int *)btrf_regc7_adr)
#define btrf_regc8_adr       0x00600320
#define btrf_regc8           (*(volatile unsigned int *)btrf_regc8_adr)
#define btrf_regc9_adr       0x00600324
#define btrf_regc9           (*(volatile unsigned int *)btrf_regc9_adr)
#define btrf_regca_adr       0x00600328
#define btrf_regca           (*(volatile unsigned int *)btrf_regca_adr)
#define btrf_regcb_adr       0x0060032c
#define btrf_regcb           (*(volatile unsigned int *)btrf_regcb_adr)
#define btrf_regcc_adr       0x00600330
#define btrf_regcc           (*(volatile unsigned int *)btrf_regcc_adr)
#define btrf_regcd_adr       0x00600334
#define btrf_regcd           (*(volatile unsigned int *)btrf_regcd_adr)
#define btrf_regce_adr       0x00600338
#define btrf_regce           (*(volatile unsigned int *)btrf_regce_adr)
#define btrf_regcf_adr       0x0060033c
#define btrf_regcf           (*(volatile unsigned int *)btrf_regcf_adr)
#define btrf_regd0_adr       0x00600340
#define btrf_regd0           (*(volatile unsigned int *)btrf_regd0_adr)
#define btrf_regd1_adr       0x00600344
#define btrf_regd1           (*(volatile unsigned int *)btrf_regd1_adr)
#define btrf_regd2_adr       0x00600348
#define btrf_regd2           (*(volatile unsigned int *)btrf_regd2_adr)
#define btrf_regd3_adr       0x0060034c
#define btrf_regd3           (*(volatile unsigned int *)btrf_regd3_adr)
#define btrf_regd4_adr       0x00600350
#define btrf_regd4           (*(volatile unsigned int *)btrf_regd4_adr)
#define btrf_regd5_adr       0x00600354
#define btrf_regd5           (*(volatile unsigned int *)btrf_regd5_adr)
#define btrf_regd6_adr       0x00600358
#define btrf_regd6           (*(volatile unsigned int *)btrf_regd6_adr)
#define btrf_regd7_adr       0x0060035c
#define btrf_regd7           (*(volatile unsigned int *)btrf_regd7_adr)
#define btrf_misc_ovr_adr    0x00600360
#define btrf_misc_ovr        (*(volatile unsigned int *)btrf_misc_ovr_adr)
#define btrf_misc_ovr_val_adr 0x00600364
#define btrf_misc_ovr_val    (*(volatile unsigned int *)btrf_misc_ovr_val_adr)
#define btrf_regda_adr       0x00600368
#define btrf_regda           (*(volatile unsigned int *)btrf_regda_adr)
#define btrf_regdb_adr       0x0060036c
#define btrf_regdb           (*(volatile unsigned int *)btrf_regdb_adr)
#define btrf_rege9_adr       0x006003a4
#define btrf_rege9           (*(volatile unsigned int *)btrf_rege9_adr)
#define btrf_regea_adr       0x006003a8
#define btrf_regea           (*(volatile unsigned int *)btrf_regea_adr)
#define btrf_regeb_adr       0x006003ac
#define btrf_regeb           (*(volatile unsigned int *)btrf_regeb_adr)
#define btrf_regec_adr       0x006003b0
#define btrf_regec           (*(volatile unsigned int *)btrf_regec_adr)
#define btrf_reged_adr       0x006003b4
#define btrf_reged           (*(volatile unsigned int *)btrf_reged_adr)
#define btrf_regee_adr       0x006003b8
#define btrf_regee           (*(volatile unsigned int *)btrf_regee_adr)
#define btrf_regef_adr       0x006003bc
#define btrf_regef           (*(volatile unsigned int *)btrf_regef_adr)
#define btrf_regf0_adr       0x006003c0
#define btrf_regf0           (*(volatile unsigned int *)btrf_regf0_adr)
#define btrf_regf1_adr       0x006003c4
#define btrf_regf1           (*(volatile unsigned int *)btrf_regf1_adr)
#define btrf_pllRamAd_adr    0x006003c8
#define btrf_pllRamAd        (*(volatile unsigned int *)btrf_pllRamAd_adr)
#define btrf_pllRamBo_adr    0x006003cc
#define btrf_pllRamBo        (*(volatile unsigned int *)btrf_pllRamBo_adr)
#define btrf_pllRamD_adr     0x006003d0
#define btrf_pllRamD         (*(volatile unsigned int *)btrf_pllRamD_adr)
#define cktp_reg2_adr        0x006003d4
#define cktp_reg2            (*(volatile unsigned int *)cktp_reg2_adr)
#define cktp_reg1_adr        0x006003d8
#define cktp_reg1            (*(volatile unsigned int *)cktp_reg1_adr)
#define cktp_reg0_adr        0x006003dc
#define cktp_reg0            (*(volatile unsigned int *)cktp_reg0_adr)
#define btrf_pwrRamAd_adr    0x006003e0
#define btrf_pwrRamAd        (*(volatile unsigned int *)btrf_pwrRamAd_adr)
#define btrf_pwrRamBo_adr    0x006003e4
#define btrf_pwrRamBo        (*(volatile unsigned int *)btrf_pwrRamBo_adr)
#define btrf_pwrRamD_adr     0x006003e8
#define btrf_pwrRamD         (*(volatile unsigned int *)btrf_pwrRamD_adr)
#define btrf_regfb_adr       0x006003ec
#define btrf_regfb           (*(volatile unsigned int *)btrf_regfb_adr)
#define btrf_regfc_adr       0x006003f0
#define btrf_regfc           (*(volatile unsigned int *)btrf_regfc_adr)
#define btrf_regfd_adr       0x006003f4
#define btrf_regfd           (*(volatile unsigned int *)btrf_regfd_adr)
#define btrf_regfe_adr       0x006003f8
#define btrf_regfe           (*(volatile unsigned int *)btrf_regfe_adr)
#define btrf_regff_adr       0x006003fc
#define btrf_regff           (*(volatile unsigned int *)btrf_regff_adr)
#define modem_idcode_adr     0x0031fc04
#define modem_idcode         (*(volatile unsigned int *)modem_idcode_adr)
#define modem_adr_shifted_base 0x0031fc08
#define gainLUT0_adr         0x0031fc08
#define gainLUT0             (*(volatile unsigned int *)gainLUT0_adr)
#define gainLUT1_adr         0x0031fc0c
#define gainLUT1             (*(volatile unsigned int *)gainLUT1_adr)
#define gainLUT2_adr         0x0031fc10
#define gainLUT2             (*(volatile unsigned int *)gainLUT2_adr)
#define gainLUT3_adr         0x0031fc14
#define gainLUT3             (*(volatile unsigned int *)gainLUT3_adr)
#define gainLUT4_adr         0x0031fc18
#define gainLUT4             (*(volatile unsigned int *)gainLUT4_adr)
#define gainLUT5_adr         0x0031fc1c
#define gainLUT5             (*(volatile unsigned int *)gainLUT5_adr)
#define gainLUT6_adr         0x0031fc20
#define gainLUT6             (*(volatile unsigned int *)gainLUT6_adr)
#define gainLUT7_adr         0x0031fc24
#define gainLUT7             (*(volatile unsigned int *)gainLUT7_adr)
#define gainLUT8_adr         0x0031fc28
#define gainLUT8             (*(volatile unsigned int *)gainLUT8_adr)
#define agcCtrl0_adr         0x0031fc2c
#define agcCtrl0             (*(volatile unsigned int *)agcCtrl0_adr)
#define agcCtrl1_adr         0x0031fc30
#define agcCtrl1             (*(volatile unsigned int *)agcCtrl1_adr)
#define agcStatus_adr        0x0031fc34
#define agcStatus            (*(volatile unsigned int *)agcStatus_adr)
#define rssiDcStatus_adr     0x0031fc38
#define rssiDcStatus         (*(volatile unsigned int *)rssiDcStatus_adr)
#define squelchCtrl_adr      0x0031fc3c
#define squelchCtrl          (*(volatile unsigned int *)squelchCtrl_adr)
#define rxPskCtrl0_adr       0x0031fc40
#define rxPskCtrl0           (*(volatile unsigned int *)rxPskCtrl0_adr)
#define rxPskCtrl1_adr       0x0031fc44
#define rxPskCtrl1           (*(volatile unsigned int *)rxPskCtrl1_adr)
#define dcOffEstCtrl0_adr    0x0031fc48
#define dcOffEstCtrl0        (*(volatile unsigned int *)dcOffEstCtrl0_adr)
#define dcOffEstCtrl1_adr    0x0031fc4c
#define dcOffEstCtrl1        (*(volatile unsigned int *)dcOffEstCtrl1_adr)
#define dcOffEstCtrl2_adr    0x0031fc50
#define dcOffEstCtrl2        (*(volatile unsigned int *)dcOffEstCtrl2_adr)
#define dcOffEstCtrl3_adr    0x0031fc54
#define dcOffEstCtrl3        (*(volatile unsigned int *)dcOffEstCtrl3_adr)
#define dcOffEstCtrl4_adr    0x0031fc58
#define dcOffEstCtrl4        (*(volatile unsigned int *)dcOffEstCtrl4_adr)
#define dcOffEstCtrl5_adr    0x0031fc5c
#define dcOffEstCtrl5        (*(volatile unsigned int *)dcOffEstCtrl5_adr)
#define demodCtrl_adr        0x0031fc60
#define demodCtrl            (*(volatile unsigned int *)demodCtrl_adr)
#define syncCtrl_adr         0x0031fc64
#define syncCtrl             (*(volatile unsigned int *)syncCtrl_adr)
#define eqCoeffCtrl0_adr     0x0031fc68
#define eqCoeffCtrl0         (*(volatile unsigned int *)eqCoeffCtrl0_adr)
#define eqCoeffCtrl1_adr     0x0031fc6c
#define eqCoeffCtrl1         (*(volatile unsigned int *)eqCoeffCtrl1_adr)
#define loopCoeffCtrl0_adr   0x0031fc70
#define loopCoeffCtrl0       (*(volatile unsigned int *)loopCoeffCtrl0_adr)
#define loopCoeffCtrl1_adr   0x0031fc74
#define loopCoeffCtrl1       (*(volatile unsigned int *)loopCoeffCtrl1_adr)
#define pskSyncWord0_adr     0x0031fc78
#define pskSyncWord0         (*(volatile unsigned int *)pskSyncWord0_adr)
#define pskSyncWord1_adr     0x0031fc7c
#define pskSyncWord1         (*(volatile unsigned int *)pskSyncWord1_adr)
#define pskSyncWord2_adr     0x0031fc80
#define pskSyncWord2         (*(volatile unsigned int *)pskSyncWord2_adr)
#define adjFcCtrl_adr        0x0031fc84
#define adjFcCtrl            (*(volatile unsigned int *)adjFcCtrl_adr)
#define ifFreq_adr           0x0031fc88
#define ifFreq               (*(volatile unsigned int *)ifFreq_adr)
#define dcOffCompQ_adr       0x0031fc8c
#define dcOffCompQ           (*(volatile unsigned int *)dcOffCompQ_adr)
#define dcOffCompI_adr       0x0031fc90
#define dcOffCompI           (*(volatile unsigned int *)dcOffCompI_adr)
#define modCtrl0_adr         0x0031fc94
#define modCtrl0             (*(volatile unsigned int *)modCtrl0_adr)
#define modCtrl1_adr         0x0031fc98
#define modCtrl1             (*(volatile unsigned int *)modCtrl1_adr)
#define loopCoeffCtrl2_adr   0x0031fc9c
#define loopCoeffCtrl2       (*(volatile unsigned int *)loopCoeffCtrl2_adr)
#define loopCoeffCtrl3_adr   0x0031fca0
#define loopCoeffCtrl3       (*(volatile unsigned int *)loopCoeffCtrl3_adr)
#define txQPskLutCtrl_adr    0x0031fcac
#define txQPskLutCtrl        (*(volatile unsigned int *)txQPskLutCtrl_adr)
#define tx8PskLutCtrl0_adr   0x0031fcb0
#define tx8PskLutCtrl0       (*(volatile unsigned int *)tx8PskLutCtrl0_adr)
#define tx8PskLutCtrl1_adr   0x0031fcb4
#define tx8PskLutCtrl1       (*(volatile unsigned int *)tx8PskLutCtrl1_adr)
#define tssiCtrl_adr         0x0031fcb8
#define tssiCtrl             (*(volatile unsigned int *)tssiCtrl_adr)
#define tssiStatus_adr       0x0031fcbc
#define tssiStatus           (*(volatile unsigned int *)tssiStatus_adr)
#define testCtrl_adr         0x0031fcc0
#define testCtrl             (*(volatile unsigned int *)testCtrl_adr)
#define pa_vctrl_ctrl_adr    0x0031fcc4
#define pa_vctrl_ctrl        (*(volatile unsigned int *)pa_vctrl_ctrl_adr)
#define txPuCtrl_adr         0x0031fcc8
#define txPuCtrl             (*(volatile unsigned int *)txPuCtrl_adr)
#define rxPuCtrl_adr         0x0031fccc
#define rxPuCtrl             (*(volatile unsigned int *)rxPuCtrl_adr)
#define synthPuCtrl_adr      0x0031fcd0
#define synthPuCtrl          (*(volatile unsigned int *)synthPuCtrl_adr)
#define kvcoCalPuCtrl_adr    0x0031fcd4
#define kvcoCalPuCtrl        (*(volatile unsigned int *)kvcoCalPuCtrl_adr)
#define paPuCtrl_adr         0x0031fcd8
#define paPuCtrl             (*(volatile unsigned int *)paPuCtrl_adr)
#define paRampCtrl_adr       0x0031fcd8
#define paRampCtrl           (*(volatile unsigned int *)paRampCtrl_adr)
#define vcoloPuCtrl_adr      0x0031fcdc
#define vcoloPuCtrl          (*(volatile unsigned int *)vcoloPuCtrl_adr)
#define dsadcPuCtrl_adr      0x0031fce0
#define dsadcPuCtrl          (*(volatile unsigned int *)dsadcPuCtrl_adr)
#define trPuCtrl_adr         0x0031fce4
#define trPuCtrl             (*(volatile unsigned int *)trPuCtrl_adr)
#define tssiPuCtrl_adr       0x0031fce8
#define tssiPuCtrl           (*(volatile unsigned int *)tssiPuCtrl_adr)
#define clkCtrl_adr          0x0031fcf0
#define clkCtrl              (*(volatile unsigned int *)clkCtrl_adr)
#define rxPskCtrl2_adr       0x0031fcf4
#define rxPskCtrl2           (*(volatile unsigned int *)rxPskCtrl2_adr)
#define rxPskCtrl3_adr       0x0031fcf8
#define rxPskCtrl3           (*(volatile unsigned int *)rxPskCtrl3_adr)
#define rxPskCtrl4_adr       0x0031fcfc
#define rxPskCtrl4           (*(volatile unsigned int *)rxPskCtrl4_adr)
#define agcCtrl2_adr         0x0031fd00
#define agcCtrl2             (*(volatile unsigned int *)agcCtrl2_adr)
#define revision_adr         0x0031fd04
#define revision             (*(volatile unsigned int *)revision_adr)
#define dcOffCompQ1_adr      0x0031fd08
#define dcOffCompQ1          (*(volatile unsigned int *)dcOffCompQ1_adr)
#define dcOffCompI1_adr      0x0031fd0c
#define dcOffCompI1          (*(volatile unsigned int *)dcOffCompI1_adr)
#define dcOffCompQ2_adr      0x0031fd10
#define dcOffCompQ2          (*(volatile unsigned int *)dcOffCompQ2_adr)
#define dcOffCompI2_adr      0x0031fd14
#define dcOffCompI2          (*(volatile unsigned int *)dcOffCompI2_adr)
#define dcOffCompQ3_adr      0x0031fd18
#define dcOffCompQ3          (*(volatile unsigned int *)dcOffCompQ3_adr)
#define dcOffCompI3_adr      0x0031fd1c
#define dcOffCompI3          (*(volatile unsigned int *)dcOffCompI3_adr)
#define dcOffCompQ4_adr      0x0031fd20
#define dcOffCompQ4          (*(volatile unsigned int *)dcOffCompQ4_adr)
#define dcOffCompI4_adr      0x0031fd24
#define dcOffCompI4          (*(volatile unsigned int *)dcOffCompI4_adr)
#define iqComp1_adr          0x0031fd28
#define iqComp1              (*(volatile unsigned int *)iqComp1_adr)
#define iqComp2_adr          0x0031fd2c
#define iqComp2              (*(volatile unsigned int *)iqComp2_adr)
#define iqComp3_adr          0x0031fd30
#define iqComp3              (*(volatile unsigned int *)iqComp3_adr)
#define iqComp4_adr          0x0031fd34
#define iqComp4              (*(volatile unsigned int *)iqComp4_adr)
#define rxPskPhErr1_adr      0x0031fd38
#define rxPskPhErr1          (*(volatile unsigned int *)rxPskPhErr1_adr)
#define rxPskPhErr2_adr      0x0031fd3c
#define rxPskPhErr2          (*(volatile unsigned int *)rxPskPhErr2_adr)
#define rxPskPhErr3_adr      0x0031fd40
#define rxPskPhErr3          (*(volatile unsigned int *)rxPskPhErr3_adr)
#define rxPskPhErr4_adr      0x0031fd44
#define rxPskPhErr4          (*(volatile unsigned int *)rxPskPhErr4_adr)
#define rxPskPhErr5_adr      0x0031fd48
#define rxPskPhErr5          (*(volatile unsigned int *)rxPskPhErr5_adr)
#define rxPskPhErr6_adr      0x0031fd4c
#define rxPskPhErr6          (*(volatile unsigned int *)rxPskPhErr6_adr)
#define rxPskMagErr_adr      0x0031fd50
#define rxPskMagErr          (*(volatile unsigned int *)rxPskMagErr_adr)
#define slna_gainLUT0_adr    0x0031fd54
#define slna_gainLUT0        (*(volatile unsigned int *)slna_gainLUT0_adr)
#define slna_gainLUT1_adr    0x0031fd58
#define slna_gainLUT1        (*(volatile unsigned int *)slna_gainLUT1_adr)
#define slna_ctrl0_adr       0x0031fd5c
#define slna_ctrl0           (*(volatile unsigned int *)slna_ctrl0_adr)
#define slna_ctrl1_adr       0x0031fd60
#define slna_ctrl1           (*(volatile unsigned int *)slna_ctrl1_adr)
#define swp_Startkick_adr    0x0031fd64
#define swp_Startkick        (*(volatile unsigned int *)swp_Startkick_adr)
#define swp_EndInc_adr       0x0031fd68
#define swp_EndInc           (*(volatile unsigned int *)swp_EndInc_adr)
#define swp_Thresh_adr       0x0031fd6c
#define swp_Thresh           (*(volatile unsigned int *)swp_Thresh_adr)
#define swp_counter_adr      0x0031fd70
#define swp_counter          (*(volatile unsigned int *)swp_counter_adr)
#define swp_vcoCalCap_adr    0x0031fd74
#define swp_vcoCalCap        (*(volatile unsigned int *)swp_vcoCalCap_adr)
#define swp_threshRssi_adr   0x0031fd78
#define swp_threshRssi       (*(volatile unsigned int *)swp_threshRssi_adr)
#define swp_specVal0_adr     0x0031fd7c
#define swp_specVal0         (*(volatile unsigned int *)swp_specVal0_adr)
#define swp_specVal1_adr     0x0031fd80
#define swp_specVal1         (*(volatile unsigned int *)swp_specVal1_adr)
#define swp_specVal2_adr     0x0031fd84
#define swp_specVal2         (*(volatile unsigned int *)swp_specVal2_adr)
#define swp_specVal3_adr     0x0031fd88
#define swp_specVal3         (*(volatile unsigned int *)swp_specVal3_adr)
#define swp_specVal4_adr     0x0031fd8c
#define swp_specVal4         (*(volatile unsigned int *)swp_specVal4_adr)
#define swp_specVal5_adr     0x0031fd90
#define swp_specVal5         (*(volatile unsigned int *)swp_specVal5_adr)
#define swp_specVal6_adr     0x0031fd94
#define swp_specVal6         (*(volatile unsigned int *)swp_specVal6_adr)
#define swp_specVal7_adr     0x0031fd98
#define swp_specVal7         (*(volatile unsigned int *)swp_specVal7_adr)
#define swp_bitmapCnt_adr    0x0031fd9c
#define swp_bitmapCnt        (*(volatile unsigned int *)swp_bitmapCnt_adr)
#define swp_skipStart_adr    0x0031fda0
#define swp_skipStart        (*(volatile unsigned int *)swp_skipStart_adr)
#define swp_skipEnd_adr      0x0031fda4
#define swp_skipEnd          (*(volatile unsigned int *)swp_skipEnd_adr)
#define swp_Inc_adr          0x0031fda8
#define swp_Inc              (*(volatile unsigned int *)swp_Inc_adr)
#define swp_skipStart2_adr   0x0031fdac
#define swp_skipStart2       (*(volatile unsigned int *)swp_skipStart2_adr)
#define psk_rssi_adr         0x0031fdb0
#define psk_rssi             (*(volatile unsigned int *)psk_rssi_adr)
#define psk_acc_cnt_adr      0x0031fdb4
#define psk_acc_cnt          (*(volatile unsigned int *)psk_acc_cnt_adr)
#define ovrsync0_adr         0x0031fdb8
#define ovrsync0             (*(volatile unsigned int *)ovrsync0_adr)
#define ovrsync1_adr         0x0031fdbc
#define ovrsync1             (*(volatile unsigned int *)ovrsync1_adr)
#define ovrsync2_adr         0x0031fdc0
#define ovrsync2             (*(volatile unsigned int *)ovrsync2_adr)
#define ovrsync3_adr         0x0031fdc4
#define ovrsync3             (*(volatile unsigned int *)ovrsync3_adr)
#define swp_skipEnd2_adr     0x0031fdc8
#define swp_skipEnd2         (*(volatile unsigned int *)swp_skipEnd2_adr)
#define spurCancel_adr       0x0031fdcc
#define spurCancel           (*(volatile unsigned int *)spurCancel_adr)
#define rssiMinMax_adr       0x0031fdd0
#define rssiMinMax           (*(volatile unsigned int *)rssiMinMax_adr)
#define swp_pause_adr        0x0031fdd4
#define swp_pause            (*(volatile unsigned int *)swp_pause_adr)
#define swp_advcds_adr       0x0031fdd8
#define swp_advcds           (*(volatile unsigned int *)swp_advcds_adr)
#define swp_Maxcalcap_adr    0x0031fddc
#define swp_Maxcalcap        (*(volatile unsigned int *)swp_Maxcalcap_adr)
#define swp_MaxRssi_adr      0x0031fde0
#define swp_MaxRssi          (*(volatile unsigned int *)swp_MaxRssi_adr)
#define btmodem_spare0_adr   0x0031fde4
#define btmodem_spare0       (*(volatile unsigned int *)btmodem_spare0_adr)
#define fskAmp_sel_adr       0x0031fde8
#define fskAmp_sel           (*(volatile unsigned int *)fskAmp_sel_adr)
#define agcStatus2_adr       0x0031fdec
#define agcStatus2           (*(volatile unsigned int *)agcStatus2_adr)
#define qpskGain_adr         0x0031fdf0
#define qpskGain             (*(volatile unsigned int *)qpskGain_adr)
#define agcCtrl3_adr         0x0031fdf4
#define agcCtrl3             (*(volatile unsigned int *)agcCtrl3_adr)
#define agcCtrl4_adr         0x0031fdf8
#define agcCtrl4             (*(volatile unsigned int *)agcCtrl4_adr)
#define tx_ctrl_adr          0x0031fe00
#define tx_ctrl              (*(volatile unsigned int *)tx_ctrl_adr)
#define agcCtrl5_adr         0x0031fe08
#define agcCtrl5             (*(volatile unsigned int *)agcCtrl5_adr)
#define txRaFracBase0_adr    0x0031fe0c
#define txRaFracBase0        (*(volatile unsigned int *)txRaFracBase0_adr)
#define txRaFracBase1_adr    0x0031fe10
#define txRaFracBase1        (*(volatile unsigned int *)txRaFracBase1_adr)
#define txRaFracShift0_adr   0x0031fe14
#define txRaFracShift0       (*(volatile unsigned int *)txRaFracShift0_adr)
#define txCtrl0_adr          0x0031fe18
#define txCtrl0              (*(volatile unsigned int *)txCtrl0_adr)
#define txEquCtrl0_adr       0x0031fe1c
#define txEquCtrl0           (*(volatile unsigned int *)txEquCtrl0_adr)
#define txEquCtrl1_adr       0x0031fe20
#define txEquCtrl1           (*(volatile unsigned int *)txEquCtrl1_adr)
#define txEquCtrl2_adr       0x0031fe24
#define txEquCtrl2           (*(volatile unsigned int *)txEquCtrl2_adr)
#define txEquCtrl3_adr       0x0031fe28
#define txEquCtrl3           (*(volatile unsigned int *)txEquCtrl3_adr)
#define txEquCtrl4_adr       0x0031fe2c
#define txEquCtrl4           (*(volatile unsigned int *)txEquCtrl4_adr)
#define rxCordicBaseLo_adr   0x0031fe30
#define rxCordicBaseLo       (*(volatile unsigned int *)rxCordicBaseLo_adr)
#define rxCtrl0_adr          0x0031fe34
#define rxCtrl0              (*(volatile unsigned int *)rxCtrl0_adr)
#define spurCancelCtrl0_adr  0x0031fe38
#define spurCancelCtrl0      (*(volatile unsigned int *)spurCancelCtrl0_adr)
#define spurCancelCtrl1_adr  0x0031fe3c
#define spurCancelCtrl1      (*(volatile unsigned int *)spurCancelCtrl1_adr)
#define spurCancelCtrl2_adr  0x0031fe40
#define spurCancelCtrl2      (*(volatile unsigned int *)spurCancelCtrl2_adr)
#define spurCancelCtrl3_adr  0x0031fe44
#define spurCancelCtrl3      (*(volatile unsigned int *)spurCancelCtrl3_adr)
#define spurCancelCtrl4_adr  0x0031fe48
#define spurCancelCtrl4      (*(volatile unsigned int *)spurCancelCtrl4_adr)
#define spurCancelCtrl5_adr  0x0031fe4c
#define spurCancelCtrl5      (*(volatile unsigned int *)spurCancelCtrl5_adr)
#define spurCancelCtrl6_adr  0x0031fe50
#define spurCancelCtrl6      (*(volatile unsigned int *)spurCancelCtrl6_adr)
#define wrssiStatus_adr      0x0031fe54
#define wrssiStatus          (*(volatile unsigned int *)wrssiStatus_adr)
#define hpfCtrl0_adr         0x0031fe58
#define hpfCtrl0             (*(volatile unsigned int *)hpfCtrl0_adr)
#define hpfCtrl1_adr         0x0031fe5c
#define hpfCtrl1             (*(volatile unsigned int *)hpfCtrl1_adr)
#define magEquLow0_adr       0x0031fe60
#define magEquLow0           (*(volatile unsigned int *)magEquLow0_adr)
#define magEquLow1_adr       0x0031fe64
#define magEquLow1           (*(volatile unsigned int *)magEquLow1_adr)
#define magEquLow2_adr       0x0031fe68
#define magEquLow2           (*(volatile unsigned int *)magEquLow2_adr)
#define magEquLow3_adr       0x0031fe6c
#define magEquLow3           (*(volatile unsigned int *)magEquLow3_adr)
#define magEquLow4_adr       0x0031fe70
#define magEquLow4           (*(volatile unsigned int *)magEquLow4_adr)
#define magEquLow5_adr       0x0031fe74
#define magEquLow5           (*(volatile unsigned int *)magEquLow5_adr)
#define magEquLow6_adr       0x0031fe78
#define magEquLow6           (*(volatile unsigned int *)magEquLow6_adr)
#define magEquLow7_adr       0x0031fe7c
#define magEquLow7           (*(volatile unsigned int *)magEquLow7_adr)
#define magEquHi0_adr        0x0031fe80
#define magEquHi0            (*(volatile unsigned int *)magEquHi0_adr)
#define magEquHi1_adr        0x0031fe84
#define magEquHi1            (*(volatile unsigned int *)magEquHi1_adr)
#define magEquHi2_adr        0x0031fe88
#define magEquHi2            (*(volatile unsigned int *)magEquHi2_adr)
#define magEquHi3_adr        0x0031fe8c
#define magEquHi3            (*(volatile unsigned int *)magEquHi3_adr)
#define magEquHi4_adr        0x0031fe90
#define magEquHi4            (*(volatile unsigned int *)magEquHi4_adr)
#define magEquHi5_adr        0x0031fe94
#define magEquHi5            (*(volatile unsigned int *)magEquHi5_adr)
#define magEquHi6_adr        0x0031fe98
#define magEquHi6            (*(volatile unsigned int *)magEquHi6_adr)
#define magEquHi7_adr        0x0031fe9c
#define magEquHi7            (*(volatile unsigned int *)magEquHi7_adr)
#define gdEquA1_adr          0x0031fea0
#define gdEquA1              (*(volatile unsigned int *)gdEquA1_adr)
#define gdEquA2_adr          0x0031fea4
#define gdEquA2              (*(volatile unsigned int *)gdEquA2_adr)
#define rxRaFracBase0_adr    0x0031fea8
#define rxRaFracBase0        (*(volatile unsigned int *)rxRaFracBase0_adr)
#define rxRaFracBase1_adr    0x0031feac
#define rxRaFracBase1        (*(volatile unsigned int *)rxRaFracBase1_adr)
#define rxRaFracShift0_adr   0x0031feb0
#define rxRaFracShift0       (*(volatile unsigned int *)rxRaFracShift0_adr)
#define rxRaFracShift1_adr   0x0031feb4
#define rxRaFracShift1       (*(volatile unsigned int *)rxRaFracShift1_adr)
#define BistCtrl0_adr        0x0031feb8
#define BistCtrl0            (*(volatile unsigned int *)BistCtrl0_adr)
#define BistCtrl1_adr        0x0031febc
#define BistCtrl1            (*(volatile unsigned int *)BistCtrl1_adr)
#define wrssiCtrl0_adr       0x0031fec0
#define wrssiCtrl0           (*(volatile unsigned int *)wrssiCtrl0_adr)
#define wrssiCtrl1_adr       0x0031fec4
#define wrssiCtrl1           (*(volatile unsigned int *)wrssiCtrl1_adr)
#define wrssiCtrl2_adr       0x0031fec8
#define wrssiCtrl2           (*(volatile unsigned int *)wrssiCtrl2_adr)
#define wrssiCtrl3_adr       0x0031fecc
#define wrssiCtrl3           (*(volatile unsigned int *)wrssiCtrl3_adr)
#define wrssiCtrl4_adr       0x0031fed0
#define wrssiCtrl4           (*(volatile unsigned int *)wrssiCtrl4_adr)
#define wrssiCtrl5_adr       0x0031fed4
#define wrssiCtrl5           (*(volatile unsigned int *)wrssiCtrl5_adr)
#define agcCtrl6_adr         0x0031fed8
#define agcCtrl6             (*(volatile unsigned int *)agcCtrl6_adr)
#define agcCtrl7_adr         0x0031fedc
#define agcCtrl7             (*(volatile unsigned int *)agcCtrl7_adr)
#define agcCtrl8_adr         0x0031fee0
#define agcCtrl8             (*(volatile unsigned int *)agcCtrl8_adr)
#define agcCtrl9_adr         0x0031fee4
#define agcCtrl9             (*(volatile unsigned int *)agcCtrl9_adr)
#define ma_bec0_adr          0x0031fee8
#define ma_bec0              (*(volatile unsigned int *)ma_bec0_adr)
#define ma_bec1_adr          0x0031feec
#define ma_bec1              (*(volatile unsigned int *)ma_bec1_adr)
#define ma_bec2_adr          0x0031fef0
#define ma_bec2              (*(volatile unsigned int *)ma_bec2_adr)
#define bt_agcRamAd_adr      0x0031fef4
#define bt_agcRamAd          (*(volatile unsigned int *)bt_agcRamAd_adr)
#define bt_agcRamBo_adr      0x0031fef8
#define bt_agcRamBo          (*(volatile unsigned int *)bt_agcRamBo_adr)
#define bt_agcRamD_adr       0x0031fefc
#define bt_agcRamD           (*(volatile unsigned int *)bt_agcRamD_adr)
#define bt_wrssiRamAd_adr    0x0031ff00
#define bt_wrssiRamAd        (*(volatile unsigned int *)bt_wrssiRamAd_adr)
#define bt_wrssiRamBo_adr    0x0031ff04
#define bt_wrssiRamBo        (*(volatile unsigned int *)bt_wrssiRamBo_adr)
#define bt_wrssiRamD_adr     0x0031ff08
#define bt_wrssiRamD         (*(volatile unsigned int *)bt_wrssiRamD_adr)
#define rxCordicBaseHi_adr   0x0031ff0c
#define rxCordicBaseHi       (*(volatile unsigned int *)rxCordicBaseHi_adr)
#define bistStatus_adr       0x0031ff14
#define bistStatus           (*(volatile unsigned int *)bistStatus_adr)
#define hpfEqSwitch_adr      0x0031ff18
#define hpfEqSwitch          (*(volatile unsigned int *)hpfEqSwitch_adr)
#define gainOvr0_adr         0x0031ff1c
#define gainOvr0             (*(volatile unsigned int *)gainOvr0_adr)
#define gainOvr1_adr         0x0031ff20
#define gainOvr1             (*(volatile unsigned int *)gainOvr1_adr)
#define tx_rf_ra_sig_adr     0x0031ff24
#define tx_rf_ra_sig         (*(volatile unsigned int *)tx_rf_ra_sig_adr)
#define rx_rf_ra_sig_adr     0x0031ff28
#define rx_rf_ra_sig         (*(volatile unsigned int *)rx_rf_ra_sig_adr)
#define txrx_ra_sig_adr      0x0031ff2c
#define txrx_ra_sig          (*(volatile unsigned int *)txrx_ra_sig_adr)
#define txrx_ra_buftrk_adr   0x0031ff30
#define txrx_ra_buftrk       (*(volatile unsigned int *)txrx_ra_buftrk_adr)
#define analog_gain_lmt_adr  0x0031ff34
#define analog_gain_lmt      (*(volatile unsigned int *)analog_gain_lmt_adr)
#define dig_gain_lmt_adr     0x0031ff38
#define dig_gain_lmt         (*(volatile unsigned int *)dig_gain_lmt_adr)
#define ma_bec3_adr          0x0031ff3c
#define ma_bec3              (*(volatile unsigned int *)ma_bec3_adr)
#define tx_digGainQ1_adr     0x0031ff40
#define tx_digGainQ1         (*(volatile unsigned int *)tx_digGainQ1_adr)
#define tx_digGainQ2_adr     0x0031ff44
#define tx_digGainQ2         (*(volatile unsigned int *)tx_digGainQ2_adr)
#define tx_digGainQ3_adr     0x0031ff48
#define tx_digGainQ3         (*(volatile unsigned int *)tx_digGainQ3_adr)
#define tx_digGainQ4_adr     0x0031ff4c
#define tx_digGainQ4         (*(volatile unsigned int *)tx_digGainQ4_adr)
#define ma_bec4_adr          0x0031ff50
#define ma_bec4              (*(volatile unsigned int *)ma_bec4_adr)
#define ma_bec5_adr          0x0031ff54
#define ma_bec5              (*(volatile unsigned int *)ma_bec5_adr)
#define txCtrl1_adr          0x0031ff58
#define txCtrl1              (*(volatile unsigned int *)txCtrl1_adr)
#define txPulseShapeCoef0_adr 0x0031ff5c
#define txPulseShapeCoef0    (*(volatile unsigned int *)txPulseShapeCoef0_adr)
#define txPulseShapeCoef1_adr 0x0031ff60
#define txPulseShapeCoef1    (*(volatile unsigned int *)txPulseShapeCoef1_adr)
#define txPulseShapeCoef2_adr 0x0031ff64
#define txPulseShapeCoef2    (*(volatile unsigned int *)txPulseShapeCoef2_adr)
#define txPulseShapeCoef3_adr 0x0031ff68
#define txPulseShapeCoef3    (*(volatile unsigned int *)txPulseShapeCoef3_adr)
#define txPulseShapeCoef4_adr 0x0031ff6c
#define txPulseShapeCoef4    (*(volatile unsigned int *)txPulseShapeCoef4_adr)
#define txPulseShapeCoef5_adr 0x0031ff70
#define txPulseShapeCoef5    (*(volatile unsigned int *)txPulseShapeCoef5_adr)
#define txPulseShapeCoef6_adr 0x0031ff74
#define txPulseShapeCoef6    (*(volatile unsigned int *)txPulseShapeCoef6_adr)
#define txPreEmpFilt1_a1_adr 0x0031ff78
#define txPreEmpFilt1_a1     (*(volatile unsigned int *)txPreEmpFilt1_a1_adr)
#define txPreEmpFilt1_a2_adr 0x0031ff7c
#define txPreEmpFilt1_a2     (*(volatile unsigned int *)txPreEmpFilt1_a2_adr)
#define txPreEmpFilt1_b1_adr 0x0031ff80
#define txPreEmpFilt1_b1     (*(volatile unsigned int *)txPreEmpFilt1_b1_adr)
#define txPreEmpFilt1_b2_adr 0x0031ff84
#define txPreEmpFilt1_b2     (*(volatile unsigned int *)txPreEmpFilt1_b2_adr)
#define txPreEmpFilt2_a1_adr 0x0031ff88
#define txPreEmpFilt2_a1     (*(volatile unsigned int *)txPreEmpFilt2_a1_adr)
#define txPreEmpFilt2_a2_adr 0x0031ff8c
#define txPreEmpFilt2_a2     (*(volatile unsigned int *)txPreEmpFilt2_a2_adr)
#define txPreEmpFilt2_b1_adr 0x0031ff90
#define txPreEmpFilt2_b1     (*(volatile unsigned int *)txPreEmpFilt2_b1_adr)
#define txPreEmpFilt2_b2_adr 0x0031ff94
#define txPreEmpFilt2_b2     (*(volatile unsigned int *)txPreEmpFilt2_b2_adr)
#define rxStatus0_adr        0x0031ff98
#define rxStatus0            (*(volatile unsigned int *)rxStatus0_adr)
#define rxRaFrac_adr         0x0031ff9c
#define rxRaFrac             (*(volatile unsigned int *)rxRaFrac_adr)
#define rxInitAngle_adr      0x0031ffa0
#define rxInitAngle          (*(volatile unsigned int *)rxInitAngle_adr)
#define wrssi_cal_adr        0x0031ffe0
#define wrssi_cal            (*(volatile unsigned int *)wrssi_cal_adr)
#define rxBlkRssiTarget_adr  0x0031ffe4
#define rxBlkRssiTarget      (*(volatile unsigned int *)rxBlkRssiTarget_adr)
#define modemSpare0_adr      0x0031ffe8
#define modemSpare0          (*(volatile unsigned int *)modemSpare0_adr)
#define modemSpare1_adr      0x0031ffec
#define modemSpare1          (*(volatile unsigned int *)modemSpare1_adr)
#define vcoCalCapSav_adr     0x0031fff0
#define vcoCalCapSav         (*(volatile unsigned int *)vcoCalCapSav_adr)
#define paDrCtrl_adr         0x0031fff4
#define paDrCtrl             (*(volatile unsigned int *)paDrCtrl_adr)
#define pwrRampCtrl_adr      0x0031fff8
#define pwrRampCtrl          (*(volatile unsigned int *)pwrRampCtrl_adr)
#define sri_slave_sel_adr    0x0031fdfc
#define sri_slave_sel        (*(volatile unsigned int *)sri_slave_sel_adr)
#define r_sri_slave_sel_adr  0x0031fffc
#define r_sri_slave_sel      (*(volatile unsigned int *)r_sri_slave_sel_adr)
#define sri_bk0_offset0_adr  0x0031e000
#define sri_bk0_offset0      (*(volatile unsigned int *)sri_bk0_offset0_adr)
#define sri_bk0_offset1_adr  0x0031e004
#define sri_bk0_offset1      (*(volatile unsigned int *)sri_bk0_offset1_adr)
#define sri_bk0_offset2_adr  0x0031e008
#define sri_bk0_offset2      (*(volatile unsigned int *)sri_bk0_offset2_adr)
#define sri_bk0_offset3_adr  0x0031e00c
#define sri_bk0_offset3      (*(volatile unsigned int *)sri_bk0_offset3_adr)
#define sri_bk0_offset4_adr  0x0031e010
#define sri_bk0_offset4      (*(volatile unsigned int *)sri_bk0_offset4_adr)
#define sri_bk0_offset5_adr  0x0031e014
#define sri_bk0_offset5      (*(volatile unsigned int *)sri_bk0_offset5_adr)
#define sri_bk1_offset0_adr  0x0031e020
#define sri_bk1_offset0      (*(volatile unsigned int *)sri_bk1_offset0_adr)
#define sri_bk1_offset1_adr  0x0031e024
#define sri_bk1_offset1      (*(volatile unsigned int *)sri_bk1_offset1_adr)
#define sri_bk1_offset2_adr  0x0031e028
#define sri_bk1_offset2      (*(volatile unsigned int *)sri_bk1_offset2_adr)
#define sri_bk1_offset3_adr  0x0031e02c
#define sri_bk1_offset3      (*(volatile unsigned int *)sri_bk1_offset3_adr)
#define sri_bk1_offset4_adr  0x0031e030
#define sri_bk1_offset4      (*(volatile unsigned int *)sri_bk1_offset4_adr)
#define sri_bk1_offset5_adr  0x0031e034
#define sri_bk1_offset5      (*(volatile unsigned int *)sri_bk1_offset5_adr)
#define sri_bk2_offset0_adr  0x0031e040
#define sri_bk2_offset0      (*(volatile unsigned int *)sri_bk2_offset0_adr)
#define sri_bk2_offset1_adr  0x0031e044
#define sri_bk2_offset1      (*(volatile unsigned int *)sri_bk2_offset1_adr)
#define sri_bk2_offset2_adr  0x0031e048
#define sri_bk2_offset2      (*(volatile unsigned int *)sri_bk2_offset2_adr)
#define sri_bk2_offset3_adr  0x0031e04c
#define sri_bk2_offset3      (*(volatile unsigned int *)sri_bk2_offset3_adr)
#define sri_bk2_offset4_adr  0x0031e050
#define sri_bk2_offset4      (*(volatile unsigned int *)sri_bk2_offset4_adr)
#define sri_bk2_offset5_adr  0x0031e054
#define sri_bk2_offset5      (*(volatile unsigned int *)sri_bk2_offset5_adr)
#define sri_bk3_offset0_adr  0x0031e060
#define sri_bk3_offset0      (*(volatile unsigned int *)sri_bk3_offset0_adr)
#define sri_bk3_offset1_adr  0x0031e064
#define sri_bk3_offset1      (*(volatile unsigned int *)sri_bk3_offset1_adr)
#define sri_bk3_offset2_adr  0x0031e068
#define sri_bk3_offset2      (*(volatile unsigned int *)sri_bk3_offset2_adr)
#define sri_bk3_offset3_adr  0x0031e06c
#define sri_bk3_offset3      (*(volatile unsigned int *)sri_bk3_offset3_adr)
#define sri_bk3_offset4_adr  0x0031e070
#define sri_bk3_offset4      (*(volatile unsigned int *)sri_bk3_offset4_adr)
#define sri_bk3_offset5_adr  0x0031e074
#define sri_bk3_offset5      (*(volatile unsigned int *)sri_bk3_offset5_adr)
#define sch_start_adr        0x0031e800
#define sch_start            (*(volatile unsigned int *)sch_start_adr)
#define sch_end_adr          0x0031f000
#define sch_end              (*(volatile unsigned int *)sch_end_adr)
#define spiffy_cfg_adr       0x00360600
#define spiffy_cfg           (*(volatile unsigned int *)spiffy_cfg_adr)
#define spiffy_TransmissionLength_adr 0x00360604
#define spiffy_TransmissionLength (*(volatile unsigned int *)spiffy_TransmissionLength_adr)
#define spiffy_TxFIFOLevel_adr 0x00360608
#define spiffy_TxFIFOLevel   (*(volatile unsigned int *)spiffy_TxFIFOLevel_adr)
#define spiffy_TxAELevel_adr 0x0036060c
#define spiffy_TxAELevel     (*(volatile unsigned int *)spiffy_TxAELevel_adr)
#define spiffy_TxDMALength_adr 0x00360610
#define spiffy_TxDMALength   (*(volatile unsigned int *)spiffy_TxDMALength_adr)
#define spiffy_RxLength_adr  0x00360614
#define spiffy_RxLength      (*(volatile unsigned int *)spiffy_RxLength_adr)
#define spiffy_RxFIFOLevel_adr 0x00360618
#define spiffy_RxFIFOLevel   (*(volatile unsigned int *)spiffy_RxFIFOLevel_adr)
#define spiffy_RxAFLevel_adr 0x0036061c
#define spiffy_RxAFLevel     (*(volatile unsigned int *)spiffy_RxAFLevel_adr)
#define spiffy_RxDMALength_adr 0x00360620
#define spiffy_RxDMALength   (*(volatile unsigned int *)spiffy_RxDMALength_adr)
#define spiffy_IntStatus_adr 0x00360624
#define spiffy_IntStatus     (*(volatile unsigned int *)spiffy_IntStatus_adr)
#define spiffy_IntEnable_adr 0x00360628
#define spiffy_IntEnable     (*(volatile unsigned int *)spiffy_IntEnable_adr)
#define spiffy_Status_adr    0x0036062c
#define spiffy_Status        (*(volatile unsigned int *)spiffy_Status_adr)
#define spiffy_ClockCfg_adr  0x00360630
#define spiffy_ClockCfg      (*(volatile unsigned int *)spiffy_ClockCfg_adr)
#define spiffy_RxFIFO_adr    0x00360700
#define spiffy_RxFIFO        (*(volatile unsigned int *)spiffy_RxFIFO_adr)
#define spiffy_TxFIFO_adr    0x00360704
#define spiffy_TxFIFO        (*(volatile unsigned int *)spiffy_TxFIFO_adr)
#define spiffy2_cfg_adr      0x00361000
#define spiffy2_cfg          (*(volatile unsigned int *)spiffy2_cfg_adr)
#define spiffy2_TransmissionLength_adr 0x00361004
#define spiffy2_TransmissionLength (*(volatile unsigned int *)spiffy2_TransmissionLength_adr)
#define spiffy2_TxFIFOLevel_adr 0x00361008
#define spiffy2_TxFIFOLevel  (*(volatile unsigned int *)spiffy2_TxFIFOLevel_adr)
#define spiffy2_TxAELevel_adr 0x0036100c
#define spiffy2_TxAELevel    (*(volatile unsigned int *)spiffy2_TxAELevel_adr)
#define spiffy2_TxDMALength_adr 0x00361010
#define spiffy2_TxDMALength  (*(volatile unsigned int *)spiffy2_TxDMALength_adr)
#define spiffy2_RxLength_adr 0x00361014
#define spiffy2_RxLength     (*(volatile unsigned int *)spiffy2_RxLength_adr)
#define spiffy2_RxFIFOLevel_adr 0x00361018
#define spiffy2_RxFIFOLevel  (*(volatile unsigned int *)spiffy2_RxFIFOLevel_adr)
#define spiffy2_RxAFLevel_adr 0x0036101c
#define spiffy2_RxAFLevel    (*(volatile unsigned int *)spiffy2_RxAFLevel_adr)
#define spiffy2_RxDMALength_adr 0x00361020
#define spiffy2_RxDMALength  (*(volatile unsigned int *)spiffy2_RxDMALength_adr)
#define spiffy2_IntStatus_adr 0x00361024
#define spiffy2_IntStatus    (*(volatile unsigned int *)spiffy2_IntStatus_adr)
#define spiffy2_IntEnable_adr 0x00361028
#define spiffy2_IntEnable    (*(volatile unsigned int *)spiffy2_IntEnable_adr)
#define spiffy2_Status_adr   0x0036102c
#define spiffy2_Status       (*(volatile unsigned int *)spiffy2_Status_adr)
#define spiffy2_ClockCfg_adr 0x00361030
#define spiffy2_ClockCfg     (*(volatile unsigned int *)spiffy2_ClockCfg_adr)
#define spiffy2_RxFIFO_adr   0x00361100
#define spiffy2_RxFIFO       (*(volatile unsigned int *)spiffy2_RxFIFO_adr)
#define spiffy2_TxFIFO_adr   0x00361104
#define spiffy2_TxFIFO       (*(volatile unsigned int *)spiffy2_TxFIFO_adr)
#define keyscan_ctl_adr      0x00390000
#define keyscan_ctl          (*(volatile unsigned int *)keyscan_ctl_adr)
#define debounce_adr         0x00390004
#define debounce             (*(volatile unsigned int *)debounce_adr)
#define keyfifo_cnt_adr      0x00390008
#define keyfifo_cnt          (*(volatile unsigned int *)keyfifo_cnt_adr)
#define keyfifo_adr          0x0039000c
#define keyfifo              (*(volatile unsigned int *)keyfifo_adr)
#define mia_irq_cfg_adr      0x00390010
#define mia_irq_cfg          (*(volatile unsigned int *)mia_irq_cfg_adr)
#define mia_ctl_adr          0x00390014
#define mia_ctl              (*(volatile unsigned int *)mia_ctl_adr)
#define mia_status_adr       0x00390018
#define mia_status           (*(volatile unsigned int *)mia_status_adr)
#define qctl_adr             0x00390020
#define qctl                 (*(volatile unsigned int *)qctl_adr)
#define qd_ch_en_adr         0x00390024
#define qd_ch_en             (*(volatile unsigned int *)qd_ch_en_adr)
#define qdx_cnt_adr          0x00390028
#define qdx_cnt              (*(volatile unsigned int *)qdx_cnt_adr)
#define qdy_cnt_adr          0x0039002c
#define qdy_cnt              (*(volatile unsigned int *)qdy_cnt_adr)
#define qdz_cnt_adr          0x00390030
#define qdz_cnt              (*(volatile unsigned int *)qdz_cnt_adr)
#define qflag_adr            0x00390034
#define qflag                (*(volatile unsigned int *)qflag_adr)
#define aclk_ctl_adr         0x00390038
#define aclk_ctl             (*(volatile unsigned int *)aclk_ctl_adr)
#define reserved42_adr       0x0039003c
#define reserved42           (*(volatile unsigned int *)reserved42_adr)
#define mia_lpo_sel_adr      0x00390040
#define mia_lpo_sel          (*(volatile unsigned int *)mia_lpo_sel_adr)
#define mia_adc_intf_ctl_adr 0x00390044
#define mia_adc_intf_ctl     (*(volatile unsigned int *)mia_adc_intf_ctl_adr)
#define mia_adc_dat_adr      0x00390048
#define mia_adc_dat          (*(volatile unsigned int *)mia_adc_dat_adr)
#define mia_ir_buf_ctl_adr   0x0039004c
#define mia_ir_buf_ctl       (*(volatile unsigned int *)mia_ir_buf_ctl_adr)
#define mia_ir_cmd0_adr      0x00390050
#define mia_ir_cmd0          (*(volatile unsigned int *)mia_ir_cmd0_adr)
#define mia_ir_cmd1_adr      0x00390054
#define mia_ir_cmd1          (*(volatile unsigned int *)mia_ir_cmd1_adr)
#define mia_ir_cmd2_adr      0x00390058
#define mia_ir_cmd2          (*(volatile unsigned int *)mia_ir_cmd2_adr)
#define mia_ir_cmd3_adr      0x0039005c
#define mia_ir_cmd3          (*(volatile unsigned int *)mia_ir_cmd3_adr)
#define mia_ir_cmd4_adr      0x00390060
#define mia_ir_cmd4          (*(volatile unsigned int *)mia_ir_cmd4_adr)
#define mia_ir_cmd5_adr      0x00390064
#define mia_ir_cmd5          (*(volatile unsigned int *)mia_ir_cmd5_adr)
#define mia_ir_cmd6_adr      0x00390068
#define mia_ir_cmd6          (*(volatile unsigned int *)mia_ir_cmd6_adr)
#define mia_ir_cmd7_adr      0x0039006c
#define mia_ir_cmd7          (*(volatile unsigned int *)mia_ir_cmd7_adr)
#define mia_ir_ctl_adr       0x00390070
#define mia_ir_ctl           (*(volatile unsigned int *)mia_ir_ctl_adr)
#define mia_irl_ctl_adr      0x00390074
#define mia_irl_ctl          (*(volatile unsigned int *)mia_irl_ctl_adr)
#define mia_irl_period_adr   0x00390078
#define mia_irl_period       (*(volatile unsigned int *)mia_irl_period_adr)
#define mia_irl_status_adr   0x0039007c
#define mia_irl_status       (*(volatile unsigned int *)mia_irl_status_adr)
#define main60hz_ctl_adr     0x00390080
#define main60hz_ctl         (*(volatile unsigned int *)main60hz_ctl_adr)
#define triac_on_val1_adr    0x00390084
#define triac_on_val1        (*(volatile unsigned int *)triac_on_val1_adr)
#define triac_off_val1_adr   0x00390088
#define triac_off_val1       (*(volatile unsigned int *)triac_off_val1_adr)
#define triac_on_val2_adr    0x0039008c
#define triac_on_val2        (*(volatile unsigned int *)triac_on_val2_adr)
#define triac_off_val2_adr   0x00390090
#define triac_off_val2       (*(volatile unsigned int *)triac_off_val2_adr)
#define main_debounce_val_adr 0x00390094
#define main_debounce_val    (*(volatile unsigned int *)main_debounce_val_adr)
#define touch_measure_ctl_adr 0x00390098
#define touch_measure_ctl    (*(volatile unsigned int *)touch_measure_ctl_adr)
#define touch_in_ctr_adr     0x0039009c
#define touch_in_ctr         (*(volatile unsigned int *)touch_in_ctr_adr)
#define mia_adc_ctl0_adr     0x003900a0
#define mia_adc_ctl0         (*(volatile unsigned int *)mia_adc_ctl0_adr)
#define mia_adc_ctl1_adr     0x003900a4
#define mia_adc_ctl1         (*(volatile unsigned int *)mia_adc_ctl1_adr)
#define mia_adc_ctl2_adr     0x003900a8
#define mia_adc_ctl2         (*(volatile unsigned int *)mia_adc_ctl2_adr)
#define triac_on_val3_adr    0x003900b0
#define triac_on_val3        (*(volatile unsigned int *)triac_on_val3_adr)
#define triac_off_val3_adr   0x003900b4
#define triac_off_val3       (*(volatile unsigned int *)triac_off_val3_adr)
#define triac_on_val4_adr    0x003900b8
#define triac_on_val4        (*(volatile unsigned int *)triac_on_val4_adr)
#define triac_off_val4_adr   0x003900bc
#define triac_off_val4       (*(volatile unsigned int *)triac_off_val4_adr)
#define pwm_cfg_adr          0x003902f0
#define pwm_cfg              (*(volatile unsigned int *)pwm_cfg_adr)
#define pwm0_init_val_adr    0x003902f4
#define pwm0_init_val        (*(volatile unsigned int *)pwm0_init_val_adr)
#define pwm0_togg_val_adr    0x003902f8
#define pwm0_togg_val        (*(volatile unsigned int *)pwm0_togg_val_adr)
#define pwm1_init_val_adr    0x003902fc
#define pwm1_init_val        (*(volatile unsigned int *)pwm1_init_val_adr)
#define pwm1_togg_val_adr    0x00390300
#define pwm1_togg_val        (*(volatile unsigned int *)pwm1_togg_val_adr)
#define pwm2_init_val_adr    0x00390304
#define pwm2_init_val        (*(volatile unsigned int *)pwm2_init_val_adr)
#define pwm2_togg_val_adr    0x00390308
#define pwm2_togg_val        (*(volatile unsigned int *)pwm2_togg_val_adr)
#define pwm3_init_val_adr    0x0039030c
#define pwm3_init_val        (*(volatile unsigned int *)pwm3_init_val_adr)
#define pwm3_togg_val_adr    0x00390310
#define pwm3_togg_val        (*(volatile unsigned int *)pwm3_togg_val_adr)
#define pwm0_cntr_adr        0x00390314
#define pwm0_cntr            (*(volatile unsigned int *)pwm0_cntr_adr)
#define pwm1_cntr_adr        0x00390318
#define pwm1_cntr            (*(volatile unsigned int *)pwm1_cntr_adr)
#define pwm2_cntr_adr        0x0039031c
#define pwm2_cntr            (*(volatile unsigned int *)pwm2_cntr_adr)
#define pwm3_cntr_adr        0x00390320
#define pwm3_cntr            (*(volatile unsigned int *)pwm3_cntr_adr)
#define mia_ir_status_adr    0x00390324
#define mia_ir_status        (*(volatile unsigned int *)mia_ir_status_adr)
#define gpio_i_st_port0_adr  0x00390100
#define gpio_i_st_port0      (*(volatile unsigned int *)gpio_i_st_port0_adr)
#define gpio_i_st_port1_adr  0x00390104
#define gpio_i_st_port1      (*(volatile unsigned int *)gpio_i_st_port1_adr)
#define gpio_i_st_port2_adr  0x00390108
#define gpio_i_st_port2      (*(volatile unsigned int *)gpio_i_st_port2_adr)
#define gpio_o_val_port0_adr 0x00390110
#define gpio_o_val_port0     (*(volatile unsigned int *)gpio_o_val_port0_adr)
#define gpio_o_val_port1_adr 0x00390114
#define gpio_o_val_port1     (*(volatile unsigned int *)gpio_o_val_port1_adr)
#define gpio_o_val_port2_adr 0x00390118
#define gpio_o_val_port2     (*(volatile unsigned int *)gpio_o_val_port2_adr)
#define gpio_int_st_port0_adr 0x00390120
#define gpio_int_st_port0    (*(volatile unsigned int *)gpio_int_st_port0_adr)
#define gpio_int_st_port1_adr 0x00390124
#define gpio_int_st_port1    (*(volatile unsigned int *)gpio_int_st_port1_adr)
#define gpio_int_st_port2_adr 0x00390128
#define gpio_int_st_port2    (*(volatile unsigned int *)gpio_int_st_port2_adr)
#define lhl_ctl_adr          0x00390130
#define lhl_ctl              (*(volatile unsigned int *)lhl_ctl_adr)
#define lhl_status_adr       0x00390134
#define lhl_status           (*(volatile unsigned int *)lhl_status_adr)
#define hidoff_keepstate0_adr 0x00390138
#define hidoff_keepstate0    (*(volatile unsigned int *)hidoff_keepstate0_adr)
#define hidoff_keepstate1_adr 0x0039013c
#define hidoff_keepstate1    (*(volatile unsigned int *)hidoff_keepstate1_adr)
#define hidoff_cnt0_adr      0x00390140
#define hidoff_cnt0          (*(volatile unsigned int *)hidoff_cnt0_adr)
#define hidoff_cnt1_adr      0x00390144
#define hidoff_cnt1          (*(volatile unsigned int *)hidoff_cnt1_adr)
#define lpo_ctl_adr          0x00390148
#define lpo_ctl              (*(volatile unsigned int *)lpo_ctl_adr)
#define ldo_ctl0_adr         0x0039014c
#define ldo_ctl0             (*(volatile unsigned int *)ldo_ctl0_adr)
#define ldo_ctl1_adr         0x00390150
#define ldo_ctl1             (*(volatile unsigned int *)ldo_ctl1_adr)
#define lhl_adc_ctl_adr      0x00390154
#define lhl_adc_ctl          (*(volatile unsigned int *)lhl_adc_ctl_adr)
#define ldo_ctl_en_adr       0x0039015c
#define ldo_ctl_en           (*(volatile unsigned int *)ldo_ctl_en_adr)
#define qdxyz_in_dly_adr     0x00390164
#define qdxyz_in_dly         (*(volatile unsigned int *)qdxyz_in_dly_adr)
#define ch_sel_cfg_adr       0x00390168
#define ch_sel_cfg           (*(volatile unsigned int *)ch_sel_cfg_adr)
#define optctl_adr           0x0039019c
#define optctl               (*(volatile unsigned int *)optctl_adr)
#define realtimeclk1_adr     0x003901a0
#define realtimeclk1         (*(volatile unsigned int *)realtimeclk1_adr)
#define realtimeclk2_adr     0x003901a4
#define realtimeclk2         (*(volatile unsigned int *)realtimeclk2_adr)
#define realtimeclk3_adr     0x003901a8
#define realtimeclk3         (*(volatile unsigned int *)realtimeclk3_adr)
#define iocfg_premux_adr     0x003901fc
#define iocfg_premux         (*(volatile unsigned int *)iocfg_premux_adr)
#define iocfg_p0_adr         0x00390200
#define iocfg_p0             (*(volatile unsigned int *)iocfg_p0_adr)
#define iocfg_p1_adr         0x00390204
#define iocfg_p1             (*(volatile unsigned int *)iocfg_p1_adr)
#define iocfg_p2_adr         0x00390208
#define iocfg_p2             (*(volatile unsigned int *)iocfg_p2_adr)
#define iocfg_p3_adr         0x0039020c
#define iocfg_p3             (*(volatile unsigned int *)iocfg_p3_adr)
#define iocfg_p4_adr         0x00390210
#define iocfg_p4             (*(volatile unsigned int *)iocfg_p4_adr)
#define iocfg_p5_adr         0x00390214
#define iocfg_p5             (*(volatile unsigned int *)iocfg_p5_adr)
#define iocfg_p6_adr         0x00390218
#define iocfg_p6             (*(volatile unsigned int *)iocfg_p6_adr)
#define iocfg_p7_adr         0x0039021c
#define iocfg_p7             (*(volatile unsigned int *)iocfg_p7_adr)
#define iocfg_p8_adr         0x00390220
#define iocfg_p8             (*(volatile unsigned int *)iocfg_p8_adr)
#define iocfg_p9_adr         0x00390224
#define iocfg_p9             (*(volatile unsigned int *)iocfg_p9_adr)
#define iocfg_p10_adr        0x00390228
#define iocfg_p10            (*(volatile unsigned int *)iocfg_p10_adr)
#define iocfg_p11_adr        0x0039022c
#define iocfg_p11            (*(volatile unsigned int *)iocfg_p11_adr)
#define iocfg_p12_adr        0x00390230
#define iocfg_p12            (*(volatile unsigned int *)iocfg_p12_adr)
#define iocfg_p13_adr        0x00390234
#define iocfg_p13            (*(volatile unsigned int *)iocfg_p13_adr)
#define iocfg_p14_adr        0x00390238
#define iocfg_p14            (*(volatile unsigned int *)iocfg_p14_adr)
#define iocfg_p15_adr        0x0039023c
#define iocfg_p15            (*(volatile unsigned int *)iocfg_p15_adr)
#define iocfg_p16_adr        0x00390240
#define iocfg_p16            (*(volatile unsigned int *)iocfg_p16_adr)
#define iocfg_p17_adr        0x00390244
#define iocfg_p17            (*(volatile unsigned int *)iocfg_p17_adr)
#define iocfg_p18_adr        0x00390248
#define iocfg_p18            (*(volatile unsigned int *)iocfg_p18_adr)
#define iocfg_p19_adr        0x0039024c
#define iocfg_p19            (*(volatile unsigned int *)iocfg_p19_adr)
#define iocfg_p20_adr        0x00390250
#define iocfg_p20            (*(volatile unsigned int *)iocfg_p20_adr)
#define iocfg_p21_adr        0x00390254
#define iocfg_p21            (*(volatile unsigned int *)iocfg_p21_adr)
#define iocfg_p22_adr        0x00390258
#define iocfg_p22            (*(volatile unsigned int *)iocfg_p22_adr)
#define iocfg_p23_adr        0x0039025c
#define iocfg_p23            (*(volatile unsigned int *)iocfg_p23_adr)
#define iocfg_p24_adr        0x00390260
#define iocfg_p24            (*(volatile unsigned int *)iocfg_p24_adr)
#define iocfg_p25_adr        0x00390264
#define iocfg_p25            (*(volatile unsigned int *)iocfg_p25_adr)
#define iocfg_p26_adr        0x00390268
#define iocfg_p26            (*(volatile unsigned int *)iocfg_p26_adr)
#define iocfg_p27_adr        0x0039026c
#define iocfg_p27            (*(volatile unsigned int *)iocfg_p27_adr)
#define iocfg_p28_adr        0x00390270
#define iocfg_p28            (*(volatile unsigned int *)iocfg_p28_adr)
#define iocfg_p29_adr        0x00390274
#define iocfg_p29            (*(volatile unsigned int *)iocfg_p29_adr)
#define iocfg_p30_adr        0x00390278
#define iocfg_p30            (*(volatile unsigned int *)iocfg_p30_adr)
#define iocfg_p31_adr        0x0039027c
#define iocfg_p31            (*(volatile unsigned int *)iocfg_p31_adr)
#define iocfg_p32_adr        0x00390280
#define iocfg_p32            (*(volatile unsigned int *)iocfg_p32_adr)
#define iocfg_p33_adr        0x00390284
#define iocfg_p33            (*(volatile unsigned int *)iocfg_p33_adr)
#define iocfg_p34_adr        0x00390288
#define iocfg_p34            (*(volatile unsigned int *)iocfg_p34_adr)
#define iocfg_p35_adr        0x0039028c
#define iocfg_p35            (*(volatile unsigned int *)iocfg_p35_adr)
#define iocfg_p36_adr        0x00390290
#define iocfg_p36            (*(volatile unsigned int *)iocfg_p36_adr)
#define iocfg_p37_adr        0x00390294
#define iocfg_p37            (*(volatile unsigned int *)iocfg_p37_adr)
#define iocfg_p38_adr        0x00390298
#define iocfg_p38            (*(volatile unsigned int *)iocfg_p38_adr)
#define iocfg_p39_adr        0x0039029c
#define iocfg_p39            (*(volatile unsigned int *)iocfg_p39_adr)
#define scanp_cntr_init_val_adr 0x003902b0
#define scanp_cntr_init_val  (*(volatile unsigned int *)scanp_cntr_init_val_adr)
#define opt0_togg_val_adr    0x003902b4
#define opt0_togg_val        (*(volatile unsigned int *)opt0_togg_val_adr)
#define opt1_togg_val_adr    0x003902b8
#define opt1_togg_val        (*(volatile unsigned int *)opt1_togg_val_adr)
#define opt2_togg_val_adr    0x003902bc
#define opt2_togg_val        (*(volatile unsigned int *)opt2_togg_val_adr)
#define opt3_togg_val_adr    0x003902c0
#define opt3_togg_val        (*(volatile unsigned int *)opt3_togg_val_adr)
#define optx_smp_val_adr     0x003902cc
#define optx_smp_val         (*(volatile unsigned int *)optx_smp_val_adr)
#define opty_smp_val_adr     0x003902d0
#define opty_smp_val         (*(volatile unsigned int *)opty_smp_val_adr)
#define optz_smp_val_adr     0x003902d4
#define optz_smp_val         (*(volatile unsigned int *)optz_smp_val_adr)

#include "mi_panel_datatype.h"

#define FLAG_DELAY            0xFE
#define FLAG_END_OF_TABLE     0xFF   // END OF REGISTERS MARKER

MI_PANEL_ParamConfig_t stPanelParam =
{
    "SAT101AT40I28R1",// const char *m_pPanelName;                ///<  PanelName
    0,//MS_U8 m_bPanelDither :1;                 ///<  PANEL_DITHER,keep the setting
    E_MI_PNL_LINK_MIPI_DSI,//MHAL_DISP_ApiPnlLinkType_e m_ePanelLinkType   :4;  ///<  PANEL_LINK

    0,       //MI_U8 MI_U8DualPort      :1;          ///<  DualPort on/off
    0,       //MI_U8 MI_U8SwapPort      :1;          ///<  Swap Port on/off
    0,       //MI_U8 MI_U8SwapOdd_ML    :1;          ///<  Swap Odd ML
    0,       //MI_U8 MI_U8SwapEven_ML   :1;          ///<  Swap Even ML
    0,       //MI_U8 MI_U8SwapOdd_RB    :1;          ///<  Swap Odd RB
    0,       //MI_U8 MI_U8SwapEven_RB   :1;          ///<  Swap Even RB

    1,       //MI_U8 MI_U8SwapLVDS_POL  :1;          ///<  Swap LVDS Channel Polairyt
    1,       //MI_U8 MI_U8SwapLVDS_CH   :1;          ///<  Swap LVDS channel
    0,       //MI_U8 MI_U8PDP10BIT      :1;          ///<  PDP 10bits on/off
    1,       //MI_U8 MI_U8LVDS_TI_MODE  :1;          ///<  Ti Mode On/Off


    0,       //MI_U8 MI_U8DCLKDelay;                 ///<  DCLK Delay
    0,       //MI_U8 MI_U8InvDCLK   :1;              ///<  CLK Invert
    0,       //MI_U8 MI_U8InvDE     :1;              ///<  DE Invert
    0,       //MI_U8 MI_U8InvHSync  :1;              ///<  HSync Invert
    0,       //MI_U8 MI_U8InvVSync  :1;              ///<  VSync Invert

    ///////////////////////////////////////////////
    // Output driving current setting
    ///////////////////////////////////////////////
    // driving current setting (0x00=4mA,0x01=6mA,0x02=8mA,0x03=12mA)
    1, //MS_U8 m_ucPanelDCKLCurrent;              ///<  define PANEL_DCLK_CURRENT
    1, //MS_U8 m_ucPanelDECurrent;                ///<  define PANEL_DE_CURRENT
    1, //MS_U8 m_ucPanelODDDataCurrent;           ///<  define PANEL_ODD_DATA_CURRENT
    1, //MS_U8 m_ucPanelEvenDataCurrent;          ///<  define PANEL_EVEN_DATA_CURRENT

    ///////////////////////////////////////////////
    // panel on/off timing
    ///////////////////////////////////////////////
    30, //MS_U16 m_wPanelOnTiming1;                ///<  time between panel & data while turn on power
    400, //MS_U16 m_wPanelOnTiming2;                ///<  time between data & back light while turn on power
    80, //MS_U16 m_wPanelOffTiming1;               ///<  time between back light & data while turn off power
    30, //MS_U16 m_wPanelOffTiming2;               ///<  time between data & panel while turn off power

    18,       //u16 u16HSyncWidth;               ///<  Hsync Width
    18,      //u16 u16HSyncBackPorch;           ///<  Hsync back porch

    4,      //u16 u16VSyncWidth;               ///<  Vsync width
    8,     //u16 u16VSyncBackPorch;           ///<  Vsync back porch

    0,      //u16 u16HStart;                   ///<  HDe start
    0,       //u16 u16VStart;                   ///<  VDe start
    800,     //u16 u16Width;                    ///<  Panel Width
    1280,    //u16 u16Height;                   ///<  Panel Height

    888,     //u16 u16MaxHTotal;                ///<  Max H Total
    854,     //u16 u16HTotal;                   ///<  H Total
    816,     //u16 u16MinHTotal;                ///<  Min H Total

    1530,    //u16 u16MaxVTotal;                ///<  Max V Total
    1314,    //u16 u16VTotal;                   ///<  V Total
    1288,    //u16 u16MinVTotal;                ///<  Min V Total

    81,      //u16 u16MaxDCLK;                  ///<  Max DCLK
    67,      //u16 u16DCLK;                     ///<  DCLK ( Htt * Vtt * Fps)
    63,      //u16 u16MinDCLK;                  ///<  Min DCLK

    0,   //u16 u16SpreadSpectrumStep;       ///<  Step of SSC
    0,   //u16 u16SpreadSpectrumSpan;       ///<  Span of SSC

    0xA0,     //MI_U8 MI_U8DimmingCtl;                 ///<  Dimming Value
    0xFF,     //MI_U8 MI_U8MaxPWMVal;                  ///<  Max Dimming Value
    0x50,     //MI_U8 MI_U8MinPWMVal;                  ///<  Min Dimming Value

    0,                           //MI_U8 MI_U8DeinterMode   :1;                  ///<  DeInter Mode
    E_MI_PNL_ASPECT_RATIO_WIDE,//MIPnlAspectRatio_e ePanelAspectRatio; ///<  Aspec Ratio

    0,                           //u16 u16LVDSTxSwapValue;         // LVDS Swap Value
    E_MI_PNL_TI_8BIT_MODE,     //MIPnlTiBitMode_e eTiBitMode;  // Ti Bit Mode
    E_MI_PNL_OUTPUT_8BIT_MODE, //MHAL_DISP_ApiPnlOutPutFormatBitMode_e m_ucOutputFormatBitMode;

    0,       //MI_U8 MI_U8SwapOdd_RG    :1;          ///<  Swap Odd RG
    0,       //MI_U8 MI_U8SwapEven_RG   :1;          ///<  Swap Even RG
    0,       //MI_U8 MI_U8SwapOdd_GB    :1;          ///<  Swap Odd GB
    0,       //MI_U8 MI_U8SwapEven_GB   :1;          ///<  Swap Even GB

    0,       //MI_U8 MI_U8DoubleClk     :1;                      ///<  Double CLK On/off
    0x1C848E,//u32 u32MaxSET;                              ///<  Max Lpll Set
    0x18EB59,//u32 u32MinSET;                              ///<  Min Lpll Set
    E_MI_PNL_CHG_HTOTAL,//MIPnlOutputTimingMode_e eOutTimingMode;   ///<  Define which panel output timing change mode is used to change VFreq for same panel
    0,                    //MI_U8 MI_U8NoiseDith     :1;                      ///<  Noise Dither On/Off
    (MI_PANEL_ChannelSwapType_e)2,
    (MI_PANEL_ChannelSwapType_e)0,
    (MI_PANEL_ChannelSwapType_e)1,
    (MI_PANEL_ChannelSwapType_e)3,
    (MI_PANEL_ChannelSwapType_e)4,
};

MI_U8 JD9365_CMD[] =
{
    0xE0,0X01,0x00,
    0xE1,0X01,0x93,
    0xE2,0X01,0x65,
    0xE3,0X01,0xF8,
    0x80,0X01,0x03,

    0xE0,0X01,0x01,
    0x00,0X01,0x00,
    0x01,0X01,0x32,

    0x0C,0X01,0x74,

    0x17,0X01,0x00,
    0x18,0X01,0xAF,
    0x19,0X01,0x01,
    0x1A,0X01,0x00,
    0x1B,0X01,0xAF,
    0x1C,0X01,0x01,

    0x24,0X01,0xFE,
    0x35,0X01,0x28,

    0x37,0X01,0x09,
    0x38,0X01,0x04,
    0x39,0X01,0x00,
    0x3A,0X01,0x01,
    0x3C,0X01,0x76,
    0x3D,0X01,0xFF,
    0x3E,0X01,0xFF,
    0x3F,0X01,0x7F,

    0x40,0X01,0x06,
    0x41,0X01,0xA0,
    0x42,0X01,0x81,
    0x43,0X01,0x08,
    0x44,0X01,0x0B,
    0x45,0X01,0x28,


    0x55,0X01,0x01,
    0x57,0X01,0x69,
    0x59,0X01,0x0A,
    0x5A,0X01,0x28,
    0x5B,0X01,0x14,

    0x5D,0X01,0x7C,
    0x5E,0X01,0x65,
    0x5F,0X01,0x54,
    0x60,0X01,0x48,
    0x61,0X01,0x43,
    0x62,0X01,0x34,
    0x63,0X01,0x38,
    0x64,0X01,0x21,
    0x65,0X01,0x39,
    0x66,0X01,0x37,
    0x67,0X01,0x37,
    0x68,0X01,0x54,
    0x69,0X01,0x43,
    0x6A,0X01,0x49,
    0x6B,0X01,0x3B,
    0x6C,0X01,0x36,
    0x6D,0X01,0x28,
    0x6E,0X01,0x14,
    0x6F,0X01,0x00,
    0x70,0X01,0x7C,
    0x71,0X01,0x65,
    0x72,0X01,0x54,
    0x73,0X01,0x48,
    0x74,0X01,0x43,
    0x75,0X01,0x34,
    0x76,0X01,0x38,
    0x77,0X01,0x21,
    0x78,0X01,0x39,
    0x79,0X01,0x37,
    0x7A,0X01,0x37,
    0x7B,0X01,0x54,
    0x7C,0X01,0x43,
    0x7D,0X01,0x49,
    0x7E,0X01,0x3B,
    0x7F,0X01,0x36,
    0x80,0X01,0x28,
    0x81,0X01,0x14,
    0x82,0X01,0x00,


    0xE0,0X01,0x02,
    0x00,0X01,0x1E,
    0x01,0X01,0x1E,
    0x02,0X01,0x41,
    0x03,0X01,0x41,
    0x04,0X01,0x43,
    0x05,0X01,0x43,
    0x06,0X01,0x1F,
    0x07,0X01,0x1F,
    0x08,0X01,0x1F,
    0x09,0X01,0x1F,
    0x0A,0X01,0x1E,
    0x0B,0X01,0x1E,
    0x0C,0X01,0x1F,
    0x0D,0X01,0x47,
    0x0E,0X01,0x47,
    0x0F,0X01,0x45,
    0x10,0X01,0x45,
    0x11,0X01,0x4B,
    0x12,0X01,0x4B,
    0x13,0X01,0x49,
    0x14,0X01,0x49,
    0x15,0X01,0x1F,


    0x16,0X01,0x1E,
    0x17,0X01,0x1E,
    0x18,0X01,0x40,
    0x19,0X01,0x40,
    0x1A,0X01,0x42,
    0x1B,0X01,0x42,
    0x1C,0X01,0x1F,
    0x1D,0X01,0x1F,
    0x1E,0X01,0x1F,
    0x1F,0X01,0x1F,
    0x20,0X01,0x1E,
    0x21,0X01,0x1E,
    0x22,0X01,0x1F,
    0x23,0X01,0x46,
    0x24,0X01,0x46,
    0x25,0X01,0x44,
    0x26,0X01,0x44,
    0x27,0X01,0x4A,
    0x28,0X01,0x4A,
    0x29,0X01,0x48,
    0x2A,0X01,0x48,
    0x2B,0X01,0x1F,


    0x58,0X01,0x40,
    0x5B,0X01,0x30,
    0x5C,0X01,0x03,
    0x5D,0X01,0x30,
    0x5E,0X01,0x01,
    0x5F,0X01,0x02,
    0x63,0X01,0x14,
    0x64,0X01,0x6A,
    0x67,0X01,0x73,
    0x68,0X01,0x05,
    0x69,0X01,0x14,
    0x6A,0X01,0x6A,
    0x6B,0X01,0x08,
    0x6C,0X01,0x00,
    0x6D,0X01,0x01,
    0x6E,0X01,0x01,
    0x6F,0X01,0x88,
    0x77,0X01,0xDD,
    0x79,0X01,0x0E,
    0x7A,0X01,0x03,
    0x7D,0X01,0x14,
    0x7E,0X01,0x6A,


    0xE0,0X01,0x04,
    0x02,0X01,0x23,
    0x09,0X01,0x10,
    0x0E,0X01,0x4A,
    0x36,0X01,0x49,

    0xE0,0X01,0x00,
    0x11,0,0x00,
    FLAG_DELAY,FLAG_DELAY,200,
    0x29,0,0x00,
    FLAG_DELAY,FLAG_DELAY,200,
    0x35,0x01,0x00,
    FLAG_END_OF_TABLE,FLAG_END_OF_TABLE,
};

MI_PANEL_MipiDsiConfig_t stMipiDsiConfig =
{
    //HsTrail HsPrpr HsZero ClkHsPrpr ClkHsExit ClkTrail ClkZero ClkHsPost DaHsExit ContDet
    0x05,  0x03, 0x05, 0x0A,    0x0E,    0x03,   0x0B,  0x0A,    0x05,   0x00,
    //Lpx   TaGet  TaSure  TaGo
    0x10,0x1a, 0x16,  0x32,

    //Hac, Hpw, Hbp, Hfp, Vac, Vpw,Vbp,Vfp, Bllp,Fps
    800,   18,   18, 18,  1280,4,  8,  24,  0,   60,

    E_MI_PNL_MIPI_DSI_LANE_4,     // MIPnlMipiDsiLaneMode_e enLaneNum;
    E_MI_PNL_MIPI_DSI_RGB888,     // MIPnlMipiDsiFormat_e enFormat;
    E_MI_PNL_MIPI_DSI_SYNC_PULSE, // MIPnlMipiDsiCtrlMode_e enCtrl;

    JD9365_CMD,
    sizeof(JD9365_CMD),
    1,0x01AF,0x01B9,0x80D2,7,
    0,0,0,0,0,
};


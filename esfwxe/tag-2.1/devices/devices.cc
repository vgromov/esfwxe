// known devices entries. each entry adheres to the following format:
// 8-symbol max latin device spec, string - short device decription, string - long device description
// HUB entries specify intelligent hub devices, which designed to gather and show data from appliances
// HUB_LIGHT entries are used for HUB devices which does not provide multi appliance support
// and, hence, does not require per-user registration.
// HUB type ids get prepended with HUB_TYPE_ and appliance ids - with APPL_TYPE_
//
DEVICE_ENTRIES_BEGIN
    HUB_ENTRY(BOI1, istrBOI1_DCR_SHORT, istrBOI1_DCR_LONG)
    APPLIANCE_ENTRY(ECOL1, istrECOL1_DCR_SHORT, istrECOL1_DCR_LONG)
    HUB_LIGHT_ENTRY(BOI2L, istrBOI2L_DCR_SHORT, istrBOI2L_DCR_LONG)
    HUB_LIGHT_ENTRY(BOI2T, istrBOI2T_DCR_SHORT, istrBOI2T_DCR_LONG)
    APPLIANCE_ENTRY(ET1, istrET1_DCR_SHORT, istrET1_DCR_LONG)
    APPLIANCE_ENTRY(EGEM1, istrEGEM1_DCR_SHORT, istrEGEM1_DCR_LONG)
    APPLIANCE_ENTRY(RAD1, istrRAD1_DCR_SHORT, istrRAD1_DCR_LONG)
    APPLIANCE_ENTRY(IRA1, istrIRA1_DCR_SHORT, istrIRA1_DCR_LONG)
    APPLIANCE_ENTRY(LD1V, istrLD1V_DCR_SHORT, istrLD1V_DCR_LONG)
    APPLIANCE_ENTRY(OAA, istrOAA_DCR_SHORT, istrOAA_DCR_LONG)
    APPLIANCE_ENTRY(EL3C, istrEL3C_DCR_SHORT, istrEL3C_DCR_LONG)
    APPLIANCE_ENTRY(ECLG1, istrECLG1_DCR_SHORT, istrECLG1_DCR_LONG)
  APPLIANCE_ENTRY(LUPINX, istrLUPINX_DCR_SHORT, istrLUPINX_DCR_LONG)
DEVICE_ENTRIES_END

// octava appliances description storage
OCTAVA_DEVICE_ENTRIES_BEGIN
    OCTAVA_APPLIANCE_ENTRY(P380E, istrP380E_DCR_SHORT, istrP380E_DCR_LONG)
    OCTAVA_APPLIANCE_ENTRY(P3811, istrP3811_DCR_SHORT, istrP3811_DCR_LONG)
    OCTAVA_APPLIANCE_ENTRY(P3812, istrP3812_DCR_SHORT, istrP3812_DCR_LONG)    
OCTAVA_DEVICE_ENTRIES_END    

// Tenzor appliances|hubs description storage
TENZOR_DEVICE_ENTRIES_BEGIN
    TENZOR_APPLIANCE_ENTRY(TECOL1, istrTECOL1_DCR_SHORT, istrTECOL1_DCR_LONG)
    TENZOR_HUB_ENTRY(TBOI1, istrTBOI1_DCR_SHORT, istrTBOI1_DCR_LONG)
  TENZOR_APPLIANCE_ENTRY(TIRA1, istrTIRA1_DCR_SHORT, istrTIRA1_DCR_LONG)
TENZOR_DEVICE_ENTRIES_END    

// Quarta-Rad appliances|hubs description storage
QUARTA_DEVICE_ENTRIES_BEGIN
    QUARTA_APPLIANCE_ENTRY(RADEX1, istrRADEX1_DCR_SHORT, istrRADEX1_DCR_LONG)
    QUARTA_APPLIANCE_ENTRY(MKC1100, istrMKC1100_DCR_SHORT, istrMKC1100_DCR_LONG)
    QUARTA_APPLIANCE_ENTRY(MR107, istrMR107_DCR_SHORT, istrMR107_DCR_LONG)
    QUARTA_APPLIANCE_ENTRY(AQ, istrAQ_DCR_SHORT, istrAQ_DCR_LONG)
QUARTA_DEVICE_ENTRIES_END

// macro definitions housekeeping
#undef DEVICE_ENTRIES_BEGIN
#undef APPLIANCE_ENTRY
#undef HUB_ENTRY
#undef HUB_LIGHT_ENTRY
#undef DEVICE_ENTRIES_END

#undef OCTAVA_DEVICE_ENTRIES_BEGIN
#undef OCTAVA_APPLIANCE_ENTRY
#undef OCTAVA_DEVICE_ENTRIES_END    

#undef TENZOR_DEVICE_ENTRIES_BEGIN
#undef TENZOR_APPLIANCE_ENTRY
#undef TENZOR_HUB_ENTRY
#undef TENZOR_DEVICE_ENTRIES_END    

#undef QUARTA_DEVICE_ENTRIES_BEGIN
#undef QUARTA_APPLIANCE_ENTRY
#undef QUARTA_HUB_ENTRY
#undef QUARTA_DEVICE_ENTRIES_END    

// NB! device types greater than or equal to 60000 are reserved for
// third-parties, like misc io adapters, radio tags, etc., and are used for
// device ID marker purpose only, as supplementary device may be "dumb", not even supporting
// standard communication (not need to).

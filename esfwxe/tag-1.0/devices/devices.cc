// known devices entries. each entry adheres to the following format:
// 8-symbol max latin device spec, string - short device decription, string - long device description
// HUB entries specify intelligent hub devices, which designed to gather and show data from appliances
// HUB_LIGHT entries are used for HUB devices which does not provide multi appliance support
// and, hence, does not require per-user registration.
// HUB type ids get prepended with HUB_TYPE_ and appliance ids - with APPL_TYPE_
//
DEVICE_ENTRIES_BEGIN
	HUB_ENTRY(BOI1, "БОИ-01", "Блок отображения информации БОИ-01")
	APPLIANCE_ENTRY(ECOL1, "ФГ-01", "Фотоголовка ФГ-01")
	HUB_LIGHT_ENTRY(BOI2L, "БОИ-02Ф", "Блок отображения информации БОИ-02Ф")
	HUB_LIGHT_ENTRY(BOI2T, "БОИ-02Т", "Блок отображения информации БОИ-02Т")
	APPLIANCE_ENTRY(MC1, "МКС-01", "Головка микроклиматическая МКС-01")
	APPLIANCE_ENTRY(EMI50, "ЭМИ-50", "Головка изм. электромагнитного поля ЭМИ-50")
	APPLIANCE_ENTRY(RAD1, "РАД-01", "Измеритель активности радона РАД-01")
	APPLIANCE_ENTRY(SA1, "СГ-01", "Спектрометрическая головка СГ-01")
DEVICE_ENTRIES_END

// macro definitions housekeeping
#undef DEVICE_ENTRIES_BEGIN
#undef APPLIANCE_ENTRY
#undef HUB_ENTRY
#undef HUB_LIGHT_ENTRY
#undef DEVICE_ENTRIES_END

// NB! device types greater than or equal to 60000 are reserved for
// third-parties, like misc io adapters, radio tags, etc., and are used for
// device ID marker purpose only, as supplementary device may be "dumb", not even supporting
// standard communication (not need to).

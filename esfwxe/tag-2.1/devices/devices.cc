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
	APPLIANCE_ENTRY(ET1, "ЭТ", "Измеритель метеорологических параметров ЭкоТерма")
	APPLIANCE_ENTRY(EGEM1, "ГЕО-01", "Головка изм. геомагнитного поля ГЕО-01")
	APPLIANCE_ENTRY(RAD1, "РАД-01", "Измеритель активности радона РАД-01")
	APPLIANCE_ENTRY(IRA1, "ИКИ-01", "Измеритель ИК излучения ИКИ-01")
	APPLIANCE_ENTRY(LD1V, "ЛД-01-В", "Головка лазерная дозиметрическая ЛД-01-В")
	APPLIANCE_ENTRY(OAA, "ПИГО", "Преобразователь ИГ Октава")
	APPLIANCE_ENTRY(EL3C, "eL-03-C", "eLight-03-C")
	APPLIANCE_ENTRY(ECLG1, "eCLg-01", "eCoLogger-01")
  APPLIANCE_ENTRY(LUPINX, "Lupin-X", "Lupin-X")
DEVICE_ENTRIES_END

// octava appliances description storage
OCTAVA_DEVICE_ENTRIES_BEGIN
	OCTAVA_APPLIANCE_ENTRY(P380E, "П3-80Е", "Измеритель электростатических полей П3-80Е")
	OCTAVA_APPLIANCE_ENTRY(P3811, "П3-81-1", "Измеритель магнитной индукции П3-81-1 мТл")
	OCTAVA_APPLIANCE_ENTRY(P3812, "П3-81-2", "Измеритель магнитной индукции П3-81-2 мкТл")	
OCTAVA_DEVICE_ENTRIES_END	

// Tenzor appliances|hubs description storage
TENZOR_DEVICE_ENTRIES_BEGIN
	TENZOR_APPLIANCE_ENTRY(TECOL1, "ГФ-01", "Головка фотометрическая ГФ-01")
	TENZOR_HUB_ENTRY(TBOI1, "БОИ-01-T", "Блок отображения информации БОИ-01-T")
  TENZOR_APPLIANCE_ENTRY(TIRA1, "ИКИ-01-Т", "Измеритель ИК излучения ИКИ-01-Т")
TENZOR_DEVICE_ENTRIES_END	

// Quarta-Rad appliances|hubs description storage
QUARTA_DEVICE_ENTRIES_BEGIN
	QUARTA_APPLIANCE_ENTRY(RADEX1, "RADEX1", "Radex One")
	QUARTA_APPLIANCE_ENTRY(MKC1100, "MKC1100", "MKC1100 Dosimeter")
	QUARTA_APPLIANCE_ENTRY(MR107, "MR107", "MR107 Radon Radiometer")
	QUARTA_APPLIANCE_ENTRY(AQ, "AQ", "Air Quality Monitor")
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

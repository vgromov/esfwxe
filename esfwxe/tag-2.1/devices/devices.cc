// known devices entries. each entry adheres to the following format:
// 8-symbol max latin device spec, string - short device decription, string - long device description
// HUB entries specify intelligent hub devices, which designed to gather and show data from appliances
// HUB_LIGHT entries are used for HUB devices which does not provide multi appliance support
// and, hence, does not require per-user registration.
// HUB type ids get prepended with HUB_TYPE_ and appliance ids - with APPL_TYPE_
//
DEVICE_ENTRIES_BEGIN
	HUB_ENTRY(BOI1, "���-01", "���� ����������� ���������� ���-01")
	APPLIANCE_ENTRY(ECOL1, "��-01", "����������� ��-01")
	HUB_LIGHT_ENTRY(BOI2L, "���-02�", "���� ����������� ���������� ���-02�")
	HUB_LIGHT_ENTRY(BOI2T, "���-02�", "���� ����������� ���������� ���-02�")
	APPLIANCE_ENTRY(ET1, "��", "���������� ����������������� ���������� ��������")
	APPLIANCE_ENTRY(EGEM1, "���-01", "������� ���. ������������� ���� ���-01")
	APPLIANCE_ENTRY(RAD1, "���-01", "���������� ���������� ������ ���-01")
	APPLIANCE_ENTRY(IRA1, "���-01", "���������� �� ��������� ���-01")
	APPLIANCE_ENTRY(LD1V, "��-01-�", "������� �������� ��������������� ��-01-�")
	APPLIANCE_ENTRY(OAA, "����", "��������������� �� ������")
	APPLIANCE_ENTRY(EL3C, "eL-03-C", "eLight-03-C")
	APPLIANCE_ENTRY(ECLG1, "eCLg-01", "eCoLogger-01")
  APPLIANCE_ENTRY(LUPINX, "Lupin-X", "Lupin-X")
DEVICE_ENTRIES_END

// octava appliances description storage
OCTAVA_DEVICE_ENTRIES_BEGIN
	OCTAVA_APPLIANCE_ENTRY(P380E, "�3-80�", "���������� ������������������ ����� �3-80�")
	OCTAVA_APPLIANCE_ENTRY(P3811, "�3-81-1", "���������� ��������� �������� �3-81-1 ���")
	OCTAVA_APPLIANCE_ENTRY(P3812, "�3-81-2", "���������� ��������� �������� �3-81-2 ����")	
OCTAVA_DEVICE_ENTRIES_END	

// Tenzor appliances|hubs description storage
TENZOR_DEVICE_ENTRIES_BEGIN
	TENZOR_APPLIANCE_ENTRY(TECOL1, "��-01", "������� ��������������� ��-01")
	TENZOR_HUB_ENTRY(TBOI1, "���-01-T", "���� ����������� ���������� ���-01-T")
  TENZOR_APPLIANCE_ENTRY(TIRA1, "���-01-�", "���������� �� ��������� ���-01-�")
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

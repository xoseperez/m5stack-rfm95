// Since this sketch only supports EU868 at the moment,
// and the Arduino-LMIC library by MCCI Catena is set to US915,
// these settings have to be copied over the ones in the
// lmic_project_config.h file in the library,
// inside the project_config folder.

#define CFG_eu868 1
#define CFG_sx1276_radio 1

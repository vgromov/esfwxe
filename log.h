// information log engine
//
#ifdef __cplusplus
    extern "C" {
#endif 

// max count of logger entries available
#define MAX_LOGGER_COUNT 16

// log level
typedef enum {
    lvlCrit,
    lvlErr,
    lvlOk,
    lvlWarn,
    lvlInfo,

    // special - log levels count
    lvlCount

} LogLvl;

extern ESE_CSTR c_logLvlStrings[lvlCount];

// logger callback
typedef void (*Logger)(LogLvl, ESE_CSTR);

// log registrar API
void logInit(void);
esBL logRegister(Logger logger);
esBL logUnregister(Logger logger);

// process log message
void logPush( LogLvl lvl, ESE_CSTR msg );

#ifdef __cplusplus
    }
#endif

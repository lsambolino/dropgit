
#ifndef GAL_H_
#define GAL_H_

#include <stdint.h>

typedef enum gal_command
{
    COMMAND_INVALID = 0,
    COMMAND_DISCOVERY,
    COMMAND_PROVISIONING,
    COMMAND_RELEASE,
    COMMAND_MONITOR_SENSOR,
    COMMAND_MONITOR_STATE,
    COMMAND_MONITOR_HISTORY,
    COMMAND_COMMIT,
    COMMAND_ROLLBACK
} gal_command;

typedef enum gal_return_code
{
    // GAL_UNDEFINED is very helpful for coding, maybe it could be useful to send to the client, I'm not sure.
    GAL_UNDEFINED = -2,
    // GAL_FAILURE should be used when an error is -1
    GAL_FAILURE = -1,
    // GAL_SUCCESS should be used in case of success
    GAL_SUCCESS = 0,
    // GAL_ERROR should be used in case of general error
    GAL_ERROR = 1,
    // GAL_NOT_IMPLEMENTED should be returned when the power management function is not implemented on the selected resouce_id
    GAL_NOT_IMPLEMENTED = 2,
    // GAL_RESOURCE_NOT_FOUND should be returned when the resource_id provided on the request is non-existent
    GAL_RESOURCE_NOT_FOUND = 4,
    // GAL_RESOURCE_NOT_AVAILABLE should be returned when the resource_id provided on the request is not available in the moment
    GAL_RESOURCE_NOT_AVAILABLE = 8,
    // GAL_PROTOCOL_ERROR should be returned when remote server answers with an invalid return code
    GAL_PROTOCOL_ERROR = 16
} gal_return_code;

#if !defined(__cplusplus)

typedef enum bool
{
    true = 1,
    false = 0
} bool;

#endif

typedef struct curve_state
{
    uint16_t offered_load;
    uint16_t maximum_consumption;

    double maximum_packet_service_time;
} curve_state;

typedef struct curve_state_vector
{
    uint32_t size;
    curve_state* elements;
} curve_state_vector;

typedef struct double_vector
{
    uint32_t size;
    double* elements;
} double_vector;

typedef struct power_state
{
    uint16_t id;

    uint16_t minimum_power_gain;
    uint16_t power_gain;
    uint16_t lpi_transition_power;
    uint16_t autonomic_ps_steps;

    uint64_t wakeup_triggers;

    double autonomic_ps_service_interruption;
    double maximum_packet_throughput;
    double maximum_bit_throughput;
    double wakeup_time;
    double sleeping_time;

    double_vector ps_transition_times;
    double_vector ps_transition_service_interruption_times;

    curve_state_vector autonomic_ps_curves;
} power_state;

typedef struct power_state_history
{
    uint16_t power_state;
    // TODO:
} power_state_history;

typedef struct gal_physical_resource
{
    char id[32];
    char description[32];

    uint16_t resource_class;
    uint16_t num_children;

    //char** depends_on;
    //char** used_by;
} gal_physical_resource;

typedef struct gal_logical_resource
{
    char id[32];
    char description[32];

    //char** depends_on;
} gal_logical_resource;

typedef enum entity_sensor_data_type
{
    other = 1,  // a measure other than those listed below
    unknown,    // unknown measurement, or arbitrary, relative numbers
    voltsAC,    // electric potential
    voltsDC,    // electric potential
    amperes,    // electric current
    watts,      // power
    hertz,      // frequency
    celsius,    // temperature
    percentRH,  // percent relative humidity
    rpm,        // shaft revolutions per minute
    cmm,        // cubic meters per minute (airflow)
    truthvalue  // value takes { true(1), false(2) }
} entity_sensor_data_type;

typedef enum entity_sensor_data_scale
{
    yocto = 1,  // 10^-24
    zepto,      // 10^-21
    atto,       // 10^-18
    femto,      // 10^-15
    pico,       // 10^-12
    nano,       // 10^-9
    micro,      // 10^-6
    milli,      // 10^-3
    units,      // 10^0
    kilo,       // 10^3
    mega,       // 10^6
    giga,       // 10^9
    tera,       // 10^12
    exa,        // 10^15
    peta,       // 10^18
    zetta,      // 10^21
    yotta       // 10^24
} entity_sensor_data_scale;

typedef enum entity_sensor_status
{
    ok = 1,
    unavailable,
    nonoperational
} entity_sensor_status;

typedef int32_t entity_sensor_precision;
typedef uint32_t entity_sensor_value_update_rate;
typedef int32_t entity_sensor_value;
typedef uint32_t entity_sensor_timestamp;

typedef struct gal_sensor_resource
{
    char id[32];
    char description[32];

    entity_sensor_data_type type;
    entity_sensor_data_scale scale;
    entity_sensor_precision precision;
    entity_sensor_value_update_rate refresh_rate;
} gal_sensor_resource;

typedef struct power_state_id_vector
{
    uint32_t size;
    uint16_t* elements;
} power_state_id_vector;

typedef struct optimal_config
{
    uint16_t id;
    power_state_id_vector power_states;
} optimal_config;

typedef struct gal_logical_resource_vector
{
    uint32_t size;
    gal_logical_resource* elements;
} gal_logical_resource_vector;

typedef struct gal_physical_resource_vector
{
    uint32_t size;
    gal_physical_resource* elements;
} gal_physical_resource_vector;

typedef struct gal_sensor_resource_vector
{
    uint32_t size;
    gal_sensor_resource* elements;
} gal_sensor_resource_vector;

typedef struct power_state_vector
{
    uint32_t size;
    power_state* elements;
} power_state_vector;

typedef struct optimal_config_vector
{
    uint32_t size;
    optimal_config* elements;
} optimal_config_vector;

typedef struct power_state_history_vector
{
    uint32_t size;
    power_state_history* elements;
} power_state_history_vector;

gal_return_code discovery(const char* resource_id,
                  bool committed,
                  gal_logical_resource_vector* logical_resources,
                  gal_physical_resource_vector* physical_resources,
                  gal_sensor_resource_vector* sensor_resources,
                  power_state_vector* power_states,
                  optimal_config_vector* edl);

gal_return_code provisioning(const char* resource_id, int power_state_id);
gal_return_code release(const char* resource_id);
gal_return_code monitor_state(const char* resource_id, bool committed, int* power_state_id);
gal_return_code monitor_history(const char* resource_id, struct power_state_history_vector* history);
gal_return_code monitor_sensor(const char* resource_id, entity_sensor_status* oper_status, entity_sensor_value* sensor_value, entity_sensor_timestamp* value_timestamp);
gal_return_code commit(const char* resource_id);
gal_return_code rollback(const char* resource_id);

#define GAL_FREE_VECTOR_ELEMENTS(vector_pointer) free((vector_pointer)->elements);

#define GAL_FOR_EACH_ELEMENT(vector_pointer, i) \
    for (i=0; i<(vector_pointer)->size; ++i)

#define GAL_ELEMENT(vector_pointer, i) (vector_pointer)->elements[i]

#define GAL_VECTOR_SIZE(vector_pointer) (vector_pointer)->size

#endif

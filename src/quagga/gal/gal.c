
#include "gal.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#define LPORT 12345
#define LADDRESS INADDR_LOOPBACK

#define RPORT 12346
#define RADDRESS INADDR_LOOPBACK

struct gal_packet
{
    gal_command command;
    char id[32];
    size_t size;
    char data[];
};

static char* init_buffer(char buffer[], gal_command command, const char* resource_id);

static gal_return_code init_gal_request(int* sock, struct sockaddr_in* addr);
static gal_return_code read_gal_response_code(int sock);
static gal_return_code do_send_gal_request(int sock, struct sockaddr_in* addr, const char* buffer, size_t size);
static gal_return_code send_gal_simple_request(gal_command command, const char* resource_id);
static gal_return_code send_gal_request_with_param(gal_command command, const char* resource_id, const char* param, size_t param_size);
static gal_return_code send_gal_request_with_reply(gal_command command, const char* resource_id, char* param, size_t param_size);
static gal_return_code send_gal_request_with_param_with_reply(gal_command command, const char* resource_id, const char* in_param, size_t in_param_size, char* out_param, size_t out_param_size);

#define GAL_COPY_MEMORY(dst, src, size) \
    memcpy((dst), (src), size); \
    src += size;

#define GAL_ALLOC_VECTOR_ELEMENTS(vector_pointer, vector_element_type) \
    size_t s = (vector_pointer)->size * sizeof(vector_element_type); \
    (vector_pointer)->elements = malloc(s); \
    assert((vector_pointer)->elements != NULL);

#define GAL_READ_VECTOR_SIZE(vector_pointer, vector_element_type, buffer_pointer) \
    (vector_pointer)->size = ((vector_element_type ## _vector *)buffer_pointer)->size; \
    buffer_pointer += sizeof((vector_pointer)->size);

#define GAL_READ_VECTOR_BLOCK_ELEMENTS(vector_pointer, vector_element_type, buffer_pointer) \
    if ((vector_pointer)->size > 0) { \
        GAL_ALLOC_VECTOR_ELEMENTS(vector_pointer, vector_element_type) \
        GAL_COPY_MEMORY((vector_pointer)->elements, buffer_pointer, s) }

#define GAL_READ_BLOCK_VECTOR(vector_pointer, vector_element_type, buffer_pointer) \
    GAL_READ_VECTOR_SIZE(vector_pointer, vector_element_type, buffer_pointer) \
    GAL_READ_VECTOR_BLOCK_ELEMENTS(vector_pointer, vector_element_type, buffer_pointer)

#define GAL_FREE_VECTOR_ELEMENTS(vector_pointer) \
    free((vector_pointer)->elements);

gal_return_code discovery(const char* resource_id,
                  bool committed,
                  gal_logical_resource_vector* logical_resources,
                  gal_physical_resource_vector* physical_resources,
                  gal_sensor_resource_vector* sensor_resources,
                  power_state_vector* power_states,
                  optimal_config_vector* edl)
{
    struct sockaddr_in addr;
    int sock;
    int ret;
    uint32_t res_len = strlen(resource_id);
    gal_command command = COMMAND_DISCOVERY;
    uint32_t size = sizeof(command) + sizeof(committed) + res_len + 1; // Add +1 for the string terminator char.
    char out_buffer[size];
    char* in_buffer = NULL;
    int i;
    gal_return_code rc;

    char* ptr = init_buffer(out_buffer, command, resource_id);

    memcpy(ptr, &committed, sizeof(committed));

    rc = init_gal_request(&sock, & addr);

    if (rc != GAL_SUCCESS)
    {
        return rc;
    }

    rc = do_send_gal_request(sock, &addr, out_buffer, size);

    if (rc != GAL_SUCCESS)
    {
        return rc;
    }

    rc = read_gal_response_code(sock);

    if (rc != GAL_SUCCESS)
    {
        close(sock);

        return rc;
    }

    ptr = (char*)&size;
    ret = recvfrom(sock, ptr, sizeof(size), 0, NULL, NULL);

    if (ret != sizeof(size))
    {
        close(sock);

        return GAL_FAILURE;
    }

    in_buffer = malloc(size);

    if (in_buffer == NULL)
    {
        close(sock);

        return GAL_FAILURE;
    }

    ptr = in_buffer;

    ret = recvfrom(sock, ptr, size, 0, NULL, NULL);

    close(sock);

    if (ret != size)
    {
        rc = GAL_FAILURE;

        goto cleanup;
    }

    GAL_READ_BLOCK_VECTOR(logical_resources, gal_logical_resource, ptr)
    GAL_READ_BLOCK_VECTOR(physical_resources, gal_physical_resource, ptr)
    GAL_READ_BLOCK_VECTOR(sensor_resources, gal_sensor_resource, ptr)

    GAL_READ_VECTOR_SIZE(power_states, power_state, ptr)

    if (power_states->size > 0)
    {
        size_t ps_size = offsetof(power_state, sleeping_time) + sizeof(double);
        power_state* ps;

        GAL_ALLOC_VECTOR_ELEMENTS(power_states, power_state)

        GAL_FOR_EACH_ELEMENT(power_states, i)
        {
            ps = &power_states->elements[i];

            GAL_COPY_MEMORY(ps, ptr, ps_size)

            GAL_READ_BLOCK_VECTOR(&ps->ps_transition_times, double, ptr)
            GAL_READ_BLOCK_VECTOR(&ps->ps_transition_service_interruption_times, double, ptr)
            GAL_READ_BLOCK_VECTOR(&ps->autonomic_ps_curves, curve_state, ptr)
        }
    }

    GAL_READ_VECTOR_SIZE(edl, optimal_config, ptr)

    if (edl->size > 0)
    {
        optimal_config* oc;

        GAL_ALLOC_VECTOR_ELEMENTS(edl, optimal_config)

        GAL_FOR_EACH_ELEMENT(edl, i)
        {
            oc = &edl->elements[i];

            GAL_COPY_MEMORY(&oc->id, ptr, sizeof(oc->id))

            GAL_READ_VECTOR_BLOCK_ELEMENTS(&oc->power_states, uint16_t, ptr)
        }
    }

    rc = GAL_SUCCESS;
cleanup:
    free(in_buffer);

    return rc;
}

gal_return_code provisioning(const char* resource_id, int power_state_id)
{
    return send_gal_request_with_param(COMMAND_PROVISIONING, resource_id, (const char*)&power_state_id, sizeof(power_state_id));
}

gal_return_code release(const char* resource_id)
{
    return send_gal_simple_request(COMMAND_RELEASE, resource_id);
}

gal_return_code monitor_state(const char* resource_id, bool committed, int* power_state_id)
{
    return send_gal_request_with_param_with_reply(COMMAND_MONITOR_STATE, resource_id, (const char*)&committed, sizeof(committed), (char*)power_state_id, sizeof(*power_state_id));
}

gal_return_code monitor_history(const char* resource_id, power_state_history_vector* history)
{
    struct sockaddr_in addr;
    int sock;
    int ret;
    uint32_t res_len = strlen(resource_id);
    gal_command command = COMMAND_MONITOR_HISTORY;
    uint32_t size = sizeof(command) + res_len + 1; // Add +1 for the string terminator char.
    char out_buffer[size];
    char* in_buffer;
    char* ptr = init_buffer(out_buffer, command, resource_id);
    gal_return_code rc = init_gal_request(&sock, & addr);

    if (rc != GAL_SUCCESS)
    {
        return rc;
    }

    rc = do_send_gal_request(sock, &addr, out_buffer, size);

    if (rc != GAL_SUCCESS)
    {
        close(sock);

        return rc;
    }

    rc = read_gal_response_code(sock);

    if (rc != GAL_SUCCESS)
    {
        close(sock);

        return rc;
    }

    ptr = (char*)&size;

    ret = recvfrom(sock, ptr, sizeof(size), 0, NULL, NULL);

    if (ret == -1)
    {
        close(sock);

        return GAL_FAILURE;
    }

    in_buffer = malloc(size);
    ptr = in_buffer;

    ret = recvfrom(sock, ptr, size, 0, NULL, NULL);

    close(sock);

    if (ret == -1)
    {
        return GAL_FAILURE;
    }

    GAL_READ_BLOCK_VECTOR(history, power_state_history, ptr)

    free(in_buffer);

    return GAL_SUCCESS;
}

gal_return_code monitor_sensor(const char* resource_id, entity_sensor_status* oper_status, entity_sensor_value* sensor_value, entity_sensor_timestamp* value_timestamp)
{
    const size_t size = sizeof(*oper_status) + sizeof(*sensor_value) + sizeof(*value_timestamp); 
    char buffer[size];
    char* ptr = buffer;
    gal_return_code rc = send_gal_request_with_reply(COMMAND_MONITOR_SENSOR, resource_id, buffer, size);

    if (rc != GAL_SUCCESS)
    {
        return rc;
    }

    memcpy(oper_status, ptr, sizeof(*oper_status));
    ptr += sizeof(*oper_status);
    memcpy(sensor_value, ptr, sizeof(*sensor_value));
    ptr += sizeof(*sensor_value);
    memcpy(value_timestamp, ptr, sizeof(*value_timestamp));

    return rc;
}

gal_return_code commit(const char* resource_id)
{
    return send_gal_simple_request(COMMAND_COMMIT, resource_id);
}

gal_return_code rollback(const char* resource_id)
{
    return send_gal_simple_request(COMMAND_ROLLBACK, resource_id);
}

static char* init_buffer(char buffer[], gal_command command, const char* resource_id)
{
    char* ptr = buffer;
    uint32_t res_len = strlen(resource_id);

    memcpy(ptr, &command, sizeof(command));
    ptr += sizeof(command);
    strncpy(ptr, resource_id, res_len);
    ptr += res_len;
    *ptr = '\0';
    ++ptr;

    return ptr;
}

static gal_return_code init_gal_request(int* sock, struct sockaddr_in* addr)
{
    *sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (*sock < 0)
    {
		printf("%s\n", strerror(errno));
        return GAL_FAILURE;
    }

    memset(addr, 0, sizeof(addr));

    addr->sin_family = AF_INET;
    addr->sin_port = htons(LPORT);
    addr->sin_addr.s_addr = htonl(LADDRESS);

    if (bind(*sock, (struct sockaddr*)addr, sizeof(*addr)) == -1)
    {
        close(*sock);
		printf("%s\n", strerror(errno));
        return GAL_FAILURE;
    }

    addr->sin_port = htons(RPORT);
    addr->sin_addr.s_addr = htonl(RADDRESS);

    return GAL_SUCCESS;
}

static gal_return_code read_gal_response_code(int sock)
{
    int32_t code;
    int ret = recvfrom(sock, (char*)&code, sizeof(code), 0, NULL, NULL);

    if (ret != sizeof(code))
    {
        return GAL_FAILURE;
    }

    return (gal_return_code)code;
}

static gal_return_code do_send_gal_request(int sock, struct sockaddr_in* addr, const char* buffer, size_t size)
{
    int ret = sendto(sock, buffer, size, 0, (struct sockaddr*)addr, sizeof(*addr));

    if (ret != size)
    {
        close(sock);

        return GAL_FAILURE;
    }

    return GAL_SUCCESS;
}

static gal_return_code send_gal_request(const char* buffer, size_t size)
{
    struct sockaddr_in addr;
    int sock;

    gal_return_code rc = init_gal_request(&sock, & addr);

    if (rc != GAL_SUCCESS)
    {
        return rc;
    }

    rc = do_send_gal_request(sock, &addr, buffer, size);

	if (rc != GAL_SUCCESS)
    {
		close(sock);

        return rc;
    }

	rc = read_gal_response_code(sock);
	close(sock);

	return rc;
}

static gal_return_code send_gal_simple_request(gal_command command, const char* resource_id)
{
    size_t res_len = strlen(resource_id);
    size_t size = sizeof(command) + res_len + 1; // Add +1 for the string terminator char.

    char buffer[size];
    char* ptr = buffer;

    memcpy(ptr, &command, sizeof(command));
    ptr += sizeof(command);
    strncpy(ptr, resource_id, res_len);
	ptr += res_len;
	*ptr = '\0';

    return send_gal_request(buffer, size);
}

static gal_return_code send_gal_request_with_param(gal_command command, const char* resource_id, const char* param, size_t param_size)
{
    size_t res_len = strlen(resource_id);
    size_t size = sizeof(command) + param_size + res_len + 1; // Add +1 for the string terminator char.

    char buffer[size];
    char* ptr = buffer;

    memcpy(ptr, &command, sizeof(command));
    ptr += sizeof(command);
    strncpy(ptr, resource_id, res_len);
    ptr += res_len;
	*ptr = '\0';
	++ptr;
    memcpy(ptr, param, param_size);

    return send_gal_request(buffer, size);
}

static gal_return_code send_gal_request_with_reply(gal_command command, const char* resource_id, char* param, size_t param_size)
{
    struct sockaddr_in addr;
    int sock;
    int ret;
    size_t res_len = strlen(resource_id);
    size_t size = sizeof(command) + res_len + 1; // Add +1 for the string terminator char.

    char buffer[size];
    char* ptr = buffer;

    memcpy(ptr, &command, sizeof(command));
    ptr += sizeof(command);
    strncpy(ptr, resource_id, res_len);

    gal_return_code rc = init_gal_request(&sock, & addr);

    if (rc != GAL_SUCCESS)
    {
        return rc;
    }

    rc = do_send_gal_request(sock, &addr, buffer, size);

    if (rc != GAL_SUCCESS)
    {
        return rc;
    }

    rc = read_gal_response_code(sock);

    if (rc != GAL_SUCCESS)
    {
        close(sock);

        return rc;
    }

    ret = recvfrom(sock, param, param_size, 0, NULL, NULL);
	close(sock);

    if (ret != param_size)
    {
        return GAL_FAILURE;
    }

    return GAL_SUCCESS;
}

static gal_return_code send_gal_request_with_param_with_reply(gal_command command, const char* resource_id, const char* in_param, size_t in_param_size, char* out_param, size_t out_param_size)
{
    struct sockaddr_in addr;
    int sock;
    int ret;
    size_t res_len = strlen(resource_id);
    size_t size = sizeof(command) + in_param_size + res_len + 1; // Add +1 for the string terminator char.

    char buffer[size];
    char* ptr = buffer;

    memcpy(ptr, &command, sizeof(command));
    ptr += sizeof(command);
    strncpy(ptr, resource_id, res_len);
	ptr += res_len;
	*ptr = '\0';
	++ptr;
    memcpy(ptr, in_param, in_param_size);

    gal_return_code rc = init_gal_request(&sock, & addr);

    if (rc != GAL_SUCCESS)
    {
        return rc;
    }

    rc = do_send_gal_request(sock, &addr, buffer, size);

    if (rc != GAL_SUCCESS)
    {
        return rc;
    }

    rc = read_gal_response_code(sock);

    if (rc != GAL_SUCCESS)
    {
        close(sock);

        return rc;
    }

    ret = recvfrom(sock, out_param, out_param_size, 0, NULL, NULL);
	close(sock);

    if (ret != out_param_size)
    {
        return GAL_FAILURE;
    }

    return GAL_SUCCESS;
}
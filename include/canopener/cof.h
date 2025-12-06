#ifndef COF_H
#define COF_H

#include <stdint.h>
#include <string.h>

// Virtual properties
#define COF_TYPE            1
#define COF_NODE_ID         2
#define COF_SDO_INDEX       3
#define COF_SDO_SUB         4
#define COF_SDO_ABORT_CODE  5
#define COF_SDO_EXPEDITED   6
#define COF_SDO_SIZE_IND    7
#define COF_SDO_EXP_SIZE    8
#define COF_SDO_EXP_DATA    9
#define COF_PDO_DATA        10
#define COF_PDO_CHANNEL     11
#define COF_COB_ID          12
#define COF_DLC             13
#define COF_DATA            14

// Message types
#define COF_TYPE_UNKNOWN               0x00
#define COF_TYPE_SDO_UPLOAD_REQUEST    0x10
#define COF_TYPE_SDO_DOWNLOAD_REQUEST  0x11
#define COF_TYPE_SDO_UPLOAD_REPLY      0x12
#define COF_TYPE_SDO_DOWNLOAD_REPLY    0x13
#define COF_TYPE_SDO_ABORT             0x14
#define COF_TYPE_PDO_TX                0x20
#define COF_TYPE_PDO_RX                0x21
#define COF_TYPE_SYNC                  0x30
#define COF_TYPE_TIME                  0x31
#define COF_TYPE_EMCY                  0x32
#define COF_TYPE_HEARTBEAT             0x33

// Abort codes
#define COF_ABORT_TOGGLE             0x05030000 // Toggle bit not alternated
#define COF_ABORT_TIMEOUT            0x05040000 // SDO protocol timed out
#define COF_ABORT_COMMAND            0x05040001 // Command specifier not valid or unknown
#define COF_ABORT_OUT_OF_MEMORY      0x05040005 // Out of memory
#define COF_ABORT_UNSUPPORTED_ACCESS 0x06010000 // Unsupported access to an object
#define COF_ABORT_NOT_EXIST          0x06020000 // Object does not exist in OD
#define COF_ABORT_READONLY           0x06010001 // Attempt to write a read-only object
#define COF_ABORT_TYPE_MISMATCH      0x06070010 // Data type does not match
#define COF_ABORT_LENGTH_EXCEEDED    0x06070012 // Length of service parameter too high
#define COF_ABORT_SUBINDEX           0x06090011 // Subindex does not exist
#define COF_ABORT_DEVICE_STATE       0x08000000 // Device internal error

typedef struct {
    uint32_t id;
    uint8_t  len;
    uint8_t  data[8];
} cof_t;

void cof_init(cof_t *frame);
void cof_set(cof_t *frame, int prop, uint32_t value);
uint32_t cof_get(const cof_t *frame, int prop);
uint8_t *cof_getp(cof_t *f, int prop);
char *cof_to_slcan(cof_t *f, char *s);
cof_t *cof_from_slcan(cof_t *f, const char *buf);
void cof_cpy(cof_t *dest, cof_t *src);

#endif // COF_H

#include "canopener/cof.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Initialize frame */
void cof_init(cof_t *frame) {
    frame->id = 0;
    memset(frame->data, 0, 8);//COF_MAX_DATA);
    frame->len = 8;
}

/* Apply type to default COB-ID and first byte */
static void cof_apply_type(cof_t *frame, uint32_t type) {
    cof_init(frame);

    switch(type) {
        case COF_TYPE_SDO_UPLOAD_REQUEST:   frame->id = 0x600; frame->data[0] = 0x40; break;
        case COF_TYPE_SDO_DOWNLOAD_REQUEST: frame->id = 0x600; frame->data[0] = 0x20; break;
        case COF_TYPE_SDO_UPLOAD_REPLY:     frame->id = 0x580; frame->data[0] = 0x43; break;
        case COF_TYPE_SDO_DOWNLOAD_REPLY:   frame->id = 0x580; frame->data[0] = 0x60; break;
        case COF_TYPE_SDO_ABORT:            frame->id = 0x580; frame->data[0] = 0x80; break;
        case COF_TYPE_PDO_TX:               frame->id = 0x180; break; // channel to be applied later
        case COF_TYPE_PDO_RX:               frame->id = 0x200; break; // channel to be applied later
        case COF_TYPE_SYNC:                  frame->id = 0x080; break;
        case COF_TYPE_TIME:                  frame->id = 0x100; break;
        case COF_TYPE_EMCY:                  frame->id = 0x080; break;
        case COF_TYPE_HEARTBEAT:             frame->id = 0x700; break;
        default: frame->id = 0; break;
    }

    switch(type) {
        case COF_TYPE_SDO_UPLOAD_REQUEST:
        case COF_TYPE_SDO_DOWNLOAD_REQUEST:
        case COF_TYPE_SDO_UPLOAD_REPLY:
        case COF_TYPE_SDO_ABORT:
            frame->len = 8;
            break;
        case COF_TYPE_SDO_DOWNLOAD_REPLY:
            frame->len = 4;
            break;
        case COF_TYPE_PDO_TX:
        case COF_TYPE_PDO_RX:
            frame->len = 8; // could be overridden by user
            break;
        case COF_TYPE_SYNC:
            frame->len = 0;
            break;
        case COF_TYPE_TIME:
            frame->len = 6;
            break;
        case COF_TYPE_EMCY:
            frame->len = 8;
            break;
        case COF_TYPE_HEARTBEAT:
            frame->len = 1;
            break;
        default:
            frame->len = 8;
            break;
    }
}

/* Set virtual property */
void cof_set(cof_t *frame, int prop, uint32_t value) {
    switch(prop) {

        case COF_TYPE:
            cof_apply_type(frame, value);
            break;

        case COF_NODE_ID: {
            uint8_t node = value & 0x7F;
            uint32_t type = cof_get(frame, COF_TYPE);
            switch(type) {
                case COF_TYPE_SDO_UPLOAD_REQUEST:
                case COF_TYPE_SDO_DOWNLOAD_REQUEST: frame->id = 0x600 | node; break;
                case COF_TYPE_SDO_UPLOAD_REPLY:
                case COF_TYPE_SDO_DOWNLOAD_REPLY:
                case COF_TYPE_SDO_ABORT:            frame->id = 0x580 | node; break;
                case COF_TYPE_PDO_TX: case COF_TYPE_PDO_RX:
                case COF_TYPE_EMCY: case COF_TYPE_HEARTBEAT:
                    frame->id = (frame->id & 0xF00) | node; break;
                default: frame->id = node; break;
            }
            break;
        }

        case COF_SDO_INDEX:
            frame->data[1] = value & 0xFF;
            frame->data[2] = (value >> 8) & 0xFF;
            break;

        case COF_SDO_SUB:
            frame->data[3] = value & 0xFF;
            break;

        case COF_SDO_ABORT_CODE:
            frame->data[4] = (value >> 0) & 0xFF;
            frame->data[5] = (value >> 8) & 0xFF;
            frame->data[6] = (value >> 16) & 0xFF;
            frame->data[7] = (value >> 24) & 0xFF;
            break;

        case COF_SDO_EXPEDITED:
            if(value) frame->data[0] |= 0x02;
            else frame->data[0] &= ~0x02;
            break;

        case COF_SDO_SIZE_IND:
            if(value) frame->data[0] |= 0x01;
            else frame->data[0] &= ~0x01;
            break;

        case COF_SDO_EXP_SIZE: {
            uint8_t n = 4 - (value & 0x03);
            frame->data[0] = (frame->data[0] & ~0x0C) | ((n << 2) & 0x0C);
            // should adjust the size of the frame
            break;
        }

        case COF_PDO_CHANNEL: {
            uint8_t ch = value & 0x03;
            uint32_t type = cof_get(frame, COF_TYPE);
            uint8_t node = cof_get(frame, COF_NODE_ID) & 0x7F;
            if(type == COF_TYPE_PDO_TX)
                frame->id = 0x180 + (ch << 7) + node;
            else if(type == COF_TYPE_PDO_RX)
                frame->id = 0x200 + (ch << 7) + node;
            break;
        }

        case COF_PDO_DATA:
            memcpy(frame->data, &value, frame->len > 4 ? 4 : frame->len);
            break;

        case COF_COB_ID:
            frame->id=value;
            break;

        default: break;
    }
}

/* Get virtual property */
uint32_t cof_get(const cof_t *frame, int prop) {
    switch(prop) {
        case COF_TYPE:
            switch(frame->data[0] & 0xE0) {
                case 0x40: return COF_TYPE_SDO_UPLOAD_REQUEST;
                case 0x20: return COF_TYPE_SDO_DOWNLOAD_REQUEST;
                case 0x43: return COF_TYPE_SDO_UPLOAD_REPLY;
                case 0x60: return COF_TYPE_SDO_DOWNLOAD_REPLY;
                case 0x80: return COF_TYPE_SDO_ABORT;
                default:
                    if ((frame->id & 0x780) == 0x180) return COF_TYPE_PDO_TX;
                    if ((frame->id & 0x780) == 0x200) return COF_TYPE_PDO_RX;
                    if ((frame->id & 0x700) == 0x700) return COF_TYPE_HEARTBEAT;
                    if ((frame->id & 0x700) == 0x080) return COF_TYPE_EMCY;
                    if ((frame->id & 0x700) == 0x080) return COF_TYPE_SYNC;
                    if ((frame->id & 0x700) == 0x100) return COF_TYPE_TIME;
                    return COF_TYPE_UNKNOWN;
            }

        case COF_NODE_ID:
            switch(cof_get(frame, COF_TYPE)) {
                case COF_TYPE_SDO_UPLOAD_REQUEST:
                case COF_TYPE_SDO_DOWNLOAD_REQUEST:
                case COF_TYPE_SDO_UPLOAD_REPLY:
                case COF_TYPE_SDO_DOWNLOAD_REPLY:
                case COF_TYPE_SDO_ABORT:
                case COF_TYPE_PDO_TX:
                case COF_TYPE_PDO_RX:
                case COF_TYPE_EMCY:
                case COF_TYPE_HEARTBEAT:
                    return frame->id & 0x7F;
                default: return frame->id;
            }

        case COF_SDO_INDEX: return frame->data[1] | (frame->data[2] << 8);
        case COF_SDO_SUB: return frame->data[3];

        case COF_SDO_ABORT_CODE:
            return (uint32_t)frame->data[4]
                 | ((uint32_t)frame->data[5] << 8)
                 | ((uint32_t)frame->data[6] << 16)
                 | ((uint32_t)frame->data[7] << 24);

        case COF_SDO_EXPEDITED: return (frame->data[0] & 0x02) != 0;
        case COF_SDO_SIZE_IND: return (frame->data[0] & 0x01) != 0;
        case COF_SDO_EXP_SIZE: return 4 - ((frame->data[0] >> 2) & 0x03);

        case COF_PDO_CHANNEL:
            if(cof_get(frame, COF_TYPE) != COF_TYPE_PDO_TX &&
               cof_get(frame, COF_TYPE) != COF_TYPE_PDO_RX)
                return 0;
            return (frame->id >> 7) & 0x03;

        case COF_PDO_DATA:
            return *((uint32_t*)&frame->data[0]);

        case COF_COB_ID:
            return frame->id;

        case COF_DLC:
            return frame->len;

        default: return 0;
    }
}

uint8_t *cof_getp(cof_t *f, int prop) {
    switch (prop) {
        case COF_DATA:
            return f->data;

        case COF_SDO_EXP_DATA:
            return &f->data[4];

        default:
            return NULL;
    }
}


char *cof_to_slcan(cof_t *f, char *buf) {
    uint8_t pos = 0;

    if (cof_get(f,COF_COB_ID) > 0x7FF) {
        buf[pos++] = 'T'; // extended
        sprintf(buf + pos, "%08lX", cof_get(f,COF_COB_ID));
        pos += 8;
    } else {
        buf[pos++] = 't'; // standard
        sprintf(buf + pos, "%03lX", cof_get(f,COF_COB_ID));
        pos += 3;
    }

    //printf("size: %d\n",cof_get(f,COF_DLC));
    buf[pos++] = '0' + cof_get(f,COF_DLC);
    //printf("size: %d\n",cof_get(f,COF_DLC));

    for (uint8_t i = 0; i < cof_get(f,COF_DLC); i++) {
        //printf("size: %d, i=%d\n",cof_get(f,COF_DLC),i);
        sprintf(buf+pos, "%02X", cof_getp(f,COF_DATA)[i]);
        pos += 2;
    }

    buf[pos] = 0;

    return buf;
}

cof_t *cof_from_slcan(cof_t *f, const char *buf) {
    bool ext = false;

    if (buf[0] == 't') ext = false;
    else if (buf[0] == 'T') ext = true;
    else { return NULL; }

    uint8_t pos = 1;

    // Parse ID
    uint8_t idChars = ext ? 8 : 3;
    char ids[8];
    memcpy(ids,buf+pos,idChars);
    ids[idChars]='\n';

    f->id = strtoul(ids, nullptr, 16);
    pos += idChars;

    // DLC
    f->len = buf[pos++] - '0';
    if (f->len > 8) f->len = 8;

    // Data bytes
    for (uint8_t i = 0; i < f->len; i++) {
        char byteStr[3] = { buf[pos], buf[pos+1], 0 };
        f->data[i] = strtoul(byteStr, nullptr, 16);
        pos += 2;
    }

    return f;
}

void cof_cpy(cof_t *dest, cof_t *src) {
    memcpy(dest,src,sizeof(cof_t));
}

#include "canopener.h"
#include <stdio.h>

using namespace canopener;

void canopener::handleSdoExpeditedRead(Device &dev, cof_t *frame) {
    //printf("here, type=%02x nodeid=%d\n",cof_get(frame,COF_TYPE),cof_get(frame,COF_NODE_ID)); fflush(stdout);

    if (cof_get(frame,COF_TYPE)!=COF_TYPE_SDO_UPLOAD_REQUEST ||
            cof_get(frame,COF_NODE_ID)!=dev.getNodeId())
        return;

    uint16_t idx=cof_get(frame,COF_SDO_INDEX);
    uint8_t sub=cof_get(frame,COF_SDO_SUB);
    Entry *e=dev.find(idx, sub);
    if (!e) {
        //printf("will abort!!!\n");
        cof_t abort;
        cof_set(&abort,COF_TYPE,COF_TYPE_SDO_ABORT); 
        cof_set(&abort,COF_NODE_ID,dev.getNodeId()); 
        cof_set(&abort,COF_SDO_INDEX,idx); 
        cof_set(&abort,COF_SDO_SUB,sub);
        cof_set(&abort,COF_SDO_ABORT_CODE,COF_ABORT_NOT_EXIST);
        dev.getBus().write(&abort);
        return;
    }

    const auto &bytes = e->raw();
    size_t size = bytes.size();
    if (size>4)
        size=4;

    cof_t reply;
    cof_set(&reply,COF_TYPE,COF_TYPE_SDO_UPLOAD_REPLY); 
    cof_set(&reply,COF_NODE_ID,dev.getNodeId()); 
    cof_set(&reply,COF_SDO_INDEX,idx); 
    cof_set(&reply,COF_SDO_SUB,sub);
    cof_set(&reply,COF_SDO_EXPEDITED,1);
    cof_set(&reply,COF_SDO_SIZE_IND,1);
    cof_set(&reply,COF_SDO_EXP_SIZE,size);
    memcpy(cof_getp(&reply,COF_SDO_EXP_DATA),bytes.data(),size);
    dev.getBus().write(&reply);

    //printf("it is an upload request!!!\n"); fflush(stdout);
}

void canopener::handleSdoExpeditedWrite(Device &dev, cof_t *frame) {
    //printf("here, type=%02x\n",cof_get(frame,COF_TYPE));

    if (cof_get(frame,COF_TYPE)!=COF_TYPE_SDO_DOWNLOAD_REQUEST ||
            !cof_get(frame,COF_SDO_EXPEDITED) ||
            cof_get(frame,COF_NODE_ID)!=dev.getNodeId())
        return;

    //printf("yes!!!\n"); fflush(stdout);
    uint16_t idx=cof_get(frame,COF_SDO_INDEX);
    uint8_t sub=cof_get(frame,COF_SDO_SUB);
    Entry *e=dev.find(idx, sub);
    if (!e) {
        //printf("will abort!!!\n");
        cof_t abort;
        cof_set(&abort,COF_TYPE,COF_TYPE_SDO_ABORT); 
        cof_set(&abort,COF_NODE_ID,dev.getNodeId()); 
        cof_set(&abort,COF_SDO_INDEX,idx); 
        cof_set(&abort,COF_SDO_SUB,sub);
        cof_set(&abort,COF_SDO_ABORT_CODE,COF_ABORT_NOT_EXIST);
        dev.getBus().write(&abort);
        return;
    }

    //printf("setting, expedited size: %d\n",cof_get(frame,COF_SDO_EXP_SIZE));

    size_t size=cof_get(frame,COF_SDO_EXP_SIZE);
    if (size>e->raw().size())
        size=e->raw().size();

    e->setRaw(cof_getp(frame,COF_SDO_EXP_DATA),size);

    cof_t reply;
    cof_set(&reply,COF_TYPE,COF_TYPE_SDO_DOWNLOAD_REPLY); 
    cof_set(&reply,COF_NODE_ID,dev.getNodeId()); 
    cof_set(&reply,COF_SDO_INDEX,idx); 
    cof_set(&reply,COF_SDO_SUB,sub);
    dev.getBus().write(&reply);
}

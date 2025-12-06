#include "canopener.h"
#include <stdio.h>

using namespace canopener;

/*void canopener::handleSdoExpeditedRead(Device &dev, Message &msg) {
    if (!msg.isSdoExpeditedRead())
        return;

    if (msg.getNodeId()!=dev.getNodeId())
        return;

    uint16_t idx = msg.index();
    uint8_t sub = msg.subindex();

    Entry *e = dev.find(idx, sub);
    if (!e) {
        dev.send(Message::sdoAbort(dev.getNodeId(), idx, sub, 0x06020000)); // object does not exist
        return;
    }

    const auto &bytes = e->raw();
    size_t size = bytes.size();

    if (size > 4) {
        // not expedited
        dev.send(Message::sdoAbort(dev.getNodeId(), idx, sub, 0x06010000));
        return;
    }

    dev.send(Message::sdoExpeditedReadReply(
        dev.getNodeId(),
        idx,
        sub,
        bytes.data(),
        size
    ));
}*/

void canopener::handleSdoExpeditedWrite(Device &dev, cof_t *frame) {
    if (cof_get(frame,COF_TYPE)!=COF_TYPE_SDO_DOWNLOAD_REQUEST ||
            !cof_get(frame,COF_SDO_EXPEDITED) ||
            cof_get(frame,COF_NODE_ID)!=dev.getNodeId())
        return;

    //printf("yes!!!\n");
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
    e->setRaw(cof_getp(frame,COF_SDO_EXP_DATA),size);

    cof_t reply;
    cof_set(&reply,COF_TYPE,COF_TYPE_SDO_DOWNLOAD_REPLY); 
    cof_set(&reply,COF_NODE_ID,dev.getNodeId()); 
    cof_set(&reply,COF_SDO_INDEX,idx); 
    cof_set(&reply,COF_SDO_SUB,sub);
    dev.getBus().write(&reply);
}

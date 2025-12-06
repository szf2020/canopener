export class CanOpenMessage {
    constructor(frame) {
        this.frame = frame;  // {id, data, ext, rtr}
        this.data = frame.data;
        this.id = frame.id;
    }

    //
    // ---------- Node ID ----------
    //
    get nodeId() {
        // SDO client->server: 0x600 + node
        // SDO server->client: 0x580 + node
        if ((this.id & 0x780) === 0x600)
            return this.id - 0x600;

        if ((this.id & 0x780) === 0x580)
            return this.id - 0x580;

        return null; // Not an SDO
    }

    //
    // ---------- Type checks ----------
    //
    isSdoRequest() {
        return (this.id & 0x780) === 0x600;
    }

    isSdoResponse() {
        return (this.id & 0x780) === 0x580;
    }

    isSdoExpeditedReadReply() {
        return this.isSdoResponse() && (this.data[0] & 0xE0) === 0x40;
    }

    isSdoExpeditedWriteReply() {
        return this.isSdoResponse() && this.data[0] === 0x60;
    }

    isSdoAbort() {
        return this.isSdoResponse() && this.data[0] === 0x80;
    }

    //
    // ---------- Parsed fields ----------
    //
    get index() {
        return this.data[1] | (this.data[2] << 8);
    }

    get sub() {
        return this.data[3];
    }

    get abortCode() {
        if (!this.isSdoAbort()) return null;
        return (
            this.data[4] |
            (this.data[5] << 8) |
            (this.data[6] << 16) |
            (this.data[7] << 24)
        ) >>> 0;
    }

    //
    // ---------- Payload (expedited only) ----------
    //
    get expeditedPayload() {
        const ccs = this.data[0];

        // expedited bit = bit 1
        if ((ccs & 0x02) === 0) return null;

        // size indicated = bit 0
        const sizeIndicated = (ccs & 0x01) !== 0;

        let size = 4;
        if (sizeIndicated) {
            const n = (ccs >> 2) & 0x03;  // number of unused bytes
            size = 4 - n;
        }

        return this.data.slice(4, 4 + size);
    }

    //
    // ---------- Static constructors ----------
    //

    // SDO expedited read request
    static sdoExpeditedRead(nodeId, index, sub) {
        return new CanOpenMessage({
            id: 0x600 + nodeId,
            ext: false,
            rtr: false,
            data: new Uint8Array([
                0x40,                      // CCS = upload request
                index & 0xFF,
                (index >> 8) & 0xFF,
                sub,
                0, 0, 0, 0
            ])
        });
    }

    // SDO expedited write request
    static sdoExpeditedWrite(nodeId, index, sub, payload) {
        const bytes = payload instanceof Uint8Array
            ? payload
            : Uint8Array.from(payload); // allow Array or Buffer too

        if (bytes.length > 4)
            throw new Error("Expedited write supports max 4 bytes");

        const n = 4 - bytes.length;  // unused bytes

        const ccs = 0x20                 // download request
                  | 0x02                 // expedited
                  | 0x01                 // size indicated
                  | (n << 2);            // unused bytes count

        const data = new Uint8Array(8);
        data[0] = ccs;
        data[1] = index & 0xFF;
        data[2] = (index >> 8) & 0xFF;
        data[3] = sub;
        data.set(bytes, 4);

        return new CanOpenMessage({
            id: 0x600 + nodeId,
            ext: false,
            rtr: false,
            data
        });
    }

    // SDO abort reply
    static sdoAbort(nodeId, index, sub, abortCode) {
        const data = new Uint8Array(8);
        data[0] = 0x80;
        data[1] = index & 0xFF;
        data[2] = (index >> 8) & 0xFF;
        data[3] = sub;
        data[4] = abortCode & 0xFF;
        data[5] = (abortCode >> 8) & 0xFF;
        data[6] = (abortCode >> 16) & 0xFF;
        data[7] = (abortCode >> 24) & 0xFF;

        return new CanOpenMessage({
            id: 0x580 + nodeId,
            ext: false,
            rtr: false,
            data
        });
    }
}

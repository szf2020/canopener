import COF from "./cof.js";
import {awaitEvent} from "./js-util.js";
import {slcanStringify} from "./SlcanBus.js";

export default class RemoteEntry {
	constructor(index, subIndex) {
		this.index=index;
		this.subIndex=subIndex;
		this.setType("int32");
	}

  	getTypeSize() {
  		switch (this.type) {
			case "int8":
			case "uint8":
			case "bool": return 1;
			case "int16":
			case "uint16": return 2;
			case "int32":
			case "uint32":
			case "float32": return 4;
			case "string": return 0;
			default:
				throw new Error("Unknown type: "+this.type);
		}
  	}

  	setType(type) {
  		this.type=type;
		this.buffer=new ArrayBuffer(this.getTypeSize());
		this.view=new DataView(this.buffer);
  	}

	async expeditedRead() {
		let cof=COF._cof_create();
		COF._cof_set(cof,COF.COF_TYPE,COF.COF_TYPE_SDO_UPLOAD_REQUEST);
        COF._cof_set(cof,COF.COF_NODE_ID,this.device.getNodeId()); 
        COF._cof_set(cof,COF.COF_SDO_INDEX,this.index); 
        COF._cof_set(cof,COF.COF_SDO_SUB,this.subIndex);
        let o=COF.cofToObject(cof);
        COF._cof_dispose(cof);

        this.device.bus.send(o);
        await awaitEvent(this.device.bus,"message",m=>{
			let cof=COF.cofFromObject(m);
			//console.log("t: "+COF._cof_get(cof,COF.COF_TYPE));

			if (COF._cof_get(cof,COF.COF_TYPE)==COF.COF_TYPE_SDO_UPLOAD_REPLY &&
					COF._cof_get(cof,COF.COF_SDO_INDEX)==this.index &&
					COF._cof_get(cof,COF.COF_SDO_SUB)==this.subIndex) {
				let size=COF._cof_get(cof,COF.COF_SDO_EXP_SIZE);
		        let p=COF._cof_getp(cof,COF.COF_SDO_EXP_DATA);
		        let d=new Uint8Array(this.buffer);
		        for (let i=0; i<size; i++)
			        d[i]=COF.HEAPU8[p+i];

		        COF._cof_dispose(cof);
				return true;
			}

			if (COF._cof_get(cof,COF.COF_TYPE)==COF.COF_TYPE_SDO_ABORT &&
					COF._cof_get(cof,COF.COF_SDO_INDEX)==this.index &&
					COF._cof_get(cof,COF.COF_SDO_SUB)==this.subIndex) {
				let e=COF.sdoErrors[COF._cof_get(cof,COF.COF_SDO_ABORT_CODE)];
		        COF._cof_dispose(cof);
		    	throw new Error(e);
			}

			else {
		        COF._cof_dispose(cof);
			}
        });
	}

	async expeditedWrite() {
		let expWriteSize=Math.min(4,this.buffer.byteLength);
		//console.log("doing expedited write: "+expWriteSize);

		let cof=COF._cof_create();
		COF._cof_set(cof,COF.COF_TYPE,COF.COF_TYPE_SDO_DOWNLOAD_REQUEST);
		COF._cof_set(cof,COF.COF_SDO_EXPEDITED,1);
		COF._cof_set(cof,COF.COF_SDO_SIZE_IND,1);
		COF._cof_set(cof,COF.COF_SDO_EXP_SIZE,expWriteSize);
        COF._cof_set(cof,COF.COF_NODE_ID,this.device.getNodeId()); 
        COF._cof_set(cof,COF.COF_SDO_INDEX,this.index); 
        COF._cof_set(cof,COF.COF_SDO_SUB,this.subIndex);

        let p=COF._cof_getp(cof,COF.COF_SDO_EXP_DATA);
        let d=new Uint8Array(this.buffer);
        for (let i=0; i<expWriteSize; i++)
	        COF.HEAPU8[p+i]=d[i];

        let o=COF.cofToObject(cof);
        COF._cof_dispose(cof);

        this.device.bus.send(o);
        await awaitEvent(this.device.bus,"message",m=>{
			let cof=COF.cofFromObject(m);
			if (COF._cof_get(cof,COF.COF_TYPE)==COF.COF_TYPE_SDO_DOWNLOAD_REPLY &&
					COF._cof_get(cof,COF.COF_SDO_INDEX)==this.index &&
					COF._cof_get(cof,COF.COF_SDO_SUB)==this.subIndex) {
		        COF._cof_dispose(cof);
				return true;
			}

			if (COF._cof_get(cof,COF.COF_TYPE)==COF.COF_TYPE_SDO_ABORT &&
					COF._cof_get(cof,COF.COF_SDO_INDEX)==this.index &&
					COF._cof_get(cof,COF.COF_SDO_SUB)==this.subIndex) {
				let e=COF.sdoErrors[COF._cof_get(cof,COF.COF_SDO_ABORT_CODE)];
		        COF._cof_dispose(cof);
		    	throw new Error(e);
			}

			else {
		        COF._cof_dispose(cof);
			}
        });
	}

	async set(value) {
		switch (this.type) {
			case "int8":    this.view.setInt8(0,value,true); break;
			case "uint8":   this.view.setUint8(0,value,true); break;
			case "int16":   this.view.setInt16(0,value,true); break;
			case "uint16":  this.view.setUint16(0,value,true); break;
			case "int32":   this.view.setInt32(0,value,true); break;
			case "uint32":  this.view.setUint32(0,value,true); break;
			case "float32": this.view.setFloat32(0,value,true); break;
			case "bool":    this.view.setUint8(0,value?1:0,true); break;
			case "string":
				throw new Error("WIP!!!");

			default:
				throw new Error("Unsuppordet type: "+this.type);
		}

		if (!this.device)
			return;

		return await this.expeditedWrite();
	}

	get() {
		switch (this.type) {
			case "int8":    return this.view.getInt8(0,true); break;
			case "uint8":   return this.view.getUint8(0,true); break;
			case "int16":   return this.view.getInt16(0,true); break;
			case "uint16":  return this.view.getUint16(0,true); break;
			case "int32":   return this.view.getInt32(0,true); break;
			case "uint32":  return this.view.getUint32(0,true); break;
			case "float32": return this.view.getFloat32(0,true); break;
			case "bool":    return !!this.view.getUint8(0,true); break;
			case "string":
				throw new Error("WIP!!!");

			default:
				throw new Error("Unsupported type: "+this.type);
		}
	}

	async refresh() {
		await this.expeditedRead();
	}
}
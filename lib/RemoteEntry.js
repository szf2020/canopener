import COF from "./cof.js";
import {awaitEvent, EventEmitter} from "./js-util.js";

export default class RemoteEntry extends EventEmitter {
	constructor(index, subIndex) {
		super();
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
		return this;
  	}

  	subscribe({interval}={}) {
  		if (this.subscribeInterval)
  			clearInterval(this.subscribeInterval);

  		if (interval) {
  			this.subscribeInterval=setInterval(async ()=>{
  				let old=this.get();
  				await this.refresh();
  				if (this.get()!=old)
  					this.emit("change");
  			},interval);
  		}

  		return this;
  	}

	async expeditedRead() {
		let cof=new COF()
		cof.set(COF.COF_FUNC, COF.COF_FUNC_SDO_RX);
		cof.set(COF.COF_NODE_ID, this.device.getNodeId());
		cof.set(COF.COF_SDO_CMD, COF.COF_SDO_CMD_UPLOAD);
        cof.set(COF.COF_SDO_INDEX,this.index); 
        cof.set(COF.COF_SDO_SUBINDEX,this.subIndex);
        this.device.getBus().send(cof);

        await awaitEvent(this.device.getBus(),"message",m=>{
			let cof=new COF(m);
			//console.log("t: "+COF._cof_get(cof,COF.COF_TYPE));

			if (cof.get(COF.COF_FUNC)==COF.COF_FUNC_SDO_TX &&
					cof.get(COF.COF_SDO_CMD)==COF.COF_SDO_SCS_UPLOAD_REPLY &&
					cof.get(COF.COF_SDO_INDEX)==this.index &&
					cof.get(COF.COF_SDO_SUBINDEX)==this.subIndex) {
				let size=4-cof.get(COF.COF_SDO_N_UNUSED);
		        let d=new Uint8Array(this.buffer);
		        for (let i=0; i<size; i++)
	    		    d[i]=cof.data[4+i];

				return true;
			}

			if (cof.get(COF.COF_FUNC)==COF.COF_FUNC_SDO_TX &&
					cof.get(COF.COF_SDO_CMD)==COF.COF_SDO_CMD_ABORT &&
					cof.get(COF.COF_SDO_INDEX)==this.index &&
					cof.get(COF.COF_SDO_SUBINDEX)==this.subIndex) {
		    	throw new Error(COF.sdoErrors[cof.get(COF.COF_SDO_ABORT_CODE)]);
		   	}
        });
	}

	async expeditedWrite() {
		let expWriteSize=Math.min(4,this.buffer.byteLength);
		//console.log("doing expedited write: "+expWriteSize);

		let cof=new COF();
		cof.set(COF.COF_FUNC, COF.COF_FUNC_SDO_RX);
		cof.set(COF.COF_NODE_ID, this.device.getNodeId());
		cof.set(COF.COF_SDO_CMD, COF.COF_SDO_CMD_DOWNLOAD);
		cof.set(COF.COF_SDO_EXPEDITED, 1);
		cof.set(COF.COF_SDO_SIZE_IND, 1);
		cof.set(COF.COF_SDO_N_UNUSED,4 - expWriteSize);
		cof.set(COF.COF_SDO_INDEX, this.index);
		cof.set(COF.COF_SDO_SUBINDEX, this.subIndex);

        let d=new Uint8Array(this.buffer);
        for (let i=0; i<expWriteSize; i++)
	        cof.data[4+i]=d[i];

        this.device.getBus().send(cof);
        await awaitEvent(this.device.getBus(),"message",m=>{
			let cof=new COF(m);

			if (cof.get(COF.COF_FUNC)==COF.COF_FUNC_SDO_TX &&
					cof.get(COF.COF_SDO_CMD)==COF.COF_SDO_SCS_DOWNLOAD_REPLY &&
					cof.get(COF.COF_SDO_INDEX)==this.index &&
					cof.get(COF.COF_SDO_SUBINDEX)==this.subIndex) {
				return true;
			}

			if (cof.get(COF.COF_FUNC)==COF.COF_FUNC_SDO_TX &&
					cof.get(COF.COF_SDO_CMD)==COF.COF_SDO_SCS_ABORT &&
					cof.get(COF.COF_SDO_INDEX)==this.index &&
					cof.get(COF.COF_SDO_SUBINDEX)==this.subIndex) {
				throw new Error(COF.sdoErrors[cof.get(COF.COF_SDO_ABORT_CODE)]);
			}
        });
	}

	async set(value) {
		//console.log("setting: "+value);

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

		await this.expeditedWrite();
		return this;
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
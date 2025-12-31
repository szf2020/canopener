import RemoteEntry from "./RemoteEntry.js";
import COF from "./cof.js";

export default class RemoteDevice {
	constructor({bus, nodeId}={}) {
		if (!bus || !nodeId)
			throw new Error("Need bus and node id!");

		this.bus=bus;
		this.entries=[];
		this.nodeId=nodeId;

		this.bus.on("message",this.handleMessage);
	}

	handleMessage=message=>{
		let cof=COF.cofFromObject(message);

		console.log("cob id: "+Number(COF._cof_get(cof,COF.COF_COB_ID)).toString(16)+" nodeid: "+COF._cof_get(cof,COF.COF_NODE_ID)+" type: "+COF._cof_get(cof,COF.COF_TYPE));

		if (COF._cof_get(cof,COF.COF_NODE_ID)==this.nodeId &&
				COF._cof_get(cof,COF.COF_TYPE)==COF.COF_TYPE_HEARTBEAT) {
			console.log("heartbeat message...");
		}

        COF._cof_dispose(cof);
	}

	getNodeId() {
		if (!this.nodeId)
			throw new Error("No node id!");

		return this.nodeId;
	}

	entry(index, subIndex) {
		for (let entry of this.entries)
			if (entry.index==index && entry.subIndex==subIndex)
				return entry;

		let entry=new RemoteEntry(index,subIndex);
		entry.device=this;
		this.entries.push(entry);

		return entry;
	}
}
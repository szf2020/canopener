import COF from "./cof.js";

export default class Device {
	constructor({bus, nodeId}={}) {
		if (!bus || !nodeId)
			throw new Error("Need bus and node id!");

		this.bus=bus;
		this.nodeId=nodeId;
		this.sendHeartbeatInterval=setInterval(this.handleSendHeartbeatInterval,1000);
	}

	handleSendHeartbeatInterval=()=>{
		let cof=new COF();

		cof.set(COF.COF_FUNC,COF.COF_FUNC_HEARTBEAT);
		cof.set(COF.COF_NODE_ID,this.getNodeId());
		cof.set(COF.COF_HEARTBEAT_STATE,COF.COF_HB_OPERATIONAL);

		this.bus.send(cof);
	}

	getNodeId() {
		if (!this.nodeId)
			throw new Error("No node id!");

		return this.nodeId;
	}

	getBus() {
		return this.bus;
	}
}
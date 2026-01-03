import RemoteEntry from "./RemoteEntry.js";
import COF from "./cof.js";
import {EventEmitter} from "./js-util.js";

export default class RemoteDevice extends EventEmitter {
	constructor({nodeId}={}) {
		super();

		if (!nodeId)
			throw new Error("Need node id!");

		this.entries=[];
		this.nodeId=nodeId;
		this.state="disconnected";
	}

	setMasterDevice(masterDevice) {
		if (this.getBus())
			this.getBus().off("message",this.handleMessage);

		this.masterDevice=masterDevice;

		if (this.getBus())
			this.getBus().on("message",this.handleMessage);

		this.state="disconnected";
	}

	getBus() {
		if (!this.masterDevice)
			return;

		return this.masterDevice.getBus();
	}

	handleMessage=message=>{
		let cof=new COF(message);
		if (cof.get(COF.COF_FUNC)==COF.COF_FUNC_HEARTBEAT &&
				cof.get(COF.COF_NODE_ID)==this.getNodeId()) {
			let heartbeatState=cof.get(COF.COF_HEARTBEAT_STATE);
			let newState;
			switch (heartbeatState) {
				case COF.COF_HB_OPERATIONAL:
					newState="operational";
					break;

				default:
					newState="unknown";
					break;
			}

			if (newState!=this.state) {
				this.state=newState;
				this.emit("stateChange",this.state);
			}

			if (this.heartbeatTimeout)
				clearTimeout(this.heartbeatTimeout);

			this.heartbeatTimeout=setTimeout(this.handleHeartbeatTimeout,3000);
		}
	}

	handleHeartbeatTimeout=()=>{
		this.heartbeatTimeout=null;
		this.state="disconnected";
		this.emit("stateChange",this.state);
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

	getState() {
		return this.state;
	}
}
import RemoteEntry from "./RemoteEntry.js";
import COF from "./cof.js";
import {EventEmitter, awaitEvent, Semaphore} from "./js-util.js";

export default class RemoteDevice extends EventEmitter {
	constructor({nodeId, bus}={}) {
		super();

		if (bus)
			throw new Error("RemoteDevice should no be created with bus!");

		if (!nodeId)
			throw new Error("Need node id!");

		this.entries=[];
		this.nodeId=nodeId;
		this.state="disconnected";

		this.entry(0x1A00,1);
		this.entry(0x1A01,1);
		this.entry(0x1A02,1);
		this.entry(0x1A03,1);

		this.dirtyEntries=[];
		this.writeSemaphore=new Semaphore();
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

			if (this.heartbeatTimeout) {
				//console.log("calling cleartimeout...");
				clearTimeout(this.heartbeatTimeout);
			}

			this.heartbeatTimeout=setTimeout(this.handleHeartbeatTimeout,3000);
		}

		for (let pdoIndex=0; pdoIndex<4; pdoIndex++) {
			let pdoId=0x180+(pdoIndex*0x100)+this.nodeId;
			if (message.id==pdoId) {
				let pdoEntry=this.entry(0x1A00+pdoIndex,1);
				let bits=pdoEntry.view8[0];
				let subIndex=pdoEntry.view8[1];
				let index=(pdoEntry.view8[2]+(pdoEntry.view8[3]<<8));

				let entry=this.entry(index,subIndex);
				entry.view8[0]=message.data[0];
				entry.view8[1]=message.data[1];
				entry.view8[2]=message.data[2];
				entry.view8[3]=message.data[3];

				//console.log("pdo update ("+index+","+subIndex+") = "+entry.get());
				entry.emit("change");
			}
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

	async awaitState(state) {
		if (this.state==state)
			return;

		await awaitEvent(this,"stateChange",ev=>{
			if (this.state==state)
				return true;
		});
	}

	async processDirty() {
		this.writeSemaphore.critical(async ()=>{
			this.awaitState("operational");
			if (!this.dirtyEntries.length)
				return;

			let entry=this.dirtyEntries.shift();
			this.dirtyEntryInProgress=entry;
			//console.log("writing: "+entry.index+":"+entry.subIndex);
			await entry.expeditedWrite();
			this.dirtyEntryInProgress=undefined;
		});
	}

	notifyDirty(entry) {
		if (!this.dirtyEntries.includes(entry)) {
			this.dirtyEntries.push(entry);
			this.processDirty();
		}
	}

	flush() {
		return new Promise((resolve,reject)=>{
			let waitForFlush=[...this.dirtyEntries];
			if (this.dirtyEntryInProgress)
				waitForFlush.push(this.dirtyEntryInProgress);

			if (!waitForFlush.length) {
				resolve();
				return;
			}

			//console.log("flushing: "+waitForFlush.length);

			function cleanup() {
				for (let w of waitForFlush)
					w.off("written",handleWritten);
			}

			let unwritten=[...waitForFlush];
			function handleWritten(e) {
				let index=unwritten.indexOf(e);
				if (index>=0)
					unwritten.splice(index,1);

				if (!unwritten.length) {
					cleanup();
					//console.log("flush done...");
					resolve();
				}
			}

			for (let w of waitForFlush)
				w.on("written",handleWritten);
		});
	}
}
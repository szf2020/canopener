import Device from "./Device.js";

export default class MasterDevice extends Device {
	constructor({bus}={}) {
		if (!bus)
			throw new Error("MasterDevice: need bus!");

		super({bus, nodeId: 1});
		this.devices=[];
	}

	addDevice(device) {
		device.setMasterDevice(this);
		this.devices.push(device);
	}
}
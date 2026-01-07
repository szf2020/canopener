import Device from "./Device.js";
import RemoteDevice from "./RemoteDevice.js";

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

	createRemoteDevice(nodeId) {
		let remoteDevice=new RemoteDevice({nodeId: nodeId});
		this.addDevice(remoteDevice);

		return remoteDevice;
	}
}
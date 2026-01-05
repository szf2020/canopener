import {openSlcanBus, RemoteDevice, MasterDevice} from "../lib/exports-node.js";

describe("RemoteDevice",()=>{
	it("expedited write without timeout",async ()=>{
		let bus=await openSlcanBus({spawn: "bin/mockdevice", spawnArgs: ["normal"]});
		let masterDevice=new MasterDevice({bus});
		let device=new RemoteDevice({nodeId: 5});
		masterDevice.addDevice(device);
		let e=device.entry(0x4001,0x33);
		e.set(0x12345678);

		await device.flush();

		expect(device.entry(0x4001,0x33).get()).toEqual(0x12345678);

		let e2=device.entry(0x4001,0x34);
		//await expectAsync(e2.set(0x12345678)).toBeRejectedWith(new Error("Object does not exist in OD"));
	});

	it("expedited write with timeout",async ()=>{
		let bus=await openSlcanBus({spawn: "bin/mockdevice", spawnArgs: ["skipfirst"]});
		let masterDevice=new MasterDevice({bus});
		let device=new RemoteDevice({nodeId: 5});
		masterDevice.addDevice(device);
		let e=device.entry(0x4001,0x33);
		e.set(0x12345678);

		await device.flush();

		//expect(device.entry(0x4001,0x33).get()).toEqual(0x12345678);

		//let e2=device.entry(0x4001,0x34);
		//await expectAsync(e2.set(0x12345678)).toBeRejectedWith(new Error("Object does not exist in OD"));
	});

	it("expedited read",async ()=>{
		let bus=await openSlcanBus({spawn: "bin/mockdevice", spawnArgs: ["normal"]});

		let masterDevice=new MasterDevice({bus});
		let device1=new RemoteDevice({nodeId: 5});
		masterDevice.addDevice(device1);

		let e1=device1.entry(0x4001,0x33);
		e1.set(11223344);

		let device2=new RemoteDevice({nodeId: 5});
		masterDevice.addDevice(device2);
		let e2=device2.entry(0x4001,0x33);
		await e2.refresh();
		expect(e2.get()).toEqual(11223344);

		let e3=device2.entry(0x4001,0x34);
		await expectAsync(e3.refresh()).toBeRejectedWith(new Error("Object does not exist in OD"));
	});
});
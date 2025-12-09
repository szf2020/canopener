import {openSlcanBus} from "../lib/SlcanBus.js";
import RemoteDevice from "../lib/RemoteDevice.js";

describe("RemoteDevice",()=>{
	it("expedited write",async ()=>{
		let bus=await openSlcanBus({spawn: "bin/mockdevice"});
		let device=new RemoteDevice({bus: bus, nodeId: 5});
		let e=device.entry(0x4001,0x33);
		await e.set(0x12345678);

		let e2=device.entry(0x4001,0x34);
		await expectAsync(e2.set(0x12345678)).toBeRejectedWith(new Error("Object does not exist in OD"));
	});

	it("expedited read",async ()=>{
		let bus=await openSlcanBus({spawn: "bin/mockdevice"});
		let device=new RemoteDevice({bus: bus, nodeId: 5});

		let e1=device.entry(0x4001,0x33);
		await e1.set(11223344);

		let e2=device.entry(0x4001,0x33);
		await e2.refresh();

		expect(e2.get()).toEqual(11223344);
	});
});
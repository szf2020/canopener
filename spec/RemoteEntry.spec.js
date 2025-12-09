import {openSlcanBus} from "../lib/SlcanBus.js";
import RemoteEntry from "../lib/RemoteEntry.js";

describe("RemoteEntry",()=>{
	it("works nicely with types",async ()=>{
		let e=new RemoteEntry(1,2);
		await e.set(0x12345678);
		expect(e.get()).toEqual(0x12345678);

		e=new RemoteEntry(1,2);
		e.setType("uint8");
		await e.set(0x12345678);
		expect(e.get()).toEqual(0x78);

		e=new RemoteEntry(1,2);
		e.setType("int8");
		await e.set(0xffffffff);
		expect(e.get()).toEqual(-1);
	});
});
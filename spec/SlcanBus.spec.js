import {openSlcanBus} from "../lib/SlcanBus.js";
import {slcanStringify} from "../lib/slcan.js";

describe("SlcanBus",()=>{
	it("can spawn a command",async ()=>{
		let bus=await openSlcanBus({spawn: "bin/mockdevice"});
		await new Promise(resolve=>{
			bus.on("message",m=>{
				let s=slcanStringify(m);
				//console.log(s);
				expect(s).toEqual("t585460014033");
				resolve();
			});

			bus.send("t60582301403378563412"); // expidited SDO write
		});
	});
});
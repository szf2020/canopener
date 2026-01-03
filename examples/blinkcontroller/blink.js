import {openSlcanBus, RemoteDevice, MasterDevice} from "canopener";


let bus=await openSlcanBus({path: "/dev/ttyESP-50:78:7D:8F:D7:E4", baudRate: 115200}); // motor
//let bus=await openSlcanBus({path: "/dev/ttyESP-50:78:7D:8F:D7:D0", baudRate: 115200}); // ui
//let bus=await openSlcanBus({path: "/dev/ttyESP-50:78:7D:91:F1:F0", baudRate: 115200}); // brain

let master=new MasterDevice({bus});
let dev=new RemoteDevice({nodeId: 7});
dev.on("stateChange",()=>{
	console.log("device state: "+dev.getState());
})

master.addDevice(dev);

let blink=dev.entry(0x2000,0).setType("bool");

while (true) {
	//console.log("blinking, state="+dev.getState());

	await blink.set(!blink.get());
	await new Promise(r=>setTimeout(r,1000));
}
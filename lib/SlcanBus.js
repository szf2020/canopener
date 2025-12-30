import {SerialPort} from "serialport";
import {ReadlineParser} from '@serialport/parser-readline'
import EventEmitter from "node:events";
import child_process from "child_process";
import {slcanParse, slcanStringify} from "./slcan.js";

export default class SlcanBus extends EventEmitter {
	constructor(options) {
		super();

        if (options.spawn) {
            this.proc=child_process.spawn("bin/mockdevice");
            this.readStream=this.proc.stdout;
            this.writeStream=this.proc.stdin;
            this.proc.on("spawn",()=>this.emit("open"));
            this.proc.on("error",()=>console.log("Error!!!"));
        }

        else {
            this.port=new SerialPort(options);
            this.readStream=this.port;
            this.writeStream=this.port;
            this.port.on("open",()=>this.emit("open"));
        }

        this.parser=this.readStream.pipe(new ReadlineParser(/*{delimiter: ...}*/));
        this.parser.on("data",this.handleData);
	}

    async close() {
        if (this.port) {
            await this.port.close();
        }
    }

	send(frame) {
        if (typeof frame!="string")
    		frame=slcanStringify(frame);

		if (!frame)
			throw new Error("Not a CAN frame!");

        //console.log("send in bus:",frame);
        this.writeStream.write(frame+"\r\n");
	}

	handleData=data=>{
        try {
            //console.log("Raw: ",data.toString());

            let frame=slcanParse(data.toString());
            //console.log("frame: ",frame);
            if (frame)
                this.emit("message",frame);

            else
                console.log("# ",data.toString());
        }

        catch (e) {
            console.log(e);
        }
	}
}

export function openSlcanBus(options) {
	return new Promise((resolve,reject)=>{
		let bus=new SlcanBus(options);
		bus.on("open",()=>resolve(bus));
		bus.on("error",reject);
	});
}

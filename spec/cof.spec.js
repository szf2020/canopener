import {COF} from "../lib/exports-node.js";

describe("cof",()=>{
	it("can create and stringify a CANopen frame",()=>{
		let cof=COF._cof_create();
		COF._cof_set(cof,COF.COF_TYPE,COF.COF_TYPE_SDO_ABORT);
		COF._cof_set(cof,COF.COF_NODE_ID,0x05);
	    COF._cof_set(cof,COF.COF_SDO_INDEX,0x4001);
	    COF._cof_set(cof,COF.COF_SDO_SUB,0x10);
		COF._cof_set(cof,COF.COF_SDO_ABORT_CODE,COF.COF_ABORT_NOT_EXIST);

		let s=COF.toSlcan(cof);
		//console.log("here: ",s);
		expect(s).toEqual("t58588001401000000206");

		COF._cof_dispose(cof);
	});

	it("it can parse a CANopen frame",()=>{
		let cof=COF.fromSlcan("t58588001401000000206");

		//console.log(COF.slcanStringify(cof));
		expect(COF.toSlcan(cof)).toEqual("t58588001401000000206");
		expect(COF._cof_get(cof,COF.COF_TYPE)).toEqual(COF.COF_TYPE_SDO_ABORT);

		COF._cof_dispose(cof);
	});
});
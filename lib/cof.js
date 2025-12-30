import defines from "../dist/cof-defines.js";
import {slcanParse, slcanStringify} from "./slcan.js";

let COF={};

COF.decorate=(o)=>{
	Object.assign(COF,o);
}

COF.cofToObject=cof=>{
	let slcan=COF.toSlcan(cof);
	return slcanParse(slcan);
}

COF.cofFromObject=o=>{
	let slcan=slcanStringify(o);
	return COF.fromSlcan(slcan);
}

COF.toSlcan=cof=>{
	let p=COF._malloc(256);
	COF._cof_to_slcan(cof,p);

	let res="";
	for (let i=p; COF.HEAPU8[i]; i++)
		res+=String.fromCharCode(COF.HEAPU8[i]);

	COF._free(p);
	return res;
}

COF.fromSlcan=slcan=>{
	let s=COF._malloc(256);
	//console.log("s="+s);
	for (let i=0; i<slcan.length; i++)
		COF.HEAPU8[s+i]=slcan[i].charCodeAt(0);

	COF.HEAPU8[s+slcan.length]=0;

	let cof=COF._cof_create();
	let res=COF._cof_from_slcan(cof,s);
	//console.log("res= ",res);
	COF._free(s);

	return cof;
}

COF={...COF,...defines};

COF.sdoErrors={}
COF.sdoErrors[COF.COF_ABORT_TOGGLE]="Toggle bit not alternated";
COF.sdoErrors[COF.COF_ABORT_TIMEOUT]="SDO protocol timed out";
COF.sdoErrors[COF.COF_ABORT_COMMAND]="Command specifier not valid or unknown";
COF.sdoErrors[COF.COF_ABORT_OUT_OF_MEMORY]="Out of memory";
COF.sdoErrors[COF.COF_ABORT_UNSUPPORTED_ACCESS]="Unsupported access to an object";
COF.sdoErrors[COF.COF_ABORT_NOT_EXIST]="Object does not exist in OD";
COF.sdoErrors[COF.COF_ABORT_READONLY]="Attempt to write a read-only object";
COF.sdoErrors[COF.COF_ABORT_TYPE_MISMATCH]="Data type does not match";
COF.sdoErrors[COF.COF_ABORT_LENGTH_EXCEEDED]="Length of service parameter too high";
COF.sdoErrors[COF.COF_ABORT_SUBINDEX]="Subindex does not exist";
COF.sdoErrors[COF.COF_ABORT_DEVICE_STATE]="Device internal error";

export default COF;

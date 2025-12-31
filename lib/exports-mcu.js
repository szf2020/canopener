/*console.log("this is the mcu export...");
console.log("_cof_create",global._cof_create);*/

import cofModule from "./cof.js";
cofModule.decorate({
	_malloc: global._malloc,
	_free: global._free,
	_cof_create: global._cof_create,
	_cof_dispose: global._cof_dispose,
	_cof_get: global._cof_get,
	_cof_getp: global._cof_getp,
	_cof_set: global._cof_set,
	_cof_from_slcan: global._cof_from_slcan,
	_cof_to_slcan: global._cof_to_slcan,
	_peek: global._peek,
	_poke: global._poke
});

export let COF=cofModule;
global.COF=COF;

export {default as RemoteDevice} from "./RemoteDevice.js";
export {slcanParse, slcanStringify} from "./slcan.js";

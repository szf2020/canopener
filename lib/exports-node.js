import cofWasmCtor from "../dist/cof-wasm.js";
import cofModule from "./cof.js";
cofModule.decorate(await cofWasmCtor());
export let COF=cofModule;

export {openSlcanBus} from "./SlcanBus.js";
export {default as RemoteDevice} from "./RemoteDevice.js";

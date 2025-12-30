export function awaitEvent(target, type, processor) {
	return new Promise((resolve, reject)=>{
		function handle(...args) {
			try {
				let v=processor(...args);
				if (v) {
					target.off(type,handle);
					resolve(v);
				}
			}

			catch (e) {
				target.off(type,handle);
				reject(e);
			}
		}

		//console.log("adding: "+options.type);
		target.on(type,handle);
	});
}

export class EventEmitter {
    constructor() {
        this._e = {}
    }

    on(t, f) {
        (this._e[t] || (this._e[t] = [])).push(f)
    }

    emit(t, a) {
        const e = this._e[t]
        if (!e) return
        for (let i = 0; i < e.length; i++)
            e[i](a)
    }
}

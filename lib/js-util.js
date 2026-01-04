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
        this._e = {};
    }

    on(type, fn) {
        if (!this._e[type]) {
            this._e[type] = [];
        }
        this._e[type].push(fn);
    }

    off(type, fn) {
        const list = this._e[type];
        if (!list) return;

        const i = list.indexOf(fn);
        if (i !== -1) {
            list.splice(i, 1);
        }

        // Optional cleanup
        if (list.length === 0) {
            delete this._e[type];
        }
    }

    emit(type, arg) {
        const list = this._e[type];
        if (!list) return;

        // copy in case listeners mutate the array
        for (const fn of [...list]) {
            fn(arg);
        }
    }
}

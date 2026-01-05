export function awaitEvent(target, type, processor, timeout) {
	return new Promise((resolve, reject)=>{
        let timeoutId;

        function cleanup() {
            //console.log("timeoutid: "+timeoutId);
            clearTimeout(timeoutId);
            target.off(type,handle);
        }

		function handle(...args) {
			try {
				let v=processor(...args);
				if (v) {
                    cleanup();
					resolve(v);
				}
			}

			catch (e) {
                cleanup();
				reject(e);
			}
		}

		//console.log("adding: "+options.type);
		target.on(type,handle);
        if (timeout) {
            timeoutId=setTimeout(()=>{
                cleanup();
                resolve();
            },timeout);
        }
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

export class Semaphore {
    constructor() {
        this._locked = false;
        this._queue = [];
    }

    async critical(fn) {
        if (this._locked) {
            await new Promise(resolve => this._queue.push(resolve));
        }

        this._locked = true;

        try {
            return await fn();
        }

        finally {
            this._locked = false;
            const next = this._queue.shift();
            if (next) next();
        }
    }
}
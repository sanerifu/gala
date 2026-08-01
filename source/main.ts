export type Brand<K> = K & { readonly __brand: unique symbol };

type VboId = Brand<number>;
type EboId = Brand<number>;

export class Gala {
    readonly canvas: HTMLCanvasElement;
    readonly gl: WebGL2RenderingContext;

    readonly vertices: { readonly buffer: WebGLBuffer[], readonly length: number[], readonly free: VboId[] } = {
        buffer: [],
        length: [],
        free: [],
    };
    readonly indices: { readonly buffer: WebGLBuffer[], readonly length: number[], readonly free: EboId[] } = {
        buffer: [],
        length: [],
        free: [],
    }

    constructor(canvas: HTMLCanvasElement) {
        this.canvas = canvas;
        const gl = this.canvas.getContext("webgl2");
        if (gl === null) {
            throw Error("Cannot initialize WebGL 2 context");
        }
        this.gl = gl;
    }

    clear() {
        const gl = this.gl;
        gl.clearColor(1.0, 0.0, 1.0, 1.0);
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    }

    createVbo(data: Uint8Array): VboId {
        const gl = this.gl;
        const buffer = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
        gl.bufferData(gl.ARRAY_BUFFER, data, gl.STATIC_DRAW);

        const id = this.vertices.free.pop();
        if (id === undefined) {
            const id = this.vertices.buffer.length;
            this.vertices.buffer.push(buffer);
            this.vertices.length.push(data.length);
            return id as VboId;
        } else {
            this.vertices.buffer[id] = buffer;
            this.vertices.length[id] = data.length;
        }
        return id;
    }

    removeVbo(id: VboId) {
        const gl = this.gl;
        gl.deleteBuffer(this.vertices.buffer[id] || null);
        this.vertices.free.push(id);
    }

    createEbo(data: Uint8Array): EboId {
        const gl = this.gl;
        const buffer = gl.createBuffer();
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, buffer);
        gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, data, gl.STATIC_DRAW);

        const id = this.indices.free.pop();
        if (id === undefined) {
            const id = this.indices.buffer.length;
            this.indices.buffer.push(buffer);
            this.indices.length.push(data.length);
            return id as EboId;
        } else {
            this.indices.buffer[id] = buffer;
            this.indices.length[id] = data.length;
        }
        return id;
    }

    removeEbo(id: EboId) {
        const gl = this.gl;
        gl.deleteBuffer(this.indices.buffer[id] || null);
        this.indices.free.push(id);
    }
};



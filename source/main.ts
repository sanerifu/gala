export class Gala {
    canvas: HTMLCanvasElement
    gl: WebGL2RenderingContext

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
};



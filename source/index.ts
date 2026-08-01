import { Gala } from "./main.js";

const canvas = document.querySelector("#screen") as HTMLCanvasElement;
const engine = new Gala(canvas);

engine.clear();
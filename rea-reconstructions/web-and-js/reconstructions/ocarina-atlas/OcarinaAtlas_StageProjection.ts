// Reconstructed from Ocarina-Atlas REA static trace (src/lib/coords.ts & src/lib/floorGrid.ts)
// Normalized image-fraction coordinate system for OoT dungeon/overworld multi-floor maps

export interface Fraction {
  x: number; // [0..1] left -> right
  y: number; // [0..1] top -> bottom
}

export interface Size {
  width: number;
  height: number;
}

export interface Box extends Size {
  left: number;
  top: number;
}

export interface View {
  cx: number;
  cy: number;
  zoom: number;
}

export const INITIAL_VIEW: View = { cx: 0.5, cy: 0.5, zoom: 1 };
export const MIN_ZOOM = 1;
export const MAX_ZOOM = 10;

export function fractionToBox(fraction: Fraction, box: Box): { x: number; y: number } {
  return {
    x: box.left + fraction.x * box.width,
    y: box.top + fraction.y * box.height,
  };
}

export function boxToFraction(point: { x: number; y: number }, box: Box): Fraction {
  return {
    x: (point.x - box.left) / box.width,
    y: (point.y - box.top) / box.height,
  };
}

export function fitSize(container: Size, aspect: number): Size {
  const width = Math.min(container.width, container.height * aspect);
  return { width, height: width / aspect };
}

export function stageBox(view: View, container: Size, aspect: number): Box {
  const fit = fitSize(container, aspect);
  const width = fit.width * view.zoom;
  const height = fit.height * view.zoom;
  return {
    width,
    height,
    left: container.width / 2 - view.cx * width,
    top: container.height / 2 - view.cy * height,
  };
}

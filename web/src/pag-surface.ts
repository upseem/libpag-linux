import { PAGModule } from './pag-module';
import { AlphaType, ColorType } from './types';
import { readBufferFromWasm } from './utils/buffer';
import { destroyVerify, wasmAwaitRewind } from './utils/decorators';

@destroyVerify
@wasmAwaitRewind
export class PAGSurface {
  /**
   * Make a PAGSurface from canvas.
   */
  public static fromCanvas(canvasID: string): PAGSurface {
    const wasmIns = PAGModule._PAGSurface._FromCanvas(canvasID);
    if (!wasmIns) throw new Error(`Make PAGSurface from canvas ${canvasID} fail!`);
    return new PAGSurface(wasmIns);
  }
  /**
   * Make a PAGSurface from texture.
   */
  public static fromTexture(textureID: number, width: number, height: number, flipY: boolean): PAGSurface {
    const wasmIns = PAGModule._PAGSurface._FromTexture(textureID, width, height, flipY);
    if (!wasmIns) throw new Error(`Make PAGSurface from texture ${textureID} fail!`);
    return new PAGSurface(wasmIns);
  }
  /**
   * Make a PAGSurface from frameBuffer.
   */
  public static fromRenderTarget(frameBufferID: number, width: number, height: number, flipY: boolean): PAGSurface {
    const wasmIns = PAGModule._PAGSurface._FromRenderTarget(frameBufferID, width, height, flipY);
    if (!wasmIns) throw new Error(`Make PAGSurface from frameBuffer ${frameBufferID} fail!`);
    return new PAGSurface(wasmIns);
  }

  public wasmIns;
  public isDestroyed = false;

  public constructor(wasmIns: any) {
    this.wasmIns = wasmIns;
  }
  /**
   * The width of surface in pixels.
   */
  public width(): number {
    return this.wasmIns._width() as number;
  }
  /**
   * The height of surface in pixels.
   */
  public height(): number {
    return this.wasmIns._height() as number;
  }
  /**
   * Update the size of surface, and reset the internal surface.
   */
  public updateSize(): void {
    this.wasmIns._updateSize();
  }
  /**
   * Erases all pixels of this surface with transparent color. Returns true if the content has
   * changed.
   */
  public clearAll(): boolean {
    return this.wasmIns._clearAll() as boolean;
  }
  /**
   * Free the cache created by the surface immediately. Can be called to reduce memory pressure.
   */
  public freeCache(): void {
    this.wasmIns._freeCache();
  }
  /**
   * Copies pixels from current PAGSurface to dstPixels with specified color type, alpha type and
   * row bytes. Returns true if pixels are copied to dstPixels.
   */
  public readPixels(colorType: ColorType, alphaType: AlphaType): Uint8Array | null {
    if (colorType === ColorType.Unknown) return null;
    const rowBytes = this.width() * (colorType === ColorType.ALPHA_8 ? 1 : 4);
    const length = rowBytes * this.height();
    const dataUint8Array = new Uint8Array(length);
    const { data, free } = readBufferFromWasm(PAGModule, dataUint8Array, (dataPtr) => {
      return this.wasmIns._readPixels(colorType, alphaType, dataPtr, rowBytes) as boolean;
    });
    free();
    return data;
  }

  public destroy(): void {
    this.wasmIns.delete();
    this.isDestroyed = true;
  }
}

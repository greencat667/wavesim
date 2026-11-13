import {Device} from './device.ts';
import assert from 'node:assert/strict';
const d=new Device();
try {
 await d.command('set',{temperature:23.4,humidity:71,epoch:1907332500,rate:0});
 const a=await d.refresh();await d.temperature.set(31.2);const b=await d.refresh();
 assert.notEqual(d.hash(a.frame),d.hash(b.frame));
 d.expectEvent(b,'SERIAL','temperature=31.2');d.expectEvent(b,'DISPLAY','FULL_REFRESH');
 d.expectPixel(b.frame,20,116,true);
 console.log('PASS TypeScript firmware → I2C → framebuffer integration');
} finally {d.close();}

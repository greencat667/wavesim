// Run with Node 22.18+ (native TypeScript) or tsx. No test framework dependency.
import {spawn} from 'node:child_process';
import {createInterface} from 'node:readline';
import {resolve} from 'node:path';
import {createHash} from 'node:crypto';
export class Device {
 process; pending: Array<{resolve:Function,reject:Function}>=[];
 constructor(root=process.cwd(),example='environment') {
  this.process=spawn(resolve(root,'build/wavesim-engine'),[resolve(root,`build/${example}.dylib`)]);
  createInterface({input:this.process.stdout}).on('line',line=>{const r=JSON.parse(line),p=this.pending.shift();if(p)r.ok?p.resolve(r.state):p.reject(new Error(r.error));});
  this.process.on('exit',code=>{for(const p of this.pending)p.reject(new Error(`engine exited ${code}`));this.pending=[];});
 }
 command(op:string,args:Record<string,unknown>={}):Promise<any>{return new Promise((resolve,reject)=>{this.pending.push({resolve,reject});this.process.stdin.write(JSON.stringify({op,...args})+'\n');});}
 temperature={set:(value:number)=>this.command('set',{temperature:value})};
 humidity={set:(value:number)=>this.command('set',{humidity:value})};
 rtc={advance:(seconds:number)=>this.command('jump',{seconds})};
 async refresh(){return this.command('run');}
 hash(frame:string){return createHash('sha256').update(Buffer.from(frame,'hex')).digest('hex');}
 expectPixel(frame:string,x:number,y:number,black:boolean){const bytes=Buffer.from(frame,'hex');if(x<0||x>=200||y<0||y>=200)throw Error('Pixel outside display');if((!(bytes[y*25+(x>>3)]&(128>>(x%8))))!==black)throw Error(`Pixel ${x},${y} mismatch`);}
 expectEvent(state:any,bus:string,fragment:string){if(!state.events.some((e:any)=>e.bus===bus&&e.message.includes(fragment)))throw Error(`Missing ${bus} ${fragment}`);}
 close(){this.process.stdin.end();}
}

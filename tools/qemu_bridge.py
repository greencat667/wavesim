#!/usr/bin/env python3
"""JSON bench protocol → same C++ board engine ← UART HAL ← Xtensa firmware."""
import pathlib,subprocess,json,sys,selectors,time,signal
signal.signal(signal.SIGTERM,lambda *_:sys.exit(0))
ROOT=pathlib.Path(__file__).resolve().parents[1]
class Bridge:
 def __init__(self):
  build=ROOT/'examples/qemu-environment/build';args=json.loads((build/'flasher_args.json').read_text());image=bytearray(b'\xff'*8388608)
  for offset,file in args['flash_files'].items():
   d=(build/file).read_bytes();o=int(offset,16);image[o:o+len(d)]=d
  import tempfile
  self.tmp=tempfile.TemporaryDirectory(prefix='wavesim-qemu-');path=pathlib.Path(self.tmp.name)/'flash.bin';path.write_bytes(image)
  self.engine=subprocess.Popen([str(ROOT/'build/wavesim-engine')],stdin=subprocess.PIPE,stdout=subprocess.PIPE,text=True)
  self.qemu=subprocess.Popen([str(ROOT/'third_party/qemu/bin/qemu-system-xtensa'),'-nographic','-monitor','none','-machine','esp32s3','-drive',f'file={path},if=mtd,format=raw'],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=sys.stderr)
  self.buf=b'';self.sel=selectors.DefaultSelector();self.sel.register(self.qemu.stdout,selectors.EVENT_READ)
  boot_deadline=time.monotonic()+15
  while True:
   if time.monotonic()>boot_deadline:raise TimeoutError('QEMU did not reach app_main READY')
   line=self.read()
   if line=='@READY':
    self.qemu.send_signal(signal.SIGSTOP)
    break
   self.cmd({'op':'io','kind':'log','text':line})
 def read(self):
  import os
  deadline=time.monotonic()+15
  while b'\n' not in self.buf:
   if time.monotonic()>deadline:raise TimeoutError('QEMU UART timed out')
   if self.sel.select(.25):
    d=os.read(self.qemu.stdout.fileno(),65536)
    if not d:raise RuntimeError('QEMU exited')
    self.buf+=d
  line,self.buf=self.buf.split(b'\n',1);return line.decode(errors='replace').strip()
 def reply(self,s):self.qemu.stdin.write((str(s)+'\n').encode());self.qemu.stdin.flush()
 def cmd(self,j):
  self.engine.stdin.write(json.dumps(j)+'\n');self.engine.stdin.flush();r=json.loads(self.engine.stdout.readline())
  r['state']['status']['CPU']='EMULATED — ESP32-S3 Xtensa / QEMU'
  r['state']['status']['Peripheral transport']='BRIDGED — UART HAL, not MMIO'
  r['state']['status']['Network']='UNSUPPORTED in QEMU HAL'
  r['state']['status']['SD']='UNSUPPORTED in QEMU HAL'
  r['state']['project']='ESP32-S3 binary: environment (UART HAL)'
  return r
 def tick(self):
  s=self.cmd({'op':'state'})['state']
  if not s['powered'] or s['sleeping']:return
  self.qemu.send_signal(signal.SIGCONT)
  self.reply('T')
  deadline=time.monotonic()+15
  while True:
   if time.monotonic()>deadline:raise TimeoutError('Firmware tick did not finish')
   line=self.read()
   if line=='@DONE':return
   if 'panic' in line.lower() or line=='@READY':raise RuntimeError('QEMU firmware restarted during tick: '+line)
   if line.startswith('@I '):
    _,address,nr,*tx=line.split();r=self.cmd({'op':'io','kind':'i2c','address':int(address),'nr':int(nr),'tx':list(bytes.fromhex(tx[0] if tx else ''))})['state']['rpc'];self.reply(str(r['rc'])+' '+bytes(r['rx']).hex())
   elif line.startswith('@D '):
    _,partial,frame=line.split();r=self.cmd({'op':'io','kind':'display','partial':int(partial),'frame':frame});self.reply(r['state']['rpc']['rc'])
   elif line.startswith('@G '):self.reply(self.cmd({'op':'io','kind':'gpio_read','pin':int(line[3:])})['state']['rpc']['rc'])
   elif line.startswith('@W '):
    _,p,v=line.split();self.reply(self.cmd({'op':'io','kind':'gpio_write','pin':int(p),'value':int(v)})['state']['rpc']['rc'])
   else:self.cmd({'op':'io','kind':'log','text':line.removeprefix('@L ')})
 def handle(self,j):
  if j.get('op')=='replay':
   self.cmd({'op':'record','on':False})
   for step in j['steps']:self.handle(step)
   return self.cmd({'op':'state'})
  op=j.get('op');run=op in ['run','jump','button','reset'] or op=='advance' and j.get('run',True) or op=='power' and j.get('on') or op=='sleep' and not j.get('on',True)
  if op=='load':j={'op':'state'} # app bootstrap; binary is fixed to shared environment source
  if op=='run':j={'op':'state'}
  if op=='advance':j={**j,'run':False}
  r=self.cmd(j)
  if r['ok'] and run:
   try:self.tick()
   finally:self.qemu.send_signal(signal.SIGSTOP)
   r=self.cmd({'op':'state'})
  return r
 def close(self):
  self.qemu.send_signal(signal.SIGCONT);self.qemu.terminate();self.qemu.wait(timeout=5);self.engine.stdin.close();self.engine.wait(timeout=5);self.qemu.stdin.close();self.qemu.stdout.close();self.engine.stdout.close();self.sel.close();self.tmp.cleanup()
if __name__=='__main__':
 bridge=None
 try:
  bridge=Bridge()
  for line in sys.stdin:
   try:print(json.dumps(bridge.handle(json.loads(line))),flush=True)
   except Exception as e:print(json.dumps({'ok':False,'error':str(e)}),flush=True)
 finally:
  if bridge:bridge.close()

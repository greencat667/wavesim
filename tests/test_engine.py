import unittest,subprocess,json,pathlib,datetime,tempfile,threading,http.server
ROOT=pathlib.Path(__file__).resolve().parents[1]
class Device:
 def __init__(self,example='environment'):
  self.p=subprocess.Popen([str(ROOT/'build/wavesim-engine'),str(ROOT/f'build/{example}.dylib')],stdin=subprocess.PIPE,stdout=subprocess.PIPE,text=True)
 def cmd(self,op,**kw):
  self.p.stdin.write(json.dumps(dict(op=op,**kw))+'\n');self.p.stdin.flush();r=json.loads(self.p.stdout.readline())
  if not r['ok']:raise RuntimeError(r['error'])
  return r['state']
 def close(self):self.p.stdin.close();self.p.wait(timeout=5);self.p.stdout.close()
def epoch(s):return datetime.datetime.fromisoformat(s).replace(tzinfo=datetime.timezone.utc).timestamp()
class Acceptance(unittest.TestCase):
 def setUp(self):self.d=Device()
 def tearDown(self):self.d.close()
 def test_01_environment(self):
  self.d.cmd('set',temperature=23.4,humidity=71,epoch=epoch('2030-06-10T14:35'))
  a=self.d.cmd('run');self.d.cmd('set',temperature=31.2);b=self.d.cmd('run')
  self.assertEqual(len(bytes.fromhex(a['frame'])),5000);self.assertNotEqual(a['frame'],b['frame'])
  self.assertTrue(any('temperature=23.4 humidity=71' in e['message'] for e in a['events']))
  self.assertTrue(any('temperature=31.2 humidity=71' in e['message'] for e in b['events']))
  # Independent raster oracle for displayed 14:35 and 31.2 glyph pixels.
  glyph={'1':[0,66,127,64,0],'4':[24,20,18,127,16],':':[0,54,54,0,0],'3':[33,65,69,75,49],'5':[39,69,69,69,57],'.':[0,96,96,0,0],'2':[66,97,81,73,70]}
  def verify(frame,s,x,y,scale):
   f=bytes.fromhex(frame)
   for i,c in enumerate(s):
    for col in range(5):
     for row in range(7):
      xx=x+i*6*scale+col*scale;yy=y+row*scale
      self.assertEqual(not bool(f[yy*25+xx//8]&(128>>(xx%8))),bool(glyph[c][col]&(1<<row)))
  verify(a['frame'],'14:35',24,43,5);verify(b['frame'],'31.2',20,116,4)
  (ROOT/'tests/artifacts').mkdir(exist_ok=True)
  for name,st in [('before',a),('after',b)]:
   (ROOT/f'tests/artifacts/{name}.pbm').write_bytes(b'P4\n200 200\n'+bytes(x^255 for x in bytes.fromhex(st['frame'])))
 def test_02_rollover(self):
  self.d.cmd('set',epoch=epoch('2030-12-31T23:59:50'),rate=60)
  s=self.d.cmd('advance',seconds=1);self.assertEqual(s['epoch'],epoch('2031-01-01T00:00:50'))
 def test_03_network_recovery(self):
  class Handler(http.server.BaseHTTPRequestHandler):
   def do_GET(self):self.send_response(200);self.end_headers();self.wfile.write(b'WaveSim local test')
   def log_message(self,*args):pass
  server=http.server.HTTPServer(('127.0.0.1',8765),Handler);threading.Thread(target=server.serve_forever,daemon=True).start()
  try:
   self.d.cmd('load',path=str(ROOT/'build/network.dylib'));self.d.cmd('set',network='OFFLINE');a=self.d.cmd('run');self.d.cmd('set',network='ONLINE');b=self.d.cmd('run')
   self.assertNotEqual(a['frame'],b['frame']);self.assertTrue(any(e['message']=='HTTP 200' for e in b['events']))
  finally:server.shutdown();server.server_close()
 def test_04_power_retention(self):
  a=self.d.cmd('run');self.d.cmd('power',on=False);self.d.cmd('set',temperature=45);b=self.d.cmd('run');self.assertEqual(a['frame'],b['frame']);c=self.d.cmd('power',on=True);self.assertNotEqual(c['frame'],a['frame'])
 def test_05_fault(self):
  a=self.d.cmd('run');self.d.cmd('set',fault='sensor_missing');b=self.d.cmd('run');self.assertNotEqual(a['frame'],b['frame']);self.assertTrue(any('ERROR SHTC3' in e['message'] for e in b['events']))
 def test_06_latency(self):
  self.d.cmd('set',realistic=True);a=self.d.cmd('run');self.assertEqual(a['frames'],0);self.assertTrue(a['busy'])
  self.d.cmd('set',temperature=32);queued=self.d.cmd('run');self.assertTrue(queued['busy']);self.assertTrue(any(e['message']=='REFRESH coalesced while BUSY' for e in queued['events']))
  b=self.d.cmd('advance',seconds=2,run=False);self.assertEqual(b['frames'],1);self.assertFalse(b['busy'])
  self.assertNotEqual(a['frame'],b['frame'])
 def test_07_buttons(self):
  a=self.d.cmd('run');b=self.d.cmd('button',down=True);self.assertEqual(b['gpio'][0],0);self.assertNotEqual(a['frame'],b['frame']);self.d.cmd('button',down=False)
 def test_08_replay(self):
  self.d.cmd('record',on=True);self.d.cmd('set',temperature=9);a=self.d.cmd('run');rec=self.d.cmd('record',on=False)['record'];self.d.cmd('set',temperature=50);b=self.d.cmd('replay',steps=rec);self.assertEqual(a['frame'],b['frame'])
 def test_09_sd(self):
  with tempfile.TemporaryDirectory() as tmp:
   self.d.cmd('set',sd=tmp);self.d.cmd('sd_write',path='test.txt',text='OK');self.assertEqual(pathlib.Path(tmp,'test.txt').read_text(),'OK')
   with self.assertRaises(RuntimeError):self.d.cmd('sd_write',path='../escape',text='no')
   self.d.cmd('set',readonly=True)
   with self.assertRaises(RuntimeError):self.d.cmd('sd_write',path='test.txt',text='no')
 def test_10_sleep(self):
  a=self.d.cmd('run');self.d.cmd('sleep');self.d.cmd('set',temperature=-10);b=self.d.cmd('run');self.assertEqual(a['frame'],b['frame']);self.assertNotEqual(a['frame'],self.d.cmd('sleep',on=False)['frame'])
if __name__=='__main__':unittest.main(verbosity=2)

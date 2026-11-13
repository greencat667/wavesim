import pathlib,sys,hashlib,json
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT/'tools'));sys.path.insert(0,str(ROOT/'tests'))
from qemu_bridge import Bridge
from test_engine import Device,epoch
import unittest
class QemuAcceptance(unittest.TestCase):
 def test_xtensa_matches_native(self):
  b=Bridge();d=Device()
  try:
   inputs={'op':'set','temperature':23.4,'humidity':71,'epoch':epoch('2030-06-10T14:35'),'rate':0}
   b.handle(inputs);d.cmd('set',**{k:v for k,v in inputs.items() if k!='op'})
   a=b.handle({'op':'run'})['state'];native=d.cmd('run');assert a['frame']==native['frame'],'Xtensa and ARM64 raster mismatch'
   b.handle({'op':'set','temperature':31.2});after=b.handle({'op':'run'})['state'];assert after['frame']!=a['frame'];assert any('temperature=31.2' in e['message'] for e in after['events'])
   result={'backend':after['status']['CPU'],'before_sha256':hashlib.sha256(bytes.fromhex(a['frame'])).hexdigest(),'after_sha256':hashlib.sha256(bytes.fromhex(after['frame'])).hexdigest(),'native_qemu_equal':True}
   (ROOT/'tests/artifacts/qemu-results.json').write_text(json.dumps(result,indent=2));print('PASS QEMU Xtensa environment: native-identical framebuffer and live sensor changes');print(json.dumps(result,indent=2))
  finally:b.close();d.close()

if __name__=="__main__":unittest.main(verbosity=2)

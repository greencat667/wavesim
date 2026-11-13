#!/usr/bin/env python3
import pathlib,subprocess,sys
from build_arduino import build as build_arduino
ROOT=pathlib.Path(__file__).resolve().parents[1]
def build(project,output):
 p=pathlib.Path(project).resolve()
 if p.suffix=='.ino' or p.is_dir() and (list(p.glob('*.ino')) or list(p.glob('*/*.ino'))):
  build_arduino(p,output);return 'Arduino source simulation (hardware adapters; HTTP fixtures only)'
 src=p/'main/app.c' if p.is_dir() else p
 if src.suffix!='.c' or not src.is_file():raise ValueError('Choose an Arduino .ino/sketch folder, or a WaveSim HAL project containing main/app.c.')
 subprocess.run(['/usr/bin/clang','-dynamiclib','-g','-O0','-I'+str(ROOT/'core'),str(src),'-o',str(output)],check=True)
 return 'native C HAL firmware'
if __name__=='__main__':
 try:print('Build succeeded — '+build(sys.argv[1],sys.argv[2]))
 except (ValueError,RuntimeError,subprocess.CalledProcessError) as e:print(str(e),file=sys.stderr);sys.exit(1)

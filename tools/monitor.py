import serial,sys
with serial.Serial(sys.argv[1],115200,timeout=1) as port:
 print('Monitoring '+port.port,flush=True)
 while True:
  data=port.read(4096)
  if data:print(data.decode(errors='replace'),end='',flush=True)

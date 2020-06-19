#!/usr/bin/env python3

def main(path, *remaining_cmdline):
  path = os.path.join(path, 'media/tools/codec_inspector/')
  os.system(f'mkdir {path}')
  os.system(f'cp ./* {path}');
  print(f'installed to {path}')

if __name__ == '__main__':
  main(*sys.argv[1:])
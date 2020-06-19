#!/usr/bin/env python3

import os
import subprocess


LICENSE = '0fca02217a5d49a14dfe2d11837bb34d  LICENSE'
LICENSE_CROS = '87dd8458232da630f5617873d42d8350  LICENSE.chromium_os'


def md5(file):
  return subprocess.check_output(['md5sum', file]).decode('utf-8').strip()


def check_current_directory():
  for sum_filen in [LICENSE, LICENSE_CROS]:
    filen = sum_filen.split('  ')[1]
    if not (os.path.isfile(filen) and md5(filen) == sum_filen):
      print(f'{filen} does not exists, or mismatch between:')
      print(md5(filen))
      print(sum_filen)
      return False
  return True


class ModBuild():
  def __enter__(self):
    os.system('cp BUILD.gn BUILD.gn_original')
    os.system('cat media/tools/codec_inspector/BUILD.extra >> BUILD.gn')

  def __exit__(self, *args, **kwargs):
    os.system('mv BUILD.gn_original BUILD.gn')


class FFMpegConfig():
  def __enter__(self):
    src = 'third_party/ffmpeg/chromium/config/Chromium/linux/x64/config.h'
    dst = 'third_party/ffmpeg/libavformat/'
    os.system(f'cp {src} {dst}')

  def __exit__(self, *args, **kwargs):
    os.system('rm third_party/ffmpeg/libavformat/config.h')


class MediaGPUMod():
  def __enter__(self):
    os.system('cp media/gpu/BUILD.gn media/gpu/BUILD.gn_original')
    os.system('head -n 40 media/gpu/BUILD.gn >> construct.gn')
    nl = '    "//media/tools/codec_inspector:codec_inspector_service",'
    os.system(f'''echo '{nl}' >> construct.gn''')
    os.system('tail -n +41 media/gpu/BUILD.gn >> construct.gn')
    os.system('mv construct.gn media/gpu/BUILD.gn')

  def __exit__(self, *args, **kwargs):
    os.system('mv media/gpu/BUILD.gn_original media/gpu/BUILD.gn')


def gen_out_dir_if_not_exist():
  if not os.path.exists('out'):
    os.system('mkdir out')
  if not os.path.isdir('out/codec_inspector'):
    os.system('gn gen out/codec_inspector')


def build():
  os.system('ninja -C out/codec_inspector codec_inspector_service -j 4')


def main():
  if not check_current_directory():
    print('you must be in the chromium source tree')
    return
  with ModBuild():
    with FFMpegConfig():
      with MediaGPUMod():
        gen_out_dir_if_not_exist()
        build()




if __name__ == '__main__':
  main()
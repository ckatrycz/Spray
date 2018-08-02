import os
import pwd
import sys

def execute_command(line):
  print(line)
  return os.system(line)
  
if __name__ == '__main__':
  if len(sys.argv) > 1:
    build_type = sys.argv[1]
    print('Build type: ', build_type)
  else:
    build_type = 'default'
    
  print('Build type = {}'.format(build_type))

  assert build_type in ['default', 'ci']

  if build_type == 'ci':
    os.environ['TC_CI'] = '1'
    username = 'travis'
  else:
    username = pwd.getpwuid(os.getuid())[0]

  try:
    import pip
  except Exception as e:
    print(e)
    execute_command('wget https://bootstrap.pypa.io/get-pip.py')
    execute_command('python3 get-pip.py --user')
    execute_command('rm get-pip.py')
  execute_command('sudo apt-get update')
  execute_command('sudo apt-get install -y python3-dev git build-essential cmake make g++ python3-tk ffmpeg')
  execute_command('sudo python3 -m pip install colorama')
  os.chdir('/home/{}/'.format(username))
  execute_command('mkdir -p repos')
  os.chdir('repos')
  os.chdir('taichi')

  taichi_root_dir = "/home/{}/repos/".format(username)
  execute_command('echo "export TAICHI_NUM_THREADS=8" >> ~/.bashrc')
  execute_command('echo "export TAICHI_ROOT_DIR={}" >> ~/.bashrc'.format(taichi_root_dir))
  execute_command('echo "export PYTHONPATH=\$TAICHI_ROOT_DIR/taichi/python/:\$PYTHONPATH" >> ~/.bashrc')
  execute_command('echo "export PATH=\$TAICHI_ROOT_DIR/taichi/bin/:\$PATH" >> ~/.bashrc')

  os.environ['TAICHI_NUM_THREADS'] = '8'
  os.environ['TAICHI_ROOT_DIR'] = taichi_root_dir
  os.environ['PYTHONPATH'] = '{}/taichi/python/:'.format(taichi_root_dir) + os.environ.get('PYTHONPATH', '')
  os.environ['PATH'] = os.path.join(taichi_root_dir, 'taichi/bin') + ':' + os.environ.get('PATH', '')
  os.environ['PYTHONIOENCODING'] = 'utf-8'

  print('PYTHONPATH={}'.format(os.environ['PYTHONPATH']))
  
  if execute_command('echo $PYTHONPATH; python3 -c "import taichi as tc"') == 0:
    if execute_command('ti') != 0:
      print('  Warning: shortcut "ti" does not work.')
    if execute_command('taichi') != 0:
      print('  Warning: shortcut "taichi" does not work.')
    print('  Successfully Installed Taichi at ~/repos/taichi.')
    print('  Please execute')
    print('    source ~/.bashrc')
    print('  or restart your terminal.')
  else:
    print('  Error: installation failed.')
    exit(-1)


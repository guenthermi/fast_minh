import os
import shutil
import subprocess
from setuptools import setup

from cmake import CMAKE_BIN_DIR

CMAKE_ARGS_CONFIGURE = ['-B', 'build/', '-S', 'fast_minh/']
CMAKE_ARGS_BUILD = ['--build', 'build/', '--config', 'Release']


def execute_cmake(args):
    return subprocess.call([os.path.join(CMAKE_BIN_DIR, 'cmake')] + args, close_fds=False)

execute_cmake(CMAKE_ARGS_CONFIGURE)
execute_cmake(CMAKE_ARGS_BUILD)

shutil.copyfile('build/libfast_minh.so', 'fast_minh/libfast_minh.so')

setup()
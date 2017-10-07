#!/usr/bin/env python
# -*- coding: utf-8 -*-

import io
import os
import sys

from setuptools import find_packages, setup
from setuptools.command.test import test as TestCommand


NAME = 'spfg'
DESCRIPTION = 'Python bindings for SPFG.'
URL = 'https://github.com/ccortezia/spfg'
EMAIL = 'cristiano.cortezia@gmail.com'
AUTHOR = 'Cristiano Cortezia'
REQUIRES_PYTHON = '>=3.6.0'
VERSION = None

REQUIREMENTS_SETUP = []
REQUIREMENTS_TESTS = ['pytest-cov', 'pytest']

# --------------------------------------------------------------------------

here = os.path.abspath(os.path.dirname(__file__))

# Import the README and use it as the long-description.
with io.open(os.path.join(here, 'README.rst'), encoding='utf-8') as f:
    long_description = '\n' + f.read()

# Load version.
about = {}
if not VERSION:
    with open(os.path.join(here, NAME, '__version__.py')) as f:
        exec(f.read(), about)
else:
    about['__version__'] = VERSION


class PyTestCommand(TestCommand):

    user_options = [('pytest-args=', 'a', "Arguments to pass to pytest")]

    def initialize_options(self):
        TestCommand.initialize_options(self)
        self.pytest_args = ''

    def run_tests(self):
        import shlex
        import pytest
        errno = pytest.main(shlex.split(self.pytest_args))
        sys.exit(errno)


setup(
    name=NAME,
    version=about['__version__'],
    description=DESCRIPTION,
    long_description=long_description,
    author=AUTHOR,
    author_email=EMAIL,
    url=URL,
    packages=find_packages(exclude=('tests',)),
    python_requires=REQUIRES_PYTHON,
    tests_require=REQUIREMENTS_TESTS,
    setup_requires=REQUIREMENTS_SETUP,
    include_package_data=True,
    license='MIT',
    cmdclass={'test': PyTestCommand},
    classifiers=[
        # Trove classifiers
        # Full list: https://pypi.python.org/pypi?%3Aaction=list_classifiers
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: Implementation :: CPython',
        'Programming Language :: Python :: Implementation :: PyPy'
    ]
)

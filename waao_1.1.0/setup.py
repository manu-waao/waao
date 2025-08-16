from setuptools import setup, Extension
import os

waaoModule = Extension(
    "waao._waao",
    sources=[
        "pythonBindings/pyWaao.c",
        "src/waaoUtils.c",
        "src/waaoIo.c",
        "src/waaoArchive.c",
        "src/waaoConvert.c"
    ],
    include_dirs=["include"],
    extra_compile_args=["-O3", "-std=c11"]
)

setup(
    name="waao",
    version="1.1.0",
    packages=["waao"],
    description="WAAO: fast C utilities for files, archives, and conversions with Python bindings",
    ext_modules=[waaoModule],
    long_description_content_type='text/markdown',
    author="manu-waao",
    author_email="manukumar4648@gmail.com",
)


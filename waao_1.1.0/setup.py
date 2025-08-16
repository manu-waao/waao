import os
from setuptools import setup, find_packages

def read(fname):
    here = os.path.abspath(os.path.dirname(__file__))
    with open(os.path.join(here, fname), encoding="utf-8") as f:
        return f.read()

setup(
    name="waao",
    version="1.1.0",
    description="WAAO: Hybrid CPython package ",
    author="Manu Kumar",
    author_email="manukumar4648@gmail.com",
    url="https://github.com/manu-waao/waao",
    license="MIT",
    packages=find_packages(where=".", include=["waao", "waao.*"]),
    package_dir={"waao": "waao"},
    include_package_data=True,

    classifiers=[
        "Development Status :: 4 - Beta",
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],

    python_requires=">=3.7",
    install_requires=[],
    zip_safe=False,
)

from setuptools import setup, find_packages
import os

setup(
    name="waao",
    version="0.1",
    packages=find_packages(),
    description="A Python utility package for file operations including conversions, searching, displaying, and file management",
    long_description=open('README.md').read() if os.path.exists('README.md') else "A Python utility package for file operations",
    long_description_content_type='text/markdown',
    author="manu-waao",
    author_email="manukumar4648@gmail.com",
    url="https://github.com/manu-waao/waao",
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    install_requires=[
        "fpdf",           
        "Pillow",         
        "python-docx",    
        "PyPDF2",         
    ],
    python_requires='>=3.11',  
    entry_points={
        'console_scripts': [
            'fileops = fileops:main',
            'disp = disp:main',
            'conversions = conversions:main', 
            'search = search:main'
        ],
    },
)


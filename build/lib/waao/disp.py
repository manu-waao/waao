import os
from typing import List, Optional, Dict, TextIO

def openFile(fileName: str, filePath: str = "C:\\") -> Optional[TextIO]:
    """
    Opens a file for reading and writing. Tries to open in current directory first,
    then in the provided path if not found.

    Args:
        fileName: Name of the file to open
        filePath: Path where the file is located (default "C:\\")

    Returns:
        File object if successful, None otherwise
    """
    try:
        return open(f"./{fileName}", "r+")
    except FileNotFoundError:
        try:
            return open(f"{filePath}{fileName}", "r+")
        except FileNotFoundError:
            print(f"File not found: {fileName} in {filePath}")
            return None
    except Exception as e:
        print(f"File error: {e}")
        return None

def displayFileLines(fromStart: bool, numLines: int, fileName: str, filePath: str = "C:\\") -> Optional[List[str]]:
    """
    Displays specified number of lines from either start or end of a file.

    Args:
        fromStart: True for first lines, False for last lines
        numLines: Number of lines to display
        fileName: Name of file to read
        filePath: Path to file (default "C:\\")

    Returns:
        List of lines if successful, None otherwise
    """
    fileObj = openFile(fileName, filePath)
    if not fileObj:
        return None
    
    try:
        fileLines = fileObj.readlines()
        if len(fileLines) < numLines:
            print(f"File contains fewer than {numLines} lines.")
            return None
        
        return fileLines[:numLines] if fromStart else fileLines[-numLines:]
    finally:
        fileObj.close()

def findPatternInFile(pattern: str, fileName: str, filePath: str = "C:\\") -> int:
    """
    Searches for pattern in file and returns line number of first match.

    Args:
        pattern: Text pattern to search for
        fileName: Name of file to search
        filePath: Path to file (default "C:\\")

    Returns:
        Line number (1-based) if found, -1 otherwise
    """
    fileObj = openFile(fileName, filePath)
    if not fileObj:
        return -1
    
    try:
        for lineNum, line in enumerate(fileObj, 1):
            if pattern in line:
                return lineNum
        return -1
    finally:
        fileObj.close()

def findFilesOrDirs(findFiles: bool, path: str) -> List[str]:
    """
    Finds either files or directories in specified path.

    Args:
        findFiles: True to find files, False to find directories
        path: Directory path to search

    Returns:
        List of file/directory names found
    """
    if not os.path.exists(path):
        print(f"Path does not exist: {path}")
        return []
    
    checker = os.path.isfile if findFiles else os.path.isdir
    return [item for item in os.listdir(path) if checker(os.path.join(path, item))]

def checkFilePermissions(filePath: str = "C:\\") -> Optional[Dict[str, bool]]:
    """
    Checks permissions for specified file or directory.

    Args:
        filePath: Path to check (default "C:\\")

    Returns:
        Dictionary with read/write/execute permissions if exists, None otherwise
    """
    if not os.path.exists(filePath):
        return None
    
    return {
        'read': os.access(filePath, os.R_OK),
        'write': os.access(filePath, os.W_OK),
        'execute': os.access(filePath, os.X_OK)
    }
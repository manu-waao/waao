import os
import concurrent.futures
from typing import List, Optional, Tuple, Set, TextIO

def openFile(fileName: str, filePath: str = "C:\\") -> Optional[TextIO]:
    """
    Opens a file for reading and writing. Attempts to open in current directory first,
    then in the provided path if not found.

    Args:
        fileName: Name of the file to open
        filePath: Path to search (default "C:\\")

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

def searchDirectory(targetType: int, targetName: str, directory: str) -> List[str]:
    """
    Searches for a file or directory in a given directory path.

    Args:
        targetType: 0 for file, 1 for directory
        targetName: Name of file/directory to find
        directory: Path to search in

    Returns:
        List of full paths to matching items
    """
    found = []
    try:
        if targetType == 0:  # File search
            if targetName in os.listdir(directory):
                found.append(os.path.join(directory, targetName))
        elif targetType == 1:  # Directory search
            for item in os.listdir(directory):
                if item == targetName and os.path.isdir(os.path.join(directory, item)):
                    found.append(os.path.join(directory, targetName))
    except PermissionError:
        pass
    return found

def findInFolder(targetType: int, targetName: str, rootPath: str = "C:\\") -> List[str]:
    """
    Recursively searches for files/directories using multiple threads.

    Args:
        targetType: 0 for file, 1 for directory
        targetName: Name to search for
        rootPath: Starting directory (default "C:\\")

    Returns:
        List of full paths to matches
    """
    if targetType not in (0, 1):
        print("Invalid targetType. Use 0 for files or 1 for directories.")
        return []

    found = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=20) as executor:
        futures = []
        for dirPath, dirNames, _ in os.walk(rootPath):
            futures.append(executor.submit(
                searchDirectory, targetType, targetName, dirPath
            ))
            for subDir in dirNames:
                futures.append(executor.submit(
                    searchDirectory, targetType, targetName, 
                    os.path.join(dirPath, subDir)
                ))

        for future in concurrent.futures.as_completed(futures):
            found.extend(future.result())

    return found

def checkFileSize(filePath: str, fileName: str, minSize: int) -> Optional[Tuple[str, int]]:
    """
    Checks if a file exceeds the specified size.

    Args:
        filePath: Directory containing the file
        fileName: Name of file to check
        minSize: Size threshold in bytes

    Returns:
        Tuple of (fullPath, size) if file is large enough, else None
    """
    fullPath = os.path.join(filePath, fileName)
    if os.path.isfile(fullPath):
        size = os.path.getsize(fullPath)
        if size > minSize:
            return (fullPath, size)
    return None

def findLargeFiles(directory: str, minSize: int) -> List[Tuple[str, int]]:
    """
    Finds files larger than specified size using multiple threads.

    Args:
        directory: Path to search
        minSize: Minimum file size in bytes

    Returns:
        List of (filePath, size) tuples
    """
    found = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=20) as executor:
        futures = {
            executor.submit(checkFileSize, directory, fileName, minSize)
            for fileName in os.listdir(directory)
        }
        for future in concurrent.futures.as_completed(futures):
            result = future.result()
            if result:
                found.append(result)
    return found

def getUniqueWords(fileName: str, filePath: str = "C:\\") -> List[str]:
    """
    Finds words that appear only once in a file.

    Args:
        fileName: Name of file to analyze
        filePath: Path to file (default "C:\\")

    Returns:
        List of unique words
    """
    fileObj = openFile(fileName, filePath)
    if not fileObj:
        return []
    
    with fileObj:
        words = fileObj.read().split()
    
    return [word for word in words if words.count(word) == 1]

def findDuplicateWords(fileName: str, filePath: str = "C:\\") -> List[str]:
    """
    Finds words that appear multiple times in a file.

    Args:
        fileName: Name of file to analyze
        filePath: Path to file (default "C:\\")

    Returns:
        List of duplicate words
    """
    uniqueWords = set(getUniqueWords(fileName, filePath))
    fileObj = openFile(fileName, filePath)
    if not fileObj:
        return []
    
    with fileObj:
        allWords = set(fileObj.read().split())
    
    return list(allWords - uniqueWords)

def searchPattern(fileName: str, pattern: str, filePath: str = "C:\\") -> int:
    """
    Finds first occurrence of a pattern in a file (case-insensitive).

    Args:
        fileName: Name of file to search
        pattern: Text pattern to find
        filePath: Path to file (default "C:\\")

    Returns:
        Index of first match or -1 if not found
    """
    fileObj = openFile(fileName, filePath)
    if not fileObj:
        return -1
    
    with fileObj:
        content = fileObj.read().lower()
    
    return content.find(pattern.lower())
import os
import shutil
from typing import List, Optional, TextIO

def openFile(fileName: str, filePath: str = "C:\\") -> Optional[TextIO]:
    """
    Opens a file for reading and writing. If the file is not found in the 
    specified path, it attempts to open it in the current directory.

    Args:
        fileName: The name of the file to open.
        filePath: The path where the file is located (default is "C:\\").

    Returns:
        A file object for reading and writing, or None if an error occurs.
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

def makeFile(fileName: str, extension: str) -> bool:
    """
    Creates a new file with the specified name and extension in the current directory.

    Args:
        fileName: The name of the file to create.
        extension: The extension to use for the file (e.g., ".txt", ".c").

    Returns:
        True if file was created, False if it already exists or an error occurred.
    """
    try:
        with open(f"{fileName}{extension}", "x"):
            pass
        print(f"File '{fileName}{extension}' created!")
        return True
    except FileExistsError:
        print(f"File '{fileName}{extension}' already exists.")
        return False
    except Exception as e:
        print(f"Error occurred: {e}")
        return False

def makeFolder(folderName: str, folderPath: str = "C:\\") -> bool:
    """
    Creates a folder in the specified path.

    Args:
        folderName: The name of the folder to create.
        folderPath: The path where the folder should be created (default is "C:\\").

    Returns:
        True if folder was created or already exists, False on error.
    """
    fullPath = os.path.join(folderPath, folderName)
    try:
        os.makedirs(fullPath, exist_ok=True)
        print(f"Folder '{folderName}' created at {fullPath}")
        return True
    except Exception as e:
        print(f"Error occurred while creating folder: {e}")
        return False

def sortFile(ascendingOrder: bool, fileName: str, filePath: str = "C:\\") -> bool:
    """
    Sorts the lines in a text file in either ascending or descending order.

    Args:
        ascendingOrder: If True, sort ascending; if False, sort descending.
        fileName: The name of the file to sort.
        filePath: The path where the file is located (default is "C:\\").

    Returns:
        True if sorting succeeded, False otherwise.
    """
    try:
        with openFile(fileName, filePath) as fileObj:
            if fileObj is None:
                return False
            lines = fileObj.readlines()
        
        if not lines:
            print(f"The file '{fileName}' is empty.")
            return False
            
        lines.sort(reverse=not ascendingOrder)
        
        with openFile(fileName, filePath) as fileObj:
            if fileObj is None:
                return False
            fileObj.writelines(lines)
            
        print(f"File '{fileName}' sorted {'ascending' if ascendingOrder else 'descending'}")
        return True
        
    except FileNotFoundError:
        print(f"File '{fileName}' not found in path '{filePath}'")
    except PermissionError:
        print(f"Permission denied when accessing file '{fileName}'")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
    return False

def compressFile(fileList: List[str], zipName: str, zipFormat: str) -> bool:
    """
    Creates a compressed archive from a list of files.

    Args:
        fileList: List of file paths to include in the archive.
        zipName: The name of the archive to create.
        zipFormat: The archive format ("zip", "tar", "gztar", "bztar").

    Returns:
        True if archive was created successfully, False otherwise.
    """
    tempDir = zipName
    try:
        os.makedirs(tempDir, exist_ok=True)
        
        for file in fileList:
            if not os.path.exists(file):
                print(f"File not found: {file}")
                shutil.rmtree(tempDir)
                return False
            shutil.copy(file, tempDir)
            
        shutil.make_archive(zipName, zipFormat, tempDir)
        shutil.rmtree(tempDir)
        print(f"Archive '{zipName}.{zipFormat}' created successfully")
        return True
        
    except Exception as e:
        if os.path.exists(tempDir):
            shutil.rmtree(tempDir)
        print(f"Error creating archive: {e}")
        return False
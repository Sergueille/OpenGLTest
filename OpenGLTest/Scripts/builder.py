import os
import re
import shutil

class SelectedFile():
    """
    Used by Selection class, represents a file with some infos
    """

    def __init__(self, path):
        self.path = path

        self.ignore_path = False
        self.rel_path = ""

        self.is_renamed = False
        self.new_name = ""


class Selection:
    """
    Represents a list of selected files, from a base directory
    """

    def __init__(self, base_path="", add_all=False):
        self.base_path = base_path
        self.files = []

        if add_all:
            subdir = [base_path] # Directories to search for
            while len(subdir) > 0:
                subdir_copy = subdir
                subdir = []
                for dir in subdir_copy:
                    for element in os.scandir(dir): # For each file in the directory
                        if element.is_file(): # If it's a file, add it to list
                            self.add_file_abs(element.path)
                        elif element.is_dir(): # If it's a directory, scan it on the next iteration
                            subdir.append(element.path)


    def add_dir(self, path, regex=""):
        """
        Adds a directory to the selection (with relative path), with files that match the regex (name only)
        """
        self.add_dir_abs(self.base_path + "\\" + path, regex)
        return self


    def add_file(self, path):
        """
        Adds a file (relative path) to the selection
        """

        file = SelectedFile(self.base_path + "\\" + path)
        file.rel_path = path
        self.files.append(file)

        return self


    def add_files_regex(self, regex):
        """
        Adds all files in the base directory that match the regex (name only)
        """
        for element in os.scandir(self.base_path):
            if element.is_file():
                if re.match(regex, element.name):
                    self.add_file_abs(element.path)

        return self


    def add_dir_abs(self, path, regex=""):
        """
        Adds a directory (with ABSOLUTE path) to the selection, with files that match the regex
        """
        search_dir = [path]
        while len(search_dir) > 0:
            search_dir_copy = search_dir
            search_dir = []
            for dir in search_dir_copy:
                for element in os.scandir(dir):
                    if element.is_file():
                        if regex == "" or re.match(regex, element.name):
                            self.add_file_abs(element.path)
                    elif element.is_dir():
                        search_dir.append(element.path)

        return self


    def add_file_abs(self, path):
        """
        Adds a file (with ABSOLUTE path) to the selection
        """
        file = SelectedFile(path)

        i = 0
        while i < len(self.base_path) and i < len(path) and self.base_path[i] == path[i]:
            i += 1

        if i < 3: i = 3

        file.rel_path = path[i:]

        self.files.append(file)

        return self


    def remove_dir(self, path):
        """
        Remove a directory from selection (relative path)
        """

        self.remove_dir_abs(self.base_path + "\\" + path)
        return self


    def remove_dir_abs(self, path):
        """
        Remove a directory from selection (absolute path)
        """
        search_dir = [path]
        while len(search_dir) > 0:
            search_dir_copy = search_dir
            search_dir = []
            for dir in search_dir_copy:
                for element in os.scandir(dir):
                    if element.is_file():
                        self.remove_file(element.path)
                    elif element.is_dir():
                        search_dir.append(element.path)

        return self


    def remove_file(self, path):
        """
        Remove a file from selection (relative or absolute path)
        """
        for i in range(len(self.files)):
            file = self.files[i]
            if file.path == path or file.rel_path == path:
                del self.files[i]

        return self


    def remove_files_regex(self, regex):
        """
        Remove all files in the base directory that match the regex (name only)
        """
        i = 0
        while i < len(self.files):
            file = self.files[i]
            file_name = get_file_name(file.path)

            if re.match(regex, file_name):
                del self.files[i]
            else:
                i += 1

        return self


    def delete(self): # NOT CHECKED
        """
        Delete all the selected files
        """
        input("Going to delete " +  str(len(self.files)) + " files")

        for file in self.files:
            os.remove(file.path)

        return self


    def copy_to(self, location):
        """
        Copy the selected file to the specified location, without deleting the originals
        """
        input("Going to copy " +  str(len(self.files)) + " files")

        for file in self.files:
            if file.ignore_path:
                new_path = shutil.copy2(file.path, location)

                if file.is_renamed:
                    os.rename(new_path, location + "\\" + file.new_name)

            else:
                full_path = location + "\\" + file.rel_path
                last_slash = full_path.rindex("\\")

                if file.is_renamed:
                    full_path = full_path[:last_slash] + "\\" + file.new_name

                try:
                    os.makedirs(full_path[:last_slash])
                except FileExistsError:
                    pass
                shutil.copy2(file.path, full_path)

        print("done")

        return self


    def move_to(self, location): # NOT CHECKED
        """
        Moves the selected file to the specified location, but deletes the originals
        """
        self.copy_to(location)
        self.delete()

        return self


    def renamed(self, new_name):
        """
        Will give a new name to the last added file (only applied when copying or moving, doesn't change the original)
        """
        self.files[-1].is_renamed = True
        self.files[-1].new_name = new_name
        return self


    def ignore_path(self):
        """
        Last added file will be copied directly in the base directory
        """
        self.files[-1].ignore_path = True
        return self


    def override_path(self, new_path):
        """
        Last added file will have the relative path (must include name)
        """
        self.files[-1].rel_path = new_path
        return self



def create_dir(path, ensure_empty=False):
    """
    Creates a new directory at the sepcified path
    Returns the path of the rirectory
    """
    try:
        os.makedirs(path)
    except FileExistsError:
        if ensure_empty:
            del_files = list(os.scandir(path))

            if len(del_files) > 0:
                input("Going to delete files in " + str(path) + " (" + str(len(del_files)) + " elements)")

                for file in del_files:
                    if file.is_file():
                        os.remove(file)
                    else:
                        shutil.rmtree(file)

    return path


def get_file_name(path):
    last_slash = path.rindex("\\")
    return path [last_slash:]

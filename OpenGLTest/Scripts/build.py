import builder
import os

base = os.getcwd() # Base directory
build = builder.create_dir(base + "\\Build", ensure_empty=True) # Build directory
project = base + "\\OpenGLTest" # Project directory

builder.Selection(project, False)\
    .add_dir("Fonts")\
    .add_dir("Images")\
    .add_dir("Levels")\
    .add_dir("Lightmaps")\
    .add_dir("Settings")\
    .add_dir("Shaders")\
    .add_dir("Sounds").remove_dir("Sounds\\Ref")\
    .add_dir("Simulations")\
    .add_dir("Localization")\
    .add_dir_abs(base + "\\Docs")\
    .add_file_abs("D:\\alexis\\Logiciels\\freetype-windows-binaries-2.11.1\\release dll\\win64\\freetype.dll").ignore_path()\
    .copy_to(build)

builder.create_dir(build + "\\Saves")

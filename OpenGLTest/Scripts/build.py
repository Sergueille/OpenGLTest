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
    .add_dir("Settings").remove_file("options.set").remove_file("progress.set")\
    .add_dir("Shaders")\
    .add_dir("Sounds").remove_dir("Sounds\\Ref")\
    .add_dir("Simulations")\
    .add_dir("Localization")\
    .add_dir_abs(base + "\\Docs")\
    .add_file_abs(base + "\\x64\\Release\\OpenGLTest.exe").ignore_path().renamed("Game.exe")\
    .add_file_abs(base + "\\x64\\Release\\freetype.dll").ignore_path()\
    .add_file_abs(base + "\\credits.txt")\
    .copy_to(build)

builder.create_dir(build + "\\Saves")

builder.create_file(build + "\\Settings\\options.set", """
Props {
	displayFPS: "1"
	fullscreen: "1"
	gameSoundsVolume: "1.000000"
	globalVolume: "1.000000"
	key0: "65"
	key1: "68"
	key2: "87"
	key3: "83"
	key4: "69"
	keyalt0: "263"
	keyalt1: "262"
	keyalt2: "32"
	keyalt3: "264"
	keyalt4: "257"
	language: "1"
	monitor: "main"
	musicVolume: "1.000000"
	screenX: "1600"
	screenY: "900"
	uiVolume: "0.500000"
}
""")

builder.create_file(build + "\\Settings\\progress.set", """
Props {
    unlockedChapters: "1"
    editorWarning: "1"
}
""")

builder.create_file(build + "\\README.txt", """INSTRUCTIONS:
- Lancez le jeu avec l'executable a la racine du dossier
- Pour utiliser l'éditeur de niveaux, réferez vous à la documentation (Docs\\fr\\index.html)

BUGS CONNUS:
- Le jeu risque de planter pendant le chargement de la map 6 à la map 7
""")

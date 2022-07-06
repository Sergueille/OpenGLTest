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
builder.create_readme(build + "\\README.txt", """INSTRUCTIONS:
- Lancez le jeu avec l'executable a la racine du dossier
- Les paramètres ne sont pas réglables pendant le jeu, mais vous pouvez les modifier dans le fichier Settings\\options.set
- Pour utiliser l'éditeur de niveaux, réferez vous à la documentation (Docs\\fr\\index.html)

Le jeu contients acuellements 5 niveaux terminés et 3 niveaux en cours de préparation

BUGS CONNUS:
- Le jeu risque de planter pendant le chargement des niveaux
- L'encodage de texte ne marche pas bien, et les accents sont remplacés par d'autres caractères
- Vers la fin du niveaux 6, la téleportation ne marche plus et il est presque impossible de passer, mais de toute facon le jeu n'est pas encore prêt après cet endroit, donc il faut s'arrêter ici

Après avoir joué, merci de transmettre ces infos au développeur:
- temps de jeu total
- images pas secondes (affichées en haut à droite)
- cartie graphique, caractérisitiques du processeur et version de windows
""")

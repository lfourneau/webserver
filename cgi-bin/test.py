#!/usr/bin/env python3
import os, sys

methode = os.environ["REQUEST_METHOD"]
if methode == "GET":
    donnees = os.environ.get("QUERY_STRING", "")         # "nom=Louis"
else:
    taille = int(os.environ.get("CONTENT_LENGTH", "0"))
    donnees = sys.stdin.read(taille)                        # "nom=Louis"

nom = donnees.split("=")[1] if "=" in donnees else "inconnu"

print("Content-Type: text/html")   # header CGI
print()                             # ligne vide obligatoire
print(f"<h1>Bonjour {nom}</h1>")    # contenu
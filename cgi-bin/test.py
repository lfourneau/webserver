#!/usr/bin/env python3
import os, sys

methode = os.environ["HTTP_REQUEST_METHOD"]
if methode == "GET":
    donnees = os.environ.get("HTTP_QUERY_STRING", "")         # "nom=Louis"
else:
    taille = int(os.environ.get("CONTENT_LENGTH", "0"))
    donnees = sys.stdin.read(taille)                        # "nom=Louis"

nom = donnees.split("=")[1] if "=" in donnees else "inconnu"

print(f"<h1>Bonjour {nom}</h1>")    # contenu
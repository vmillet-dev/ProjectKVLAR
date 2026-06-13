# Code name: Project KVLAR

Also in [![United Kingdom](https://raw.githubusercontent.com/stevenrskelton/flag-icon/master/png/16/country-4x3/gb.png "United Kingdom")](README.md)

Ce dépôt contient un projet de jeu développé avec Unreal Engine 5 et C++.

## Prérequis

Pour compiler et lancer ce projet, assurez-vous d'avoir installé :

* **Unreal Engine 5.7.4**
* **JetBrains Rider** (IDE recommandé)
* **Visual Studio Build Tools** (avec la charge de travail développement de jeux en C++ et le SDK Windows requis)

## Configuration & Flux de Travail

Nous utilisons JetBrains Rider avec l'ouverture directe du fichier `.uproject`. Il n'est **pas nécessaire** de générer les fichiers de projet Visual Studio (`.sln`).

1. Clonez ce dépôt.
2. Ouvrez **JetBrains Rider**.
3. Sélectionnez **Open File or Folder** et choisissez le fichier `*.uproject`.
4. Rider va automatiquement indexer le projet et gérer les dépendances.
5. Sélectionnez votre configuration de build (ex: `Development Editor`) et compilez le projet (`Ctrl+Shift+B`).

## Problèmes Récurrents & Solutions

### 1. Nettoyage des Caches (Anomalies de Compilation)
Si vous rencontrez des erreurs de linkage bizarres ou des modifications de code qui ne s'appliquent pas en jeu :
1. Fermez l'Éditeur Unreal et JetBrains Rider.
2. Supprimez les dossiers suivants à la racine du projet :
    * `Binaries/`
    * `Intermediate/`
    * `Saved/`
    * `Build/`
3. Réouvrez le fichier `*.uproject` dans Rider. Cliquez sur **Oui** lorsque l'on vous demande de recompiler les modules manquants.

### 2. Soucis liés au Live Coding
Le Live Coding peut verrouiller les fichiers binaires ou créer des désynchronisations entre Rider et l'Éditeur.
* Si la compilation échoue à cause de fichiers verrouillés, désactivez le Live Coding dans l'Éditeur Unreal (icône en bas à droite ou `Ctrl+Alt+F11`) et compilez depuis Rider.
* **Bonne pratique :** Fermez l'Éditeur Unreal lors de modifications de fichiers d'en-tête (`.h`), d'ajouts de nouvelles classes ou de changements de macros UPROPERTY/UFUNCTION. Compilez depuis Rider avant de relancer l'Éditeur.

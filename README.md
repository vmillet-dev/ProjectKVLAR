# CityBuilder

Developed with Unreal Engine 5

## Building UE5 Windows Docker Image

### About this workaround

Building a Docker image from Unreal Engine normally requires compiling the engine
from source, which involves linking your GitHub account to your Epic Games account
and cloning the private UE5 repository — a process that can take several hours.

To avoid this, the files located in the `.docker` folder are designed to be placed
directly at the root of your local Unreal Engine installation directory
(e.g. `C:\Program Files\Epic Games\UE_5.7`), downloaded via the Epic Games Launcher.
This allows Docker to copy the pre-built engine binaries directly from your local
installation, completely bypassing the source compilation step.

### Goal

The goal is to produce a ready-to-use Docker image that can be pushed to a container
registry and pulled by a **CircleCI pipeline** to automate the build and packaging
of the project without requiring any local UE5 installation on the CI runner.

### Steps

- Install Unreal Engine `5.7` via the Epic Games Launcher
- Copy the contents of the `.docker` folder to the root of your UE installation
  (e.g. `C:\Program Files\Epic Games\UE_5.7`)
- Build the Docker image from that directory
- Push the image to your container registry (e.g. `ghcr.io`)
- The CircleCI pipeline will automatically pull and use it on every build
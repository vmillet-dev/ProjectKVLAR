# Code name: Project KVLAR

Aussi en [![France](https://raw.githubusercontent.com/stevenrskelton/flag-icon/master/png/16/country-4x3/fr.png "France")](README_fr.md)

This repository contains an Unreal Engine 5 C++ game project.

## Project Features

## Getting Started

### Prerequisites

To build and run this project, ensure you have the following installed:

* **Unreal Engine 5.7.4**
* **JetBrains Rider** (Recommended IDE)
* **Visual Studio Build Tools** (with C++ desktop/game development workloads and required Windows SDK)

### Project Setup & Workflow

We use JetBrains Rider with the direct `.uproject` opening workflow. You do **not** need to generate Visual Studio project files (`.sln`).

1. Clone this repository.
2. Open **JetBrains Rider**.
3. Select **Open File or Folder** and choose the `*.uproject` file.
4. Rider will automatically index the project and resolve dependencies.
5. Select your build configuration (e.g., `Development Editor`) and build the project (`Ctrl+Shift+B`).

## CI/CD

### CircleCI environment variables

The project uses CircleCI to automate build, packaging, and release workflows for Windows targets. The following variables must be set in the CircleCI **`github` context**
(Project Settings → Contexts) before a tagged build can succeed:

| Variable               | Description                                                          |
|------------------------|----------------------------------------------------------------------|
| `GITHUB_TOKEN`         | GitHub token with `read:packages` and `write:packages` permissions   |
| `GITHUB_USERNAME`      | GitHub username                                                      |
| `UE5_DOCKER_IMAGE`     | UE5 Docker image reference                                           |
| `PROJECT_NAME`         | Unreal project name (`.uproject`)                                    |
| `EOS_ARTIFACT_NAME`    | EOS artifact name                                                    |
| `EOS_CLIENT_ID`        | EOS Client ID                                                        |
| `EOS_CLIENT_SECRET`    | EOS Client Secret                                                    |
| `EOS_PRODUCT_ID`       | EOS Product ID                                                       |
| `EOS_SANDBOX_ID`       | EOS Sandbox ID                                                       |
| `EOS_DEPLOYMENT_ID`    | EOS Deployment ID                                                    |
| `EOS_ENCRYPTION_KEY`   | 64-character hex encryption key                                      |

These values replace the placeholders in `Config/DefaultEngine.ini` at build time
and must never be committed to the repository.

### How to build Windows UE5 image for CI

To avoid compiling Unreal Engine from source during CI execution, the pipeline relies on a prebuilt Docker image containing a local Unreal Engine installation for Windows target. 
I wrote a doc to explain how to build this image [here](Docs/HOW_TO_BUILD_WINDOWS_UE_IMAGE.md). 

## Licence

## Credits
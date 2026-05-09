# CityBuilder

Unreal Engine 5 project with automated build and release pipeline.

---

## CI/CD

The project uses CircleCI to automate build, packaging, and release workflows for Windows targets.

To avoid compiling Unreal Engine from source during CI execution, the pipeline relies on a prebuilt Docker image containing a local Unreal Engine installation.

### UE5 Docker Image Setup

#### Prerequisites

* Unreal Engine `5.7` installed via Epic Games Launcher
* Docker installed
* Access to GitHub Container Registry (GHCR)

#### Build the Image

1. Copy the `.docker` directory into the root of your UE installation:

```bash id="bgx3me"
xcopy /E /I .docker "C:\Program Files\Epic Games\UE_5.7"
```

2. Authenticate to GHCR:

```bash id="c3d6zf"
echo YOUR_GITHUB_TOKEN | docker login ghcr.io -u YOUR_GITHUB_USERNAME --password-stdin
```

3. Build the Docker image:

```bash id="tmjv6g"
cd "C:\Program Files\Epic Games\UE_5.7"

docker build -t ghcr.io/YOUR_GITHUB_USERNAME/ue5-windows:5.7 .
```

4. Push the image:

```bash id="bspdz0"
docker push ghcr.io/YOUR_GITHUB_USERNAME/ue5-windows:5.7
```

> Rebuild the image only when upgrading the Unreal Engine version.

### Pipeline Workflow

On each push of tags, CircleCI:

1. Pulls the UE5 Docker image
2. Runs `BuildCookRun` and packages the project for Windows Shipping (64-bit)
3. Archives the build as `.zip`
4. Publishes the build to a GitHub Release

### CircleCI Environment Variables

Configure the following variables inside the `github` context:

| Variable           | Description                                                        |
| ------------------ | ------------------------------------------------------------------ |
| `GITHUB_TOKEN`     | GitHub token with `read:packages` and `write:packages` permissions |
| `GITHUB_USERNAME`  | GitHub username                                                    |
| `UE5_DOCKER_IMAGE` | UE5 Docker image reference                                         |
| `PROJECT_NAME`     | Unreal project name (`.uproject`)                                  |

#### Example

```text id="lnvuzq"
UE5_DOCKER_IMAGE=ghcr.io/yourname/ue5-windows:5.7
PROJECT_NAME=CityBuilder
```

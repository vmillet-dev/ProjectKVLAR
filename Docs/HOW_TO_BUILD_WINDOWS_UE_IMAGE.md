# UE5 Docker Image Setup

## Prerequisites

* Unreal Engine `5.7` installed via Epic Games Launcher
* Docker installed
* Access to GitHub Container Registry (GHCR)

## Build the Image

1. Copy the content of `docker` directory into the root of your UE installation with this command:

```bash id="bgx3me"
xcopy /E /I docker "C:\Program Files\Epic Games\UE_5.7"
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
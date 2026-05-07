FROM mcr.microsoft.com/windows/server:ltsc2022 AS ddl

FROM mcr.microsoft.com/windows/servercore:ltsc2022

COPY --from=ddl \
    C:/Windows/System32/avicap32.dll \
    C:/Windows/System32/avifil32.dll \
    C:/Windows/System32/avrt.dll \
    C:/Windows/System32/d3d10warp.dll \
    C:/Windows/System32/D3DSCache.dll \
    C:/Windows/System32/dsound.dll \
    C:/Windows/System32/dxva2.dll \
    C:/Windows/System32/glu32.dll \
    C:/Windows/System32/InputHost.dll \
    C:/Windows/System32/mf.dll \
    C:/Windows/System32/mfcore.dll \
    C:/Windows/System32/mfplat.dll \
    C:/Windows/System32/MFPlay.dll \
    C:/Windows/System32/mfreadwrite.dll \
    C:/Windows/System32/msacm32.dll \
    C:/Windows/System32/msdmo.dll \
    C:/Windows/System32/msvfw32.dll \
    C:/Windows/System32/opengl32.dll \
    C:/Windows/System32/RESAMPLEDMO.DLL \
    C:/Windows/System32/qwave.dll \
    C:/Windows/System32/ResourcePolicyClient.dll \
    C:/Windows/System32/XInput1_4.dll \
    C:/Windows/System32/


RUN reg add HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\FileSystem /v LongPathsEnabled /t REG_DWORD /d 1 /f

RUN powershell -NoProfile -ExecutionPolicy Bypass -Command "$env:chocolateyVersion = '1.4.0'; Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))"

ARG 17

COPY ["install-prerequisites.ps1", "C:/install-prerequisites.ps1"]

RUN powershell -ExecutionPolicy Bypass -File C:\install-prerequisites.ps1 %VISUAL_STUDIO_BUILD_NUMBER%

WORKDIR /UnrealEngine

RUN mkdir C:\UnrealEngine\Engine

COPY ["Engine/Binaries", "C:/UnrealEngine/Engine/Binaries"]
COPY ["Engine/Content", "C:/UnrealEngine/Engine/Content"]
COPY ["Engine/Extras", "C:/UnrealEngine/Engine/Extras"]
COPY ["Engine/Intermediate", "C:/UnrealEngine/Engine/Intermediate"]
COPY ["Engine/Plugins", "C:/UnrealEngine/Engine/Plugins"]
COPY ["Engine/Source", "C:/UnrealEngine/Engine/Source"]
COPY ["Engine/DerivedDataCache", "C:/UnrealEngine/Engine/DerivedDataCache"]
COPY ["Engine/Build", "C:/UnrealEngine/Engine/Build"]
COPY ["Engine/Platforms", "C:/UnrealEngine/Engine/Platforms"]
COPY ["Engine/Shaders", "C:/UnrealEngine/Engine/Shaders"]
COPY ["Engine/Programs", "C:/UnrealEngine/Engine/Programs"]
COPY ["Engine/Config", "C:/UnrealEngine/Engine/Config"]

CMD ["cmd"]
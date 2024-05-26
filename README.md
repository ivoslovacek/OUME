# OUMP
### Openly Useless Media Player

> [!WARNING]
> This software is under development, thus everything wrong with it is a feature not a bug.

## OUMP is a basic video player, that provides sensational functionality
- basic video playing with audio
- seeking in those videos
- possibilities of encountering random formats that libav can't handle

## Dependencies
* `Qt6` ([Qt Licensing](https://doc.qt.io/qt-6/licensing.html))
* `FFmpeg` ([FFmpeg Licensing](https://www.ffmpeg.org/legal.html))
* `PulseAudio` ([PulseAudio Licensing](https://www.freedesktop.org/wiki/Software/PulseAudio/About/))

### Installing dependencies on Arch Linux
**Qt:**

```bash
sudo pacman -Syu qt6-base
```
**FFmpeg:**

```bash
sudo pacman -Syu ffmpeg
```

**PulseAudio:**

```bash
sudo pacman -Syu pulseaudio
```

### Compiling the project:
**In Release:**
```bash
mkdir build; cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
```

**In Debug:**
```bash
mkdir build; cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## Important notes

- This isn't a great video player by anymeans, but it gets the job done.
- For some reason, Qt doesn't want to display the video without resizing the window first, after manually resizing it works well.

### Why should you use it?

You really shouldn't, because the ux can vary depending of the codecs used to encode your media. *Sometime its even like playing russian roulette*

### Can this player be improved?

Certainly yes, but be warned of the shitstorm that is **libav** and **Qt**.

### Final thoughts?

Don't look at the source code, it causes eye cancer.


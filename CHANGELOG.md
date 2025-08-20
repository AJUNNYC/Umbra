# Changelog

All notable changes to the plugin will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Todo
- Fix high CPU usage (currently 75-85% constant usage)
- Resolve volume spikes during parameter changes
- Fix system freezing during parameter automation
- Improve stability for live performance use
- Test compatibility on macOS and Linux
- Test compatibility with other DAWs beyond REAPER
- Optimize performance to reduce baseline CPU usage

## [1.0.0] - 2025-08-19

### Added
- **8-channel audio processing** with parallel diffusion and feedback networks
- **Real-time 3D spectrogram visualization** using OpenGL
  - X-axis: Frequency (logarithmic scale, 25 points)
  - Y-axis: Magnitude (-60dB to 0dB range)
  - Z-axis: Time (50 frames of history, 30Hz refresh rate)
  - Wireframe display in velvet red with transparency
- **Advanced reverb algorithm** combining:
  - Dark Velvet Noise (DVN) diffusers
  - Dual Feedback Delay Networks (FDN) with Hadamard matrix
  - Recursive Running-Sum (RRS) filters for efficiency
- **Comprehensive parameter controls**:
  - Mix (0-100%): Dry/wet signal blend
  - Stereo Width (0.0-2.0): Stereo separation control
  - Low Pass Filter (20-20,000 Hz): High frequency attenuation
  - High Pass Filter (20-20,000 Hz): Low frequency attenuation
  - Room Size (0.1-2.0): Delay length scaling for room simulation
  - Dampening (20-20,000 Hz): Feedback path low-pass filtering
  - Initial Delay (0.0-0.1 seconds): Pre-reverb delay
- **Performance optimizations**:
  - OpenMP parallelization for multi-channel processing
  - VBO rendering for efficient GPU utilization
  - Thread-safe FFT processing with critical sections
  - Magnitude filtering to skip rendering quiet signals
  - Limited DVN pulses (200 maximum) for real-time constraints
- **VST3 plugin format** support
- **Cross-platform build system** using JUCE Framework 8.0.8



### Known Issues
- **High CPU Usage**: 75-85% constant usage regardless of audio state
- **Volume Spikes**: Rapid parameter changes can cause sudden volume increases
- **System Instability**: Parameter automation may cause temporary system lag/freezing
- **Limited Platform Testing**: Only tested on Windows 11
- **Limited DAW Testing**: Only verified with REAPER v7.33 (WinX64)
- **Live Performance**: Not recommended due to stability issues



---

**Note**: This is experimental software. Always use appropriate gain staging, monitor levels carefully, and test thoroughly before any critical use.
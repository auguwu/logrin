---
title: Logrin | Changelog
availableAt:
    - https://github.com/auguwu/logrin/blob/master/CHANGELOG.md
    - https://floofy.dev/oss/logrin/changelog
---

### unreleased
**Git History**: <https://github.com/Noelware/violet/compare/26.02.04...master>

### 26.02.04
- Bump **Noelware.Violet** frameworks to 26.03.04
- Add OpenTelemetry sink support
- Remove `otel_sink` and `http_sink` flags
- Removed the "uber" library

**Git History**: <https://github.com/Noelware/violet/compare/26.02.03...26.02.04>

### 26.02.03
- Fix errors when using references in **Console::WithFormatter** (@auguwu)
- Add more safe guards for using `LogLevel::Off` in the log factory (@auguwu)
- Fix UAF in `LogFactory::Shutdown` (@auguwu)

**Git History**: <https://github.com/auguwu/logrin/compare/26.02.02...26.02.03>

### 26.02.02
- Don't assert if no log factory was ever initialized, just provide a dummy logger instead (@auguwu)
- Add **LogLevel::Off** to turn off all logging (@auguwu)
- Bump **Noelware.Violet** frameworks to 26.03.04 (@auguwu)

**Git History**: <https://github.com/auguwu/logrin/compare/26.02.01...26.02.02>

### 26.02.01
- Bump **Noelware.Violet** frameworks to 26.03 (@auguwu)

**Git History**: <https://github.com/auguwu/logrin/compare/26.02...26.02.01>

### 26.02
- Bump **Noelware.Violet** frameworks to 26.02.03
- Include missing header files

**Git History**: <https://github.com/Noelware/violet/compare/26.01.02...26.02>

### 26.01.02
- Remove `LOGRIN_VERSION_STR` and add `LOGRIN_VERSION` to list of `define`s instead of `copt`s

**Git History**: <https://github.com/Noelware/violet/compare/26.01.01...26.01.02>

### 26.01.01
- Add **LOGRIN_VERSION** and **LOGRIN_VERSION_STR** definitions
- Use mutexes and file descriptors for console sink instead of `std::c{out|err}`

#### Breaking Changes
- `LogFactory::Init` uses shared pointers instead of raw pointers when initializing

**Git History**: <https://github.com/Noelware/violet/compare/26.01...26.01.01>

### 26.01
🦊💖🐻‍❄️

**Git History**: <https://github.com/auguwu/logrin/commits/26.01>

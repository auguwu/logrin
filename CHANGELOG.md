---
title: Logrin | Changelog
availableAt:
    - https://github.com/auguwu/logrin/blob/master/CHANGELOG.md
    - https://floofy.dev/oss/logrin/changelog
---

### unreleased
**Git History**: <https://github.com/Noelware/violet/compare/26.02...master>

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

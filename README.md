# Teacher Scheduler

Đây là dự án C++ dùng để lập lịch giảng viên. Hiện tại dự án chỉ hỗ trợ **macOS**, chưa phát triển cross-platform.

## Yêu cầu

- macOS
- Homebrew
- C++17 compiler
- Thư viện:
  - OR-Tools
  - nlohmann_json
 
## Build dự án

```
rm -rf build

cmake -S . -B build

cmake --build build
```

##  Chạy chương trình

``` ./build/bin/teacher_scheduler ```
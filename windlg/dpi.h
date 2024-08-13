#pragma once
#include <Windows.h>

namespace lib::dpi {

[[nodiscard]] UINT cx(UINT cx);
[[nodiscard]] UINT cy(UINT cy);
[[nodiscard]] POINT pt(POINT p);
[[nodiscard]] SIZE sz(SIZE s);
[[nodiscard]] int x(int x);
[[nodiscard]] int y(int y);

}
